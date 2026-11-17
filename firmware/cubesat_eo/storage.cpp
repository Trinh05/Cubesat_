#include "storage.h"
#include <LittleFS.h>
#include <Preferences.h>
static Preferences prefs;

bool storageBegin() {
  prefs.begin("eo", false);
  return LittleFS.begin(true);
}
uint32_t nextImageId() { uint32_t id = prefs.getUInt("img", 0) + 1; prefs.putUInt("img", id); return id; }
uint32_t lastSentId()  { return prefs.getUInt("sent", 0); }
void setLastSentId(uint32_t id) { prefs.putUInt("sent", id); }
size_t freeBytes() { return LittleFS.totalBytes() - LittleFS.usedBytes(); }

void telemetryToJson(const Telemetry& t, char* out, size_t n) {
  snprintf(out, n,
    "{\"id\":%lu,\"st\":%u,\"t\":\"%s\",\"fix\":%d,\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.1f,"
    "\"roll\":%.1f,\"pitch\":%.1f,\"yaw\":%.1f,\"tmp\":%.1f,\"vbat\":%.2f,\"ibat\":%.3f,"
    "\"rssi\":%d,\"img\":%lu,\"err\":%u}",
    (unsigned long)t.id, t.st, t.utc, t.gpsFix, t.lat, t.lon, t.alt, t.roll, t.pitch, t.yaw,
    isnan(t.tempBat) ? 0.0f : t.tempBat, t.vbat, t.ibat, t.rssi, (unsigned long)t.img, t.err);
}

bool saveImage(uint32_t id, const uint8_t* buf, size_t len, const Telemetry& t) {
  if (freeBytes() < len + 2048) return false;               // hết bộ nhớ
  char p[32]; snprintf(p, sizeof(p), "/img_%05lu.jpg", (unsigned long)id);
  File f = LittleFS.open(p, FILE_WRITE);
  if (!f) return false;
  bool ok = (f.write(buf, len) == len); f.close();
  snprintf(p, sizeof(p), "/img_%05lu.json", (unsigned long)id);
  f = LittleFS.open(p, FILE_WRITE);
  if (f) { char js[400]; telemetryToJson(t, js, sizeof(js)); f.print(js); f.close(); }
  return ok;
}
