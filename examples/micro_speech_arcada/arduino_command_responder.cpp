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

#include "command_responder.h"

#include "Arduino.h"

#include <Adafruit_Arcada.h>
extern Adafruit_Arcada arcada;

// Toggles the built-in LED every inference, and lights a colored LED depending
// on which word was detected.
void RespondToCommand(tflite::ErrorReporter* error_reporter,
                      int32_t current_time, const char* found_command,
                      uint8_t score, bool is_new_command) {
  static bool is_initialized = false;
  if (!is_initialized) {
    pinMode(LED_BUILTIN, OUTPUT);
    // Pins for the built-in RGB LEDs on the Arduino Nano 33 BLE Sense
    is_initialized = true;
  }
  static int32_t last_command_time = 0;
  static int count = 0;
  static int certainty = 220;

  if (is_new_command) {
    error_reporter->Report("Heard %s (%d) @%dms", found_command, score,
                           current_time);
    if (found_command[0] == 'y') {
      last_command_time = current_time;
      ImageReturnCode stat = arcada.drawBMP((char *)"yes.bmp", 0, 0);
      if (stat != IMAGE_SUCCESS) {
        arcada.display->fillScreen(ARCADA_BLACK);
        arcada.display->setCursor(20, 20);
        arcada.display->setTextColor(ARCADA_GREEN);
        arcada.display->setTextSize(ceil(arcada.display->width() / 30));
        arcada.display->print("YES");
      }
      arcada.WavPlayComplete("yes.wav");
      arcada.pixels.fill(arcada.pixels.Color(0, 50, 0));
      arcada.pixels.show();
    }

    if (found_command[0] == 'n') {
      last_command_time = current_time;
      ImageReturnCode stat = arcada.drawBMP((char *)"no.bmp", 0, 0);
      if (stat != IMAGE_SUCCESS) {
        arcada.display->fillScreen(ARCADA_BLACK);
        arcada.display->setCursor(20, 20);
        arcada.display->setTextColor(ARCADA_RED);
        arcada.display->setTextSize(ceil(arcada.display->width() / 30));
        arcada.display->print("NO");
      }
      arcada.WavPlayComplete("no.wav");
      arcada.pixels.fill(arcada.pixels.Color(50, 0, 0));
      arcada.pixels.show();
    }

    if (found_command[0] == 'u') {
      last_command_time = current_time;
      last_command_time = current_time;
      ImageReturnCode stat = arcada.drawBMP((char *)"no.bmp", 0, 0);
      if (stat != IMAGE_SUCCESS) {
        arcada.display->fillScreen(ARCADA_BLACK);
        arcada.display->setCursor(20, 20);
        arcada.display->setTextColor(ARCADA_LIGHTGREY);
        arcada.display->setTextSize(ceil(arcada.display->width() / 30));
        arcada.display->print("???");
      }
      arcada.pixels.fill(arcada.pixels.Color(10, 10, 10));
      arcada.pixels.show();
    }
  }

  // If last_command_time is non-zero but was 1 seconds ago, zero it
  // and switch off the LED.
  if (last_command_time != 0) {
    if (last_command_time < (current_time - 1000)) {
      last_command_time = 0;
      // draw intro
      ImageReturnCode stat = arcada.drawBMP((char *)"howto.bmp", 0, 0);
      if (stat != IMAGE_SUCCESS) {
        arcada.display->fillScreen(ARCADA_BLACK);
        arcada.display->setCursor(0, 0);
        arcada.display->setTextColor(ARCADA_WHITE);
        arcada.display->setTextSize(ceil(arcada.display->width() / 180.0));
        arcada.display->println("Hold microphone");
        arcada.display->println("approx. 6-8\" away ");
        arcada.display->println("from mouth and say");
        arcada.display->println("either YES or NO");
      }
      arcada.pixels.fill(arcada.pixels.Color(0, 0, 0));
      arcada.pixels.show();
    }
    // If it is non-zero but <3 seconds ago, do nothing.
    return;
  }

  // Otherwise, toggle the LED every time an inference is performed.
  ++count;
  if (count & 1) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
