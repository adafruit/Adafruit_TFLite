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

#ifndef TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_MAGIC_WAND_CONSTANTS_H_
#define TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_MAGIC_WAND_CONSTANTS_H_

// The expected accelerometer data sample frequency
const float kTargetHz = 25;

// The number of expected consecutive inferences for each gesture type
extern const int kConsecutiveInferenceThresholds[3];
#endif  // TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_MAGIC_WAND_CONSTANTS_H_
