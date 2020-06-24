/*!
 * @file Adafruit_TFLite.h
 */
#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include <Arduino.h>
#include <SdFat.h>
#include <TensorFlowLite.h>

#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/experimental/micro/compatibility.h"
#include "tensorflow/lite/experimental/micro/debug_log.h"
#include "tensorflow/lite/experimental/micro/debug_log_numbers.h"

/*!
 * TensofFlow Lite class
 */
class Adafruit_TFLite {
public:
  /*!
   * TensorFlow Lite constructor
   * @param arenasize How much ram to pre-allocate for TensorFLow
   */
  Adafruit_TFLite(uint32_t arenasize);
  /*!
   * @brief Attempts to allocate RAM
   * @return Returns true if RAM allocation was successful
   */
  bool begin(void);
  /*!
   * @brief Gets the amount of RAM preallocated for TensorFLow
   * @return Returns the amount of RAM preallocated for TensorFlow
   */
  uint32_t getArenaSize(void);
  /*!
   * @brief Gets the arena
   * @return Returns the arena, the block of RAM allocated for TensorFlow
   */
  uint8_t *getArena(void);

  /*!
   * @brief Loads model from data
   * @param model_data[] Model data to be loaded
   * @return Returns false if model is not supported
   */
  bool loadModel(const unsigned char model_data[]);
  /*!
   * @brief Loads model from file
   * @param modelfile File to with model to load
   * @return Returns false if model could not be read, otherwise, returns true
   */
  bool loadModel(File modelfile);

  /*!
   * @brief Outputs debug information
   */
  tflite::MicroErrorReporter micro_error_reporter;
  /*!
   * @brief Outputs debug information
   */
  tflite::ErrorReporter *error_reporter;

  /*!
   * @brief Contains code to load and run models
   */
  tflite::MicroInterpreter *interpreter = nullptr;
  /*!
   * @brief TensorFlow input
   */
  TfLiteTensor *input = nullptr;
  /*!
   * @brief TensorFlow output
   */
  TfLiteTensor *output = nullptr;

private:
  uint32_t _arena_size = 0;
  uint8_t *_tensor_arena = nullptr;
  unsigned char *_model_data;
  const tflite::Model *_model = nullptr;
  tflite::ops::micro::AllOpsResolver _resolver;
};
