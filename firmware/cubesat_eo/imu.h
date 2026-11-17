#pragma once
bool  imuBegin();          // khởi tạo + hiệu chuẩn bias con quay (giữ mô hình đứng yên)
void  imuUpdate();         // gọi mỗi 20 ms (50 Hz)
float imuRoll();
float imuPitch();
float imuYaw();
float imuTemp();
