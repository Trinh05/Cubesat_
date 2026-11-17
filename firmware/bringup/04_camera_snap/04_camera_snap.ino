// Bước 4: kiểm tra camera: chụp 1 ảnh JPEG và in kích thước. Dùng khai báo chân từ firmware/cubesat_eo/config.h
// (sao chép config.h và camera_mod.* vào thư mục sketch này, hoặc mở trực tiếp sketch chính).
#include "config.h"
#include "camera_mod.h"
void setup() {
  Serial.begin(115200); delay(1000);
  if (!camBegin()) { Serial.println("Camera FAIL"); return; }
  camera_fb_t* fb = camCapture();
  if (fb) { Serial.printf("OK: %u bytes, %ux%u\n", (unsigned)fb->len, fb->width, fb->height); camRelease(fb); }
  else Serial.println("Capture FAIL");
}
void loop() {}
