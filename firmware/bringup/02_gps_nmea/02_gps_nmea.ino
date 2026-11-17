// Bước 2: in thô chuỗi NMEA của GPS (đặt anten ngoài trời). GPS: RX=GPIO1, TX=GPIO2.
void setup() { Serial.begin(115200); Serial2.begin(9600, SERIAL_8N1, 1, 2); }
void loop() { while (Serial2.available()) Serial.write(Serial2.read()); }
