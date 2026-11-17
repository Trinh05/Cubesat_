// Bước 1: quét I2C. Phải thấy 0x68 (MPU6050) và 0x40 (INA219).
#include <Wire.h>
void setup() { Serial.begin(115200); delay(1000); Wire.begin(47, 21, 400000); }
void loop() {
  Serial.println("Scanning...");
  for (uint8_t a = 1; a < 127; a++) { Wire.beginTransmission(a); if (Wire.endTransmission() == 0) Serial.printf("Found 0x%02X\n", a); }
  delay(3000);
}
