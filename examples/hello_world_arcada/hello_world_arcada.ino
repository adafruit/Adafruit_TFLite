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

#include <TensorFlowLite.h>
#include "Adafruit_TFLite.h"
#include "Adafruit_Arcada.h"

#include "output_handler.h"
#include "sine_model_data.h"

// Create an area of memory to use for input, output, and intermediate arrays.
// Finding the minimum value for your model may require some trial and error.
const int kTensorAreaSize  (2 * 1024);

// This constant represents the range of x values our model was trained on,
// which is from 0 to (2 * Pi). We approximate Pi to avoid requiring additional
// libraries.
const float kXrange = 2.f * 3.14159265359f;

// Will need tuning for your chipset
const int kInferencesPerCycle = 200;
int inference_count = 0;

Adafruit_Arcada arcada;
Adafruit_TFLite ada_tflite(kTensorAreaSize);

// The name of this function is important for Arduino compatibility.
void setup() {
  Serial.begin(115200);
  //while (!Serial) yield();

  arcada.arcadaBegin();
  // If we are using TinyUSB we will have the filesystem show up!
  arcada.filesysBeginMSD();
  arcada.filesysListFiles();
  // Set the display to be on!
  arcada.displayBegin();
  arcada.setBacklight(255);
  arcada.display->fillScreen(ARCADA_BLUE);
  
  if (! ada_tflite.begin()) {
    arcada.haltBox("Failed to initialize TFLite");
    while (1) yield();
  }
  if (arcada.exists("model.tflite")) {
    arcada.infoBox("Loading model.tflite from disk!");
    if (! ada_tflite.loadModel(arcada.open("model.tflite"))) {
      arcada.haltBox("Failed to load model file");
    }
  } else if (! ada_tflite.loadModel(g_sine_model_data)) {
    arcada.haltBox("Failed to load default model");
  }
  Serial.println("\nOK");

  // Keep track of how many inferences we have performed.
  inference_count = 0;
}

// The name of this function is important for Arduino compatibility.
void loop() {
  // Calculate an x value to feed into the model. We compare the current
  // inference_count to the number of inferences per cycle to determine
  // our position within the range of possible x values the model was
  // trained on, and use this to calculate a value.
  float position = static_cast<float>(inference_count) /
                   static_cast<float>(kInferencesPerCycle);
  float x_val = position * kXrange;

  // Place our calculated x value in the model's input tensor
  ada_tflite.input->data.f[0] = x_val;

  // Run inference, and report any error
  TfLiteStatus invoke_status = ada_tflite.interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    ada_tflite.error_reporter->Report("Invoke failed on x_val: %f\n",
                           static_cast<double>(x_val));
    return;
  }

  // Read the predicted y value from the model's output tensor
  float y_val = ada_tflite.output->data.f[0];

  // Output the results. A custom HandleOutput function can be implemented
  // for each supported hardware target.
  HandleOutput(ada_tflite.error_reporter, x_val, y_val);

  // Increment the inference_counter, and reset it if we have reached
  // the total number per cycle
  inference_count += 1;
  if (inference_count >= kInferencesPerCycle) inference_count = 0;
}
