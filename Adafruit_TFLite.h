#include <Arduino.h>
#include <TensorFlowLite.h>
#include <SdFat.h>
#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/experimental/micro/compatibility.h"
#include "tensorflow/lite/experimental/micro/debug_log.h"
#include "tensorflow/lite/experimental/micro/debug_log_numbers.h"

class Adafruit_TFLite {
 public:
  Adafruit_TFLite(uint32_t arenasize);
  bool begin(void);

  uint32_t getArenaSize(void);
  uint8_t *getArena(void);

  bool loadModel(const unsigned char model_data[]);
  bool loadModel(File modelfile);

  tflite::MicroErrorReporter micro_error_reporter;
  tflite::ErrorReporter* error_reporter;

  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;

 private:
  uint32_t _arena_size = 0;
  uint8_t *_tensor_arena = nullptr;
  unsigned char *_model_data;
  const tflite::Model* _model = nullptr;
  tflite::ops::micro::AllOpsResolver _resolver;

};
