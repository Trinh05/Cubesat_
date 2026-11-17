// Bước 3: cầu nối Serial Monitor <-> A7680C. Gõ AT, AT+CPIN?, AT+CSQ, AT+CEREG? (chọn "Both NL & CR").
// Modem: ESP32 TX=GPIO43 -> RXD, ESP32 RX=GPIO44 <- TXD.
void setup() { Serial.begin(115200); Serial1.begin(115200, SERIAL_8N1, 44, 43); Serial.println("Gõ lệnh AT..."); }
void loop() { while (Serial.available()) Serial1.write(Serial.read()); while (Serial1.available()) Serial.write(Serial1.read()); }
