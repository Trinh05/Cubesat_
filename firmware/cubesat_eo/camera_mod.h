#pragma once
#include "esp_camera.h"
bool          camBegin();
camera_fb_t*  camCapture();                 // ảnh JPEG 640x480 (nhớ camRelease)
void          camRelease(camera_fb_t* fb);
bool          camThumbToFile(const char* path); // ảnh thu nhỏ 160x120 lưu vào LittleFS
