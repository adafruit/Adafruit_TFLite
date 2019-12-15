/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "audio_provider.h"
#include "micro_features_micro_model_settings.h"
#include <Adafruit_Arcada.h>

#if defined(__SAMD51__)
  //#define USE_EXTERNAL_MIC A8  // D2 on pybadge
  #define USE_EDGEBADGE_PDMMIC
  //#define AUDIO_OUT A0         // uncomment to 'echo' audio to A0 for debugging
  #define DEFAULT_BUFFER_SIZE 512

  #if defined(USE_EDGEBADGE_PDMMIC)
    #include <Adafruit_ZeroPDMSPI.h>
    #define PDM_SPI            SPI2    // PDM mic SPI peripheral
    #define TIMER_CALLBACK     SERCOM3_0_Handler
    Adafruit_ZeroPDMSPI pdmspi(&PDM_SPI);
  #endif
#endif

#if defined(ARDUINO_NRF52840_CIRCUITPLAY)
  #include <PDM.h>
  #define DEFAULT_BUFFER_SIZE DEFAULT_PDM_BUFFER_SIZE
  extern PDMClass PDM;
  static volatile int samplesRead; // number of samples read
#endif

extern Adafruit_Arcada arcada;
void CaptureSamples();

namespace {
bool g_is_audio_initialized = false;
// An internal buffer able to fit 16x our sample size
constexpr int kAudioCaptureBufferSize = DEFAULT_BUFFER_SIZE * 16;
int16_t g_audio_capture_buffer[kAudioCaptureBufferSize];
// A buffer that holds our output
int16_t g_audio_output_buffer[kMaxAudioSampleSize];
// Mark as volatile so we can check in a while loop to see if
// any samples have arrived yet.
volatile int32_t g_latest_audio_timestamp = 0;
// Our callback buffer for collecting a chunk of data
volatile int16_t recording_buffer[DEFAULT_BUFFER_SIZE];

volatile int max_audio = -32768, min_audio = 32768;
}  // namespace

#if defined(ARDUINO_NRF52840_CIRCUITPLAY)
  // IRQ handler
  static void onPDMdata() {
    // query the number of bytes available
    int bytesAvailable = PDM.available();
  
    // read into the sample buffer
    PDM.read((int16_t *)recording_buffer, bytesAvailable);
  
    // 16-bit, 2 bytes per sample
    samplesRead = bytesAvailable / 2;
  }
#endif

void TIMER_CALLBACK() {
  static bool ledtoggle = false;
  static uint32_t audio_idx = 0;
  int32_t sample = 0;

#if defined(USE_EDGEBADGE_PDMMIC)
  uint16_t read_pdm;
  if (!pdmspi.decimateFilterWord(&read_pdm)) {
    return; // not ready for data yet!
  }
  sample = read_pdm;
#endif

  // tick tock test
  //digitalWrite(LED_BUILTIN, ledtoggle);
  //ledtoggle = !ledtoggle;

#if defined(ARDUINO_NRF52840_CIRCUITPLAY)
  PDM.IrqHandler();    // wait for samples to be read
  if (samplesRead) {
    max_audio = -32768, min_audio = 32768;
    for (int i=0; i<samplesRead; i++) {
      max_audio = max(recording_buffer[i], max_audio);
      min_audio = min(recording_buffer[i], min_audio);
    }
    CaptureSamples();
    samplesRead = 0;
  }
  // we did a whole buffer at once, so we're done
  return;
#endif
  
  if (audio_idx >= DEFAULT_BUFFER_SIZE) {
    CaptureSamples();
    max_audio = -32768, min_audio = 32768;
    audio_idx = 0;
  }

#if defined(USE_EXTERNAL_MIC)
  sample = analogRead(USE_EXTERNAL_MIC);
  sample -= 2047; // 12 bit audio unsigned  0-4095 to signed -2048-~2047
#endif
#if defined(USE_EDGEBADGE_PDMMIC)
  sample -= 32676; // from 0-65535 to -32768 to 32768
#endif
#if defined(AUDIO_OUT)
  analogWrite(AUDIO_OUT, sample+2048); 
#endif

  recording_buffer[audio_idx] = sample;
  max_audio = max(max_audio, sample);
  min_audio = min(min_audio, sample);
  audio_idx++;
}

TfLiteStatus InitAudioRecording(tflite::ErrorReporter* error_reporter) {
  Serial.begin(115200);
  
  Serial.println("init audio"); delay(10);
  
  // Hook up the callback that will be called with each sample
#if defined(USE_EXTERNAL_MIC)
  arcada.timerCallback(kAudioSampleFrequency, TIMER_CALLBACK);
  analogReadResolution(12);
#endif
#if defined(USE_EDGEBADGE_PDMMIC)
  pdmspi.begin(kAudioSampleFrequency);
  Serial.print("Final PDM frequency: "); Serial.println(pdmspi.sampleRate);
#endif
#if defined(AUDIO_OUT)  
  analogWriteResolution(12);
#endif
#if defined(ARDUINO_NRF52840_CIRCUITPLAY)
  PDM.onReceive(onPDMdata);
  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM!");
    while (1) yield();
  }
  arcada.timerCallback(kAudioSampleFrequency, TIMER_CALLBACK);
#endif

  // Block until we have our first audio sample
  while (!g_latest_audio_timestamp) {
    delay(1);
  }

  return kTfLiteOk;
}

void CaptureSamples() {
  // This is how many bytes of new data we have each time this is called
  const int number_of_samples = DEFAULT_BUFFER_SIZE;
  // Calculate what timestamp the last audio sample represents
  const int32_t time_in_ms =
      g_latest_audio_timestamp +
      (number_of_samples / (kAudioSampleFrequency / 1000));
  // Determine the index, in the history of all samples, of the last sample
  const int32_t start_sample_offset =
      g_latest_audio_timestamp * (kAudioSampleFrequency / 1000);
  // Determine the index of this sample in our ring buffer
  const int capture_index = start_sample_offset % kAudioCaptureBufferSize;
  // Read the data to the correct place in our buffer, note 2 bytes per buffer entry
  memcpy(g_audio_capture_buffer + capture_index, (void *)recording_buffer, DEFAULT_BUFFER_SIZE*2);
  // This is how we let the outside world know that new audio data has arrived.
  g_latest_audio_timestamp = time_in_ms;

  int peak = (max_audio - min_audio);
  Serial.printf("pp %d\n", peak);
  //int normalized = map(peak, 20, 2000, 0, 65535);
  //arcada.pixels.setPixelColor(0, arcada.pixels.gamma32(arcada.pixels.ColorHSV(normalized)));
  //arcada.pixels.show();
}

TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                             int start_ms, int duration_ms,
                             int* audio_samples_size, int16_t** audio_samples) {
  // Set everything up to start receiving audio
  if (!g_is_audio_initialized) {
    TfLiteStatus init_status = InitAudioRecording(error_reporter);
    if (init_status != kTfLiteOk) {
      return init_status;
    }
    g_is_audio_initialized = true;
  }
  // This next part should only be called when the main thread notices that the
  // latest audio sample data timestamp has changed, so that there's new data
  // in the capture ring buffer. The ring buffer will eventually wrap around and
  // overwrite the data, but the assumption is that the main thread is checking
  // often enough and the buffer is large enough that this call will be made
  // before that happens.

  // Determine the index, in the history of all samples, of the first
  // sample we want
  const int start_offset = start_ms * (kAudioSampleFrequency / 1000);
  // Determine how many samples we want in total
  const int duration_sample_count =
      duration_ms * (kAudioSampleFrequency / 1000);
  for (int i = 0; i < duration_sample_count; ++i) {
    // For each sample, transform its index in the history of all samples into
    // its index in g_audio_capture_buffer
    const int capture_index = (start_offset + i) % kAudioCaptureBufferSize;
    // Write the sample to the output buffer
    g_audio_output_buffer[i] = g_audio_capture_buffer[capture_index];
  }

  // Set pointers to provide access to the audio
  *audio_samples_size = kMaxAudioSampleSize;
  *audio_samples = g_audio_output_buffer;

  return kTfLiteOk;
}

int32_t LatestAudioTimestamp() { return g_latest_audio_timestamp; }
