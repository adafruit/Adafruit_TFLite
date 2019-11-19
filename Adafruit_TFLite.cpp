#include "Adafruit_TFLite.h"

Adafruit_TFLite::Adafruit_TFLite(uint32_t arenasize) {
  _arena_size = arenasize;
  error_reporter = &micro_error_reporter;
}

bool Adafruit_TFLite::begin(void) {
  _tensor_arena = (uint8_t *)malloc(_arena_size);
  if (! _tensor_arena) {
    Serial.println("Failed to malloc tensor area");
    return false;
  }

  return true;
}

uint32_t Adafruit_TFLite::getArenaSize(void) {
  return _arena_size;
}

uint8_t *Adafruit_TFLite::getArena(void) {
  return _tensor_arena;
}


bool Adafruit_TFLite::loadModel(File modelfile) {
  if (! modelfile) {
    Serial.print("Could not open file");
    return false;
  }
  ssize_t filesize = modelfile.size();
  _model_data = (unsigned char *)malloc(filesize);
  if (! _model_data) {
    Serial.print("Could not allocate memory");
    return false;
  }
  Serial.printf("Allocated %d byte model to address $%08x\n", 
	       filesize, &_model_data);
  if (modelfile.read(_model_data, filesize) != filesize) {
    Serial.print("Could not read file");
    return false;
  }
  return loadModel(_model_data);
}

bool Adafruit_TFLite::loadModel(const unsigned char model_data[]) {
  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  _model = tflite::GetModel(model_data);
  if (_model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        _model->version(), TFLITE_SCHEMA_VERSION);
    return false;
  }

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter 
    static_interpreter(_model, _resolver, _tensor_arena, 
		       _arena_size, error_reporter);
  interpreter = &static_interpreter;


  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    return false;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);

  return true;
}
