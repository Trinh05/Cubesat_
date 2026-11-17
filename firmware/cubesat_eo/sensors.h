#pragma once
#include "config.h"
bool      sensorsBegin();              // khởi tạo GPS UART, DS18B20, INA219 (IMU khởi tạo riêng)
void      sensorsStartTask();          // tạo task đọc cảm biến (nhân 1)
Telemetry snapshot();                  // bản sao telemetry an toàn (có mutex)
void      setState(uint8_t st);
void      setErr(uint8_t bit, bool on);
void      setImg(uint32_t id);
void      setRssi(int rssi);
void      setId(uint32_t id);
bool      attitudeOk();                // |roll|,|pitch| < TILT_MAX_DEG
bool      gpsAlive();                  // đã nhận được chuỗi NMEA
bool      tempAlive();
bool      powerAlive();
