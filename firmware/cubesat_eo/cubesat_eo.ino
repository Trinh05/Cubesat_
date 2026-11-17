/*
 * CubeSat 1U - nhiệm vụ quan sát Trái Đất (OBC ESP32-S3 CAM N16R8)
 * Máy trạng thái: BOOT -> SELF_TEST -> STANDBY -> DATA_ACQ -> PROCESSING -> TRANSMISSION -> SAFE_MODE
 * Board: "ESP32S3 Dev Module", Flash 16MB, PSRAM "OPI PSRAM", USB CDC On Boot: Enabled
 */
#include <LittleFS.h>
#include <esp_task_wdt.h>
#include "config.h"
#include "secrets.h"
#include "imu.h"
#include "sensors.h"
#include "camera_mod.h"
#include "storage.h"
#include "modem.h"

static State state = BOOT;
static uint32_t tState = 0, tCycle = 0, tBeat = 0, tLastOk = 0, tCmd = 0, tSafeOk = 0;
static uint8_t camFail = 0;
static camera_fb_t* fb = nullptr;
static uint32_t curImg = 0;

static void go(State s) {
  Serial.printf("[FSM] %d -> %d (t=%lu ms)\n", state, s, millis());
  state = s; tState = millis(); setState(s);
}

// ---------- gửi dữ liệu ----------
static bool sendTelemetry() {
  Telemetry t = snapshot();
  t.rssi = modemCsq(); setRssi(t.rssi);
  static uint32_t seq = 0; t.id = ++seq; setId(seq);
  char js[400]; telemetryToJson(t, js, sizeof(js));
  int code = httpPostRetry(TELEMETRY_URL, (uint8_t*)js, strlen(js), "application/json");
  bool ok = (code >= 200 && code < 300);
  setErr(ERR_NET, !ok);
  if (ok) tLastOk = millis();
  Serial.printf("[TX] telemetry #%lu -> HTTP %d\n", (unsigned long)seq, code);
  return ok;
}

static bool sendFileChunks(const char* path, uint32_t id) {
  File f = LittleFS.open(path, FILE_READ);
  if (!f) return false;
  size_t total = f.size(), chunks = (total + CHUNK_SIZE - 1) / CHUNK_SIZE;
  uint8_t* buf = (uint8_t*)malloc(CHUNK_SIZE);
  if (!buf) { f.close(); return false; }
  bool ok = true;
  for (size_t i = 0; i < chunks && ok; i++) {
    size_t n = f.read(buf, CHUNK_SIZE);
    char url[220];
    snprintf(url, sizeof(url), "%s&img=%lu&idx=%u&total=%u", IMAGE_URL, (unsigned long)id, (unsigned)i, (unsigned)chunks);
    int code = httpPostRetry(url, buf, n, "application/octet-stream");
    ok = (code >= 200 && code < 300);
    Serial.printf("[TX] img %lu chunk %u/%u -> HTTP %d\n", (unsigned long)id, (unsigned)i + 1, (unsigned)chunks, code);
    esp_task_wdt_reset();
  }
  free(buf); f.close();
  return ok;
}

static bool sendImage(uint32_t id) {
  char p[32];
#if IMAGE_MODE == 'A'
  snprintf(p, sizeof(p), "/img_%05lu.jpg", (unsigned long)id);
#else
  snprintf(p, sizeof(p), "/thumb_%05lu.jpg", (unsigned long)id);
#endif
  return sendFileChunks(p, id);
}

static void pollCommand() {
  if (strlen(COMMAND_URL) == 0) return;
  char cmd[64];
  if (!httpGet(COMMAND_URL, cmd, sizeof(cmd))) return;
  Serial.printf("[CMD] %s\n", cmd);
  if (strstr(cmd, "CAPTURE") && state == STANDBY) { tCycle = 0; }
  else if (strstr(cmd, "SAFE")) go(SAFE_MODE);
  else if (strstr(cmd, "RESUME") && state == SAFE_MODE) go(STANDBY);
  else if (strstr(cmd, "RESET")) ESP.restart();
}

// ---------- tự kiểm tra ----------
static uint8_t selfTest() {
  uint8_t critical = 0;
  if (!camBegin())             { setErr(ERR_CAM, true);  critical++; Serial.println("[ST] CAMERA FAIL"); }
  if (!modemBegin() || !modemAttach(60000)) { setErr(ERR_NET, true); critical++; Serial.println("[ST] MODEM/NET FAIL"); }
  if (freeBytes() < 200000)    { setErr(ERR_FLASH, true); critical++; Serial.println("[ST] FLASH FULL"); }
  Telemetry t = snapshot();
  if (powerAlive() && t.vbat < VBAT_SELFTEST) { setErr(ERR_PWR, true); critical++; Serial.println("[ST] VBAT LOW"); }
  // Lỗi phụ -> chỉ đặt cờ, vẫn vào STANDBY (chế độ suy giảm)
  delay(3000);
  if (!gpsAlive())  { setErr(ERR_GPS, true);  Serial.println("[ST] GPS no NMEA"); }
  if (!tempAlive()) { setErr(ERR_TEMP, true); Serial.println("[ST] DS18B20 missing"); }
  return critical;
}

static bool needSafe() {
  Telemetry t = snapshot();
  if (powerAlive() && t.vbat > 0.5f && t.vbat < VBAT_SAFE_LOW) return true;
  if (!isnan(t.tempBat) && t.tempBat > TEMP_SAFE_HIGH) return true;
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== CubeSat EO Engineering Model ===");
  esp_reset_reason_t rr = esp_reset_reason();
  Serial.printf("Reset reason: %d\n", rr);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  esp_task_wdt_config_t wdt = { .timeout_ms = 30000, .idle_core_mask = 0, .trigger_panic = true };
  esp_task_wdt_reconfigure(&wdt);
#else
  esp_task_wdt_init(30, true);
#endif
  esp_task_wdt_add(NULL);
  if (!storageBegin()) Serial.println("LittleFS FAIL");
  sensorsBegin();
  sensorsStartTask();
  go(BOOT);
}

void loop() {
  esp_task_wdt_reset();
  if (state != SAFE_MODE && state != BOOT && state != SELF_TEST && needSafe()) go(SAFE_MODE);

  switch (state) {
    case BOOT:
      tLastOk = millis();
      go(SELF_TEST);
      break;

    case SELF_TEST:
      if (selfTest() == 0) { tCycle = millis(); go(STANDBY); }
      else go(SAFE_MODE);
      break;

    case STANDBY:
      if (millis() - tBeat >= BEAT_MS) { tBeat = millis(); sendTelemetry(); pollCommand(); }
      if (millis() - tCycle >= CYCLE_MS) { tCycle = millis(); go(DATA_ACQ); }
      if (millis() - tLastOk > LINK_LOST_MS) go(SAFE_MODE);
      break;

    case DATA_ACQ:
      if (attitudeOk() || millis() - tState >= ATT_WAIT_MS) {
        bool tilted = !attitudeOk();
        fb = camCapture();
        if (!fb) { if (++camFail >= 3) { setErr(ERR_CAM, true); go(SAFE_MODE); } else go(STANDBY); break; }
        camFail = 0;
        curImg = nextImageId(); setImg(curImg);
        Telemetry t = snapshot();
        if (tilted) Serial.println("[ACQ] lệch tư thế - gắn cờ");
        if (!saveImage(curImg, fb->buf, fb->len, t)) setErr(ERR_FLASH, true);
        camRelease(fb); fb = nullptr;
#if IMAGE_MODE == 'B'
        char tp[32]; snprintf(tp, sizeof(tp), "/thumb_%05lu.jpg", (unsigned long)curImg);
        camThumbToFile(tp);
#endif
        go(PROCESSING);
      }
      break;

    case PROCESSING:
      // metadata + CRC32 đã được ghi cùng ảnh; danh sách chờ gửi = (lastSentId, curImg]
      go(TRANSMISSION);
      break;

    case TRANSMISSION: {
      bool ok = sendTelemetry();
      uint32_t sent = lastSentId();
      uint8_t n = 0;
      while (ok && sent < curImg && n < MAX_IMG_PER_TX) {
        if (sendImage(sent + 1)) { setLastSentId(++sent); n++; } else { ok = false; }
      }
      go(STANDBY);
      break;
    }

    case SAFE_MODE: {
      static uint32_t tBeacon = 0;
      if (millis() - tBeacon >= SAFE_BEACON_MS) { tBeacon = millis(); if (modemAlive()) sendTelemetry(); pollCommand(); }
      Telemetry t = snapshot();
      bool good = (!powerAlive() || t.vbat > VBAT_SAFE_EXIT) && (isnan(t.tempBat) || t.tempBat < TEMP_SAFE_HIGH - 5);
      if (!good) tSafeOk = millis();
      if (good && millis() - tSafeOk > 60000 && millis() - tState > 60000) { tLastOk = millis(); go(STANDBY); }
      break;
    }
  }
  delay(10);
}
