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

// The pin of the Arduino's built-in LED
int led = LED_BUILTIN;

// Track whether the function has run at least once
bool initialized = false;

// helper function to let us scale floating point values nicely
double mapf(double x, double in_min, double in_max, double out_min, double out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Animates a dot across the screen to represent the current x and y values
void HandleOutput(tflite::ErrorReporter* error_reporter, float x_value,
                  float y_value) {
  // Do this only once
  if (!initialized) {
    // Add some text to describe what's up!
    arcada.display->fillScreen(ARCADA_BLACK);
    arcada.display->setTextColor(ARCADA_WHITE);
    arcada.display->setTextSize(1);
    const char *header = "TensorFlow Lite";
    const char *footer = "Sine wave model";
    arcada.display->setCursor((arcada.display->width()-strlen(header)*6)/2, 0);
    arcada.display->print(header);
    arcada.display->setCursor((arcada.display->width()-strlen(footer)*6)/2, arcada.display->height()-8);
    arcada.display->print(footer);
    initialized = true;
  }

  // map the x input value (0-2*PI) and the y value (-1.5 to 1.5)
  // to the size of the display
  float pixel_x, pixel_y;
  static float last_pixel_x, last_pixel_y;
  pixel_x = mapf(x_value, 0, 2*3.1415, 0, arcada.display->width());
  pixel_y = mapf(y_value, -1.75, 1.75, 0, arcada.display->height());
  if (pixel_x == 0) {
     // clear the screen
     arcada.display->fillRect(0, 10, arcada.display->width(), arcada.display->width()-20, ARCADA_BLACK);
  }

  arcada.display->fillCircle(pixel_x, pixel_y, 3, ST77XX_RED);
  last_pixel_x = pixel_x;
  last_pixel_y = pixel_y;

  // slow it down so we can see the ball!
  delay(3);
}
