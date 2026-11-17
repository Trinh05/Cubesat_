# CubeSat 1U – Nhiệm vụ quan sát Trái Đất (ESP32-S3 + GPS + 4G)

Engineering Model vệ tinh nhỏ: chụp ảnh mặt đất, gắn thẻ GPS/tư thế/nhiệt độ, lưu trên OBC và truyền xuống trạm mặt đất qua 4G.
Đồ án môn *Thiết kế, tích hợp vệ tinh nhỏ và đồ án* (EMA3103 34), Nhóm 02, ĐH Công nghệ – ĐHQGHN.

## Phần cứng
ESP32-S3 CAM N16R8 · GPS GY-GPS6MV2 (NEO-6M) · modem 4G A7680C · pin 18650 + mạch sạc/bảo vệ.
Sơ đồ chân: `docs/pinout.md`.

## Kiến trúc
Payload → OBC (ESP32-S3) → Data Handling (LittleFS, JSON) → Communication (A7680C, HTTP) → Ground Station (server miễn phí).

Máy trạng thái: `BOOT → SELF_TEST → STANDBY → DATA_ACQ → PROCESSING → TRANSMISSION → SAFE_MODE`.

## Cấu trúc thư mục
```
firmware/cubesat_eo/   chương trình chính (mở cubesat_eo.ino bằng Arduino IDE)
firmware/bringup/      4 chương trình kiểm tra từng module (I2C, GPS, modem AT, camera)
tools/                 server giả lập và công cụ ghép ảnh chạy trên máy tính
docs/                  sơ đồ chân, ảnh, báo cáo
```


## Thứ tự kiểm tra (bring-up)
`01_i2c_scan` → `02_gps_nmea` → `03_modem_at` → `04_camera_snap` → `cubesat_eo`.

