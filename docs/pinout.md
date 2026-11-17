# Gán chân (khớp Bảng 3.1 của báo cáo)

| Module | Chân ESP32-S3 | Giao tiếp |
|---|---|---|
| A7680C | GPIO43 (TX→RXD), GPIO44 (RX←TXD), GPIO42 (→PW) | UART 115200 |
| GPS GY-GPS6MV2 | GPIO1 (RX←TXD), GPIO2 (TX→RXD) | UART 9600 |
| MPU6050 (0x68), INA219 (0x40) | GPIO47 (SDA), GPIO21 (SCL) | I2C 400 kHz |
| DS18B20 | GPIO41 (+ trở kéo 4,7 kΩ) | 1-Wire |
| Camera | Đầu FPC (GPIO4–18) | DVP |

<img width="962" height="519" alt="image" src="https://github.com/user-attachments/assets/cb8cfe45-3bc8-493c-b5cf-f99059979608" />
