#include "sensors.h"
#include "imu.h"
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_INA219.h>

static Telemetry gTm;
static SemaphoreHandle_t gMutex;
static TinyGPSPlus gps;
static OneWire ow(PIN_ONEWIRE);
static DallasTemperature ds(&ow);
static Adafruit_INA219 ina;
static bool inaOk = false, dsOk = false, imuOk = false;

bool sensorsBegin() {
  gMutex = xSemaphoreCreateMutex();
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);
  Serial2.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
  ds.begin();  dsOk  = ds.getDeviceCount() > 0;
  inaOk = ina.begin();
  imuOk = imuBegin();
  if (!imuOk) gTm.err |= ERR_IMU;
  return imuOk;
}

static void sensorsTask(void*) {
  uint32_t last1s = 0;
  for (;;) {
    uint32_t t0 = millis();
    while (Serial2.available()) gps.encode(Serial2.read());   // GPS liên tục
    if (imuOk) imuUpdate();                                    // IMU 50 Hz

    bool slow = (t0 - last1s >= 1000);
    float tb = NAN, vb = 0, ib = 0;
    if (slow) {
      last1s = t0;
      if (dsOk) { ds.requestTemperatures(); tb = ds.getTempCByIndex(0); }
      if (inaOk) { vb = ina.getBusVoltage_V(); ib = ina.getCurrent_mA() / 1000.0f; }
    }
    if (xSemaphoreTake(gMutex, 10) == pdTRUE) {
      gTm.roll = imuRoll(); gTm.pitch = imuPitch(); gTm.yaw = imuYaw(); gTm.tempImu = imuTemp();
      if (gps.location.isValid()) {
        gTm.gpsFix = true; gTm.lat = gps.location.lat(); gTm.lon = gps.location.lng();
        if (gps.altitude.isValid()) gTm.alt = gps.altitude.meters();
      } else gTm.gpsFix = false;
      if (gps.date.isValid() && gps.time.isValid())
        snprintf(gTm.utc, sizeof(gTm.utc), "%04d-%02d-%02dT%02d:%02d:%02dZ", gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
      if (slow) { if (!isnan(tb) && tb > -100) gTm.tempBat = tb; if (inaOk) { gTm.vbat = vb; gTm.ibat = ib; } }
      xSemaphoreGive(gMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(20));   // 50 Hz
  }
}

void sensorsStartTask() { xTaskCreatePinnedToCore(sensorsTask, "sensors", 6144, nullptr, 2, nullptr, 1); }

Telemetry snapshot() { Telemetry c; xSemaphoreTake(gMutex, portMAX_DELAY); c = gTm; xSemaphoreGive(gMutex); return c; }
#define LOCKED(stmt) do { xSemaphoreTake(gMutex, portMAX_DELAY); stmt; xSemaphoreGive(gMutex); } while (0)
void setState(uint8_t st) { LOCKED(gTm.st = st); }
void setErr(uint8_t bit, bool on) { LOCKED(if (on) gTm.err |= bit; else gTm.err &= ~bit); }
void setImg(uint32_t id) { LOCKED(gTm.img = id); }
void setRssi(int r) { LOCKED(gTm.rssi = r); }
void setId(uint32_t id) { LOCKED(gTm.id = id); }

bool attitudeOk() { Telemetry t = snapshot(); return fabsf(t.roll) < TILT_MAX_DEG && fabsf(t.pitch) < TILT_MAX_DEG; }
bool gpsAlive()   { return gps.charsProcessed() > 10; }
bool tempAlive()  { return dsOk; }
bool powerAlive() { return inaOk; }
