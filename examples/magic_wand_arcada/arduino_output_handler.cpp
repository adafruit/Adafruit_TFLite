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

#include "output_handler.h"

#include "Arduino.h"
#include "Adafruit_Arcada.h"

extern Adafruit_Arcada arcada;


void HandleOutput(tflite::ErrorReporter* error_reporter, int kind) {
  // The first time this method runs, set up our LED
  static int last_kind = -1;
  
  static bool is_initialized = false;
  if (!is_initialized) {
    pinMode(LED_BUILTIN, OUTPUT);
    is_initialized = true;
  }
  // Toggle the LED every time an inference is performed
  static int count = 0;
  ++count;
  if (count & 1) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Print some ASCII art for each gesture
  if (kind == 0) {
    error_reporter->Report(
        "WING:\n\r*         *         *\n\r *       * *       "
        "*\n\r  *     *   *     *\n\r   *   *     *   *\n\r    * *       "
        "* *\n\r     *         *\n\r");
    ImageReturnCode stat = arcada.drawBMP((char *)"wing.bmp", 0, 0);
    if (stat != IMAGE_SUCCESS) {
      arcada.display->fillScreen(ARCADA_BLACK);
      arcada.display->setCursor(20, 20);
      arcada.display->setTextColor(ARCADA_YELLOW);
      arcada.display->setTextSize(ceil(arcada.display->width() / 30));
      arcada.display->print("WING");
    }
    arcada.WavPlayComplete("wing.wav");
    arcada.pixels.fill(arcada.pixels.Color(50, 50, 0));
    arcada.pixels.show();
  } else if (kind == 1) {
    error_reporter->Report(
        "RING:\n\r          *\n\r       *     *\n\r     *         *\n\r "
        "   *           *\n\r     *         *\n\r       *     *\n\r      "
        "    *\n\r");
    ImageReturnCode stat = arcada.drawBMP((char *)"ring.bmp", 0, 0);
    if (stat != IMAGE_SUCCESS) {
      arcada.display->fillScreen(ARCADA_BLACK);
      arcada.display->setCursor(20, 20);
      arcada.display->setTextColor(ARCADA_PURPLE);
      arcada.display->setTextSize(ceil(arcada.display->width() / 30));
      arcada.display->print("RING");
    }
    arcada.WavPlayComplete("ring.wav");
    arcada.pixels.fill(arcada.pixels.Color(50, 0, 50));
    arcada.pixels.show();
  } else if (kind == 2) {
    error_reporter->Report(
        "SLOPE:\n\r        *\n\r       *\n\r      *\n\r     *\n\r    "
        "*\n\r   *\n\r  *\n\r * * * * * * * *\n\r");
    ImageReturnCode stat = arcada.drawBMP((char *)"slope.bmp", 0, 0);
    if (stat != IMAGE_SUCCESS) {
      arcada.display->fillScreen(ARCADA_BLACK);
      arcada.display->setCursor(20, 20);
      arcada.display->setTextColor(ARCADA_BLUE);
      arcada.display->setTextSize(ceil(arcada.display->width() / 40));
      arcada.display->print("SLOPE");
    }
    arcada.WavPlayComplete("slope.wav");
    arcada.pixels.fill(arcada.pixels.Color(0, 50, 50));
    arcada.pixels.show();
  } else {
    if (last_kind <= 2) {
      // re-draw intro
      ImageReturnCode stat = arcada.drawBMP((char *)"howto.bmp", 0, 0);
      if (stat != IMAGE_SUCCESS) {
        arcada.display->fillScreen(ARCADA_BLACK);
        arcada.display->setCursor(0, 0);
        arcada.display->setTextColor(ARCADA_WHITE);
        arcada.display->setTextSize(ceil(arcada.display->width() / 180.0));
        arcada.display->println("With screen facing");
        arcada.display->println("you, move board in");
        arcada.display->println("the shape of a");
        arcada.display->println("W, clockwise O or L");
      }
      arcada.pixels.fill(0);
      arcada.pixels.show();
    }
  }
  last_kind = kind;
}
