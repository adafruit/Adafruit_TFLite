#include "Adafruit_Arcada_TFLite.h"

Adafruit_Arcada arcada;

Adafruit_Arcada_TFLite::Adafruit_Arcada_TFLite(uint32_t arenasize) {
  _arena_size = arenasize;
  error_reporter = &micro_error_reporter;
}

bool Adafruit_Arcada_TFLite::begin(void) {
  _tensor_arena = (uint8_t *)malloc(_arena_size);
  if (! _tensor_arena) {
    Serial.println("Failed to malloc tensor area");
    return false;
  }

  return true;
}

uint32_t Adafruit_Arcada_TFLite::getArenaSize(void) {
  return _arena_size;
}

uint8_t *Adafruit_Arcada_TFLite::getArena(void) {
  return _tensor_arena;
}
