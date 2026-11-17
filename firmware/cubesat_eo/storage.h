#pragma once
#include <Arduino.h>
#include "config.h"
bool     storageBegin();
uint32_t nextImageId();                              // tăng và lưu bộ đếm ảnh (NVS)
uint32_t lastSentId();
void     setLastSentId(uint32_t id);
bool     saveImage(uint32_t id, const uint8_t* buf, size_t len, const Telemetry& t);
size_t   freeBytes();
void     telemetryToJson(const Telemetry& t, char* out, size_t n);
