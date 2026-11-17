#include "camera_mod.h"
#include "config.h"
#include <LittleFS.h>

bool camBegin() {
  camera_config_t cfg = {};
  cfg.ledc_channel = LEDC_CHANNEL_0;
  cfg.ledc_timer   = LEDC_TIMER_0;
  cfg.pin_d0 = CAM_PIN_D0; cfg.pin_d1 = CAM_PIN_D1; cfg.pin_d2 = CAM_PIN_D2; cfg.pin_d3 = CAM_PIN_D3;
  cfg.pin_d4 = CAM_PIN_D4; cfg.pin_d5 = CAM_PIN_D5; cfg.pin_d6 = CAM_PIN_D6; cfg.pin_d7 = CAM_PIN_D7;
  cfg.pin_xclk = CAM_PIN_XCLK; cfg.pin_pclk = CAM_PIN_PCLK;
  cfg.pin_vsync = CAM_PIN_VSYNC; cfg.pin_href = CAM_PIN_HREF;
  cfg.pin_sccb_sda = CAM_PIN_SIOD; cfg.pin_sccb_scl = CAM_PIN_SIOC;
  cfg.pin_pwdn = CAM_PIN_PWDN; cfg.pin_reset = CAM_PIN_RESET;
  cfg.xclk_freq_hz = 20000000;
  cfg.pixel_format = PIXFORMAT_JPEG;
  cfg.frame_size   = FRAMESIZE_VGA;      // 640x480
  cfg.jpeg_quality = 12;                 // 10-15: số nhỏ = chất lượng cao, ảnh nặng hơn
  cfg.fb_count     = 2;
  cfg.fb_location  = CAMERA_FB_IN_PSRAM; // cần bật PSRAM = OPI PSRAM
  cfg.grab_mode    = CAMERA_GRAB_LATEST;
  return esp_camera_init(&cfg) == ESP_OK;
}

camera_fb_t* camCapture() {
  camera_fb_t* fb = esp_camera_fb_get();   // bỏ khung đầu (thường cũ) rồi lấy khung mới
  if (fb) { esp_camera_fb_return(fb); fb = esp_camera_fb_get(); }
  return fb;
}
void camRelease(camera_fb_t* fb) { if (fb) esp_camera_fb_return(fb); }

bool camThumbToFile(const char* path) {
  sensor_t* s = esp_camera_sensor_get();
  if (!s) return false;
  s->set_framesize(s, FRAMESIZE_QQVGA);    // 160x120
  delay(150);
  camera_fb_t* fb = camCapture();
  bool ok = false;
  if (fb) {
    File f = LittleFS.open(path, FILE_WRITE);
    if (f) { ok = (f.write(fb->buf, fb->len) == fb->len); f.close(); }
    esp_camera_fb_return(fb);
  }
  s->set_framesize(s, FRAMESIZE_VGA);      // trả về VGA
  delay(150);
  return ok;
}
