#pragma once
#include <Arduino.h>
bool modemBegin();                                   // bật nguồn, kiểm tra AT
bool modemAttach(uint32_t timeoutMs = 60000);        // SIM, đăng ký mạng, APN
int  modemCsq();                                     // cường độ tín hiệu (0-31, 99 = không có)
int  httpPost(const char* url, const uint8_t* body, size_t len, const char* ctype);  // trả mã HTTP, <0 nếu lỗi
int  httpPostRetry(const char* url, const uint8_t* body, size_t len, const char* ctype);
bool httpGet(const char* url, char* out, size_t n);  // đọc lệnh từ server (nếu có)
void modemReset();                                   // khởi động lại modem qua PWRKEY
bool modemAlive();
