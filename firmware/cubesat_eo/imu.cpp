#include "imu.h"
#include "config.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

static Adafruit_MPU6050 mpu;
static float roll_ = 0, pitch_ = 0, yaw_ = 0, temp_ = NAN;
static float gbx = 0, gby = 0, gbz = 0;   // bias con quay (rad/s)
static uint32_t tLast = 0;

bool imuBegin() {
  if (!mpu.begin(0x68)) return false;
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(100);
  // Hiệu chuẩn bias: trung bình 200 mẫu khi đứng yên
  sensors_event_t a, g, t;
  for (int i = 0; i < 200; i++) {
    mpu.getEvent(&a, &g, &t);
    gbx += g.gyro.x; gby += g.gyro.y; gbz += g.gyro.z;
    delay(5);
  }
  gbx /= 200; gby /= 200; gbz /= 200;
  // Khởi tạo góc từ gia tốc kế
  mpu.getEvent(&a, &g, &t);
  roll_  = atan2f(a.acceleration.y, a.acceleration.z) * RAD_TO_DEG;
  pitch_ = atan2f(-a.acceleration.x, sqrtf(a.acceleration.y*a.acceleration.y + a.acceleration.z*a.acceleration.z)) * RAD_TO_DEG;
  yaw_ = 0;
  tLast = micros();
  return true;
}

void imuUpdate() {
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);
  uint32_t now = micros();
  float dt = (now - tLast) * 1e-6f;
  tLast = now;
  if (dt <= 0 || dt > 0.5f) dt = 0.02f;

  float wx = (g.gyro.x - gbx) * RAD_TO_DEG;   // độ/giây
  float wy = (g.gyro.y - gby) * RAD_TO_DEG;
  float wz = (g.gyro.z - gbz) * RAD_TO_DEG;

  float accRoll  = atan2f(a.acceleration.y, a.acceleration.z) * RAD_TO_DEG;
  float accPitch = atan2f(-a.acceleration.x, sqrtf(a.acceleration.y*a.acceleration.y + a.acceleration.z*a.acceleration.z)) * RAD_TO_DEG;

  // Bộ lọc bù: theta = alpha*(theta + w*dt) + (1-alpha)*theta_acc
  roll_  = COMP_ALPHA * (roll_  + wx * dt) + (1.0f - COMP_ALPHA) * accRoll;
  pitch_ = COMP_ALPHA * (pitch_ + wy * dt) + (1.0f - COMP_ALPHA) * accPitch;
  yaw_  += wz * dt;                // chỉ tích phân con quay -> bị trôi (không có từ kế)
  temp_  = t.temperature;
}

float imuRoll()  { return roll_; }
float imuPitch() { return pitch_; }
float imuYaw()   { return yaw_; }
float imuTemp()  { return temp_; }
