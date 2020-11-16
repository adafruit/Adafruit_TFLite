/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

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

#include "accelerometer_handler.h"

#include <Arduino.h>
#include "Adafruit_Arcada.h"
extern Adafruit_Arcada arcada;

/* this is a little annoying to figure out, as a tip - when
 * holding the board straight, output should be (0, 0, 1)
 * tiling the board 90* left, output should be (0, 1, 0)
 * tilting the board 90* forward, output should be (1, 0, 0);
 */
 
#if defined(ADAFRUIT_PYBADGE_M4_EXPRESS)
// holding up with screen/neopixels facing you
const int X_POSITION = 1;
const int Y_POSITION = 2;
const int Z_POSITION = 0;
const bool INVERT_X = true;
const bool INVERT_Y = true;
const bool INVERT_Z = false;
#endif

#if defined(ARDUINO_NRF52840_CIRCUITPLAY)
// holding up with gizmo facing you
const int X_POSITION = 1;
const int Y_POSITION = 2;
const int Z_POSITION = 0;
const bool INVERT_X = true;
const bool INVERT_Y = true;
const bool INVERT_Z = false;
#endif

#if defined(ARDUINO_NRF52840_CLUE)
// holding up with gizmo facing you
const int X_POSITION = 1;
const int Y_POSITION = 2;
const int Z_POSITION = 0;
const bool INVERT_X = true;
const bool INVERT_Y = true;
const bool INVERT_Z = false;
#endif

#include "constants.h"

// A buffer holding the last 200 sets of 3-channel values
float save_data[600] = {0.0};
// Most recent position in the save_data buffer
int begin_index = 0;
// True if there is not yet enough data to run inference
bool pending_initial_data = true;
// How often we should save a measurement during downsampling
int sample_every_n;
// The number of measurements since we last saved one
int sample_skip_counter = 1;

uint32_t last_reading_stamp = 0;

TfLiteStatus SetupAccelerometer(tflite::ErrorReporter* error_reporter) {
  // Wait until we know the serial port is ready
  //while (!Serial) { yield(); }

  arcada.pixels.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  arcada.accel->setRange(LIS3DH_RANGE_4_G);
  arcada.accel->setDataRate(LIS3DH_DATARATE_25_HZ);
  float sample_rate = 25;
  
  // Determine how many measurements to keep in order to
  // meet kTargetHz
  sample_every_n = static_cast<int>(roundf(sample_rate / kTargetHz));

  error_reporter->Report("Magic starts!");

  return kTfLiteOk;
}

bool ReadAccelerometer(tflite::ErrorReporter* error_reporter, float* input,
                       int length, bool reset_buffer) {
  // Clear the buffer if required, e.g. after a successful prediction
  if (reset_buffer) {
    memset(save_data, 0, 600 * sizeof(float));
    begin_index = 0;
    pending_initial_data = true;
  }
  // Keep track of whether we stored any new data
  bool new_data = false;
  // Loop through new samples and add to buffer
  while (arcada.accel->haveNewData()) {
    float x, y, z;
    
    // Read each sample, removing it from the device's FIFO buffer
    sensors_event_t event; 
    
    if (! arcada.accel->getEvent(&event)) {
      error_reporter->Report("Failed to read data");
      break;
    }
    
    // Throw away this sample unless it's the nth
    if (sample_skip_counter != sample_every_n) {
      sample_skip_counter += 1;
      continue;
    }
    
    float values[3] = {0, 0, 0};
    values[X_POSITION] = event.acceleration.x / 9.8;
    values[Y_POSITION] = event.acceleration.y / 9.8;
    values[Z_POSITION] = event.acceleration.z / 9.8;

    x = values[0];
    y = values[1];
    z = values[2];

    if (INVERT_X) {
      x *= -1;
    }
    if (INVERT_Y) {
      y *= -1;
    }
    if (INVERT_Z) {
      z *= -1;
    }
    Serial.print(x, 2);
    Serial.print(", "); Serial.print(y, 2);
    Serial.print(", "); Serial.println(z, 2);
    
    last_reading_stamp = millis();
    // Write samples to our buffer, converting to milli-Gs
    save_data[begin_index++] = x * 1000;
    save_data[begin_index++] = y * 1000;
    save_data[begin_index++] = z * 1000;
    
    // Since we took a sample, reset the skip counter
    sample_skip_counter = 1;
    // If we reached the end of the circle buffer, reset
    if (begin_index >= 600) {
      begin_index = 0;
    }
    new_data = true;
  }

  // Skip this round if data is not ready yet
  if (!new_data) {
    return false;
  }

  // Check if we are ready for prediction or still pending more initial data
  if (pending_initial_data && begin_index >= 200) {
    pending_initial_data = false;
  }

  // Return if we don't have enough data
  if (pending_initial_data) {
    return false;
  }

  // Copy the requested number of bytes to the provided input tensor
  for (int i = 0; i < length; ++i) {
    int ring_array_index = begin_index + i - length;
    if (ring_array_index < 0) {
      ring_array_index += 600;
    }
    input[i] = save_data[ring_array_index];
  }

  return true;
}
