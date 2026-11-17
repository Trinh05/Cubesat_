#pragma once
#include <Arduino.h>

// ================= CHÂN KẾT NỐI (khớp Bảng 3.1 của báo cáo) =================
#define PIN_MODEM_TX   43   // ESP32 TX -> RXD của A7680C
#define PIN_MODEM_RX   44   // ESP32 RX <- TXD của A7680C
#define PIN_MODEM_PWR  42   // -> chân PW (PWRKEY) của A7680C (tùy chọn)
#define PIN_GPS_RX      1   // ESP32 RX <- TXD của GPS
#define PIN_GPS_TX      2   // ESP32 TX -> RXD của GPS
#define PIN_I2C_SDA    47
#define PIN_I2C_SCL    21
#define PIN_ONEWIRE    41   // DS18B20 (kéo lên 4,7k)

// Camera ESP32-S3 CAM N16R8 (theo sơ đồ chân CAM_* trong báo cáo)
#define CAM_PIN_PWDN  -1
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK  15
#define CAM_PIN_SIOD   4
#define CAM_PIN_SIOC   5
#define CAM_PIN_D0    11   // Y2
#define CAM_PIN_D1     9   // Y3
#define CAM_PIN_D2     8   // Y4
#define CAM_PIN_D3    10   // Y5
#define CAM_PIN_D4    12   // Y6
#define CAM_PIN_D5    18   // Y7
#define CAM_PIN_D6    17   // Y8
#define CAM_PIN_D7    16   // Y9
#define CAM_PIN_VSYNC  6
#define CAM_PIN_HREF   7
#define CAM_PIN_PCLK  13

// ================= THAM SỐ NHIỆM VỤ =================
constexpr uint32_t CYCLE_MS        = 60000;  // chu kỳ chụp ảnh
constexpr uint32_t BEAT_MS         = 10000;  // chu kỳ telemetry ở STANDBY
constexpr uint32_t SAFE_BEACON_MS  = 60000;  // beacon ở SAFE MODE
constexpr uint32_t ATT_WAIT_MS     = 5000;   // chờ tư thế hợp lệ tối đa
constexpr uint32_t LINK_LOST_MS    = 300000; // mất liên lạc 5 phút -> SAFE MODE
constexpr float    TILT_MAX_DEG    = 15.0f;
constexpr float    VBAT_SELFTEST   = 3.5f;
constexpr float    VBAT_SAFE_LOW   = 3.4f;
constexpr float    VBAT_SAFE_EXIT  = 3.7f;
constexpr float    TEMP_SAFE_HIGH  = 45.0f;  // nhiệt độ pin
constexpr float    COMP_ALPHA      = 0.98f;  // hệ số bộ lọc bù (chọn bằng thử - sai)
constexpr uint8_t  SEND_RETRIES    = 3;
constexpr size_t   CHUNK_SIZE      = 4096;
constexpr uint8_t  MAX_IMG_PER_TX  = 3;      // số ảnh gửi tối đa mỗi lần TRANSMISSION

// 'A' = gửi ảnh đầy đủ theo khối; 'B' = chỉ gửi ảnh thu nhỏ 160x120
#define IMAGE_MODE 'B'

// Bit lỗi trong telemetry (trường err)
enum ErrBit : uint8_t { ERR_CAM=1, ERR_IMU=2, ERR_GPS=4, ERR_NET=8, ERR_FLASH=16, ERR_PWR=32, ERR_TEMP=64 };

enum State : uint8_t { BOOT, SELF_TEST, STANDBY, DATA_ACQ, PROCESSING, TRANSMISSION, SAFE_MODE };

struct Telemetry {
  uint32_t id = 0;
  uint8_t  st = BOOT;
  char     utc[24] = "";
  bool     gpsFix = false;
  double   lat = 0, lon = 0;
  float    alt = 0;
  float    roll = 0, pitch = 0, yaw = 0;
  float    tempBat = NAN, tempImu = NAN;
  float    vbat = 0, ibat = 0;
  int      rssi = 99;
  uint32_t img = 0;
  uint8_t  err = 0;
};
