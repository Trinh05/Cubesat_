#include "modem.h"
#include "config.h"
#include "secrets.h"

#define MODEM Serial1
static String gResp;

static bool waitFor(const char* expect, uint32_t to) {
  gResp = ""; uint32_t t = millis();
  while (millis() - t < to) {
    while (MODEM.available()) gResp += (char)MODEM.read();
    if (gResp.indexOf(expect) >= 0) return true;
    if (gResp.indexOf("ERROR") >= 0) return false;
    delay(5);
  }
  return false;
}
static bool atCmd(const String& cmd, const char* expect = "OK", uint32_t to = 2000) {
  while (MODEM.available()) MODEM.read();
  MODEM.println(cmd);
  return waitFor(expect, to);
}

bool modemAlive() { return atCmd("AT", "OK", 1000); }

void modemReset() {
  // Xung PWRKEY: mức/độ rộng xung tùy breakout - đối chiếu datasheet breakout của nhóm
  pinMode(PIN_MODEM_PWR, OUTPUT);
  digitalWrite(PIN_MODEM_PWR, HIGH); delay(1500);
  digitalWrite(PIN_MODEM_PWR, LOW);  delay(5000);
}

bool modemBegin() {
  MODEM.begin(115200, SERIAL_8N1, PIN_MODEM_RX, PIN_MODEM_TX);
  for (int i = 0; i < 3; i++) if (modemAlive()) { atCmd("ATE0"); return true; }
  modemReset();
  for (int i = 0; i < 5; i++) if (modemAlive()) { atCmd("ATE0"); return true; }
  return false;
}

bool modemAttach(uint32_t timeoutMs) {
  if (!atCmd("AT+CPIN?", "READY", 3000)) return false;          // SIM sẵn sàng
  atCmd(String("AT+CGDCONT=1,\"IP\",\"") + APN + "\"");
  uint32_t t = millis();
  while (millis() - t < timeoutMs) {                              // chờ đăng ký LTE
    if (atCmd("AT+CEREG?", "+CEREG:", 2000) && (gResp.indexOf(",1") >= 0 || gResp.indexOf(",5") >= 0)) return true;
    delay(2000);
  }
  return false;
}

int modemCsq() {
  if (!atCmd("AT+CSQ", "+CSQ:", 2000)) return 99;
  int i = gResp.indexOf("+CSQ:");
  return atoi(gResp.c_str() + i + 5);
}

int httpPost(const char* url, const uint8_t* body, size_t len, const char* ctype) {
  atCmd("AT+HTTPTERM");                                           // dọn phiên cũ (có thể báo lỗi, bỏ qua)
  if (!atCmd("AT+HTTPINIT")) return -1;
  int code = -2;
  do {
    if (!atCmd(String("AT+HTTPPARA=\"URL\",\"") + url + "\"")) break;
    if (!atCmd(String("AT+HTTPPARA=\"CONTENT\",\"") + ctype + "\"")) break;
    if (!atCmd(String("AT+HTTPDATA=") + len + ",10000", "DOWNLOAD", 3000)) break;
    MODEM.write(body, len);                                        // gửi nội dung (nhị phân)
    if (!waitFor("OK", 10000)) break;
    if (!atCmd("AT+HTTPACTION=1", "+HTTPACTION:", 30000)) break;
    int i = gResp.indexOf("+HTTPACTION:");
    int m = 0, st = 0, n = 0;
    if (sscanf(gResp.c_str() + i, "+HTTPACTION: %d,%d,%d", &m, &st, &n) >= 2) code = st;
  } while (0);
  atCmd("AT+HTTPTERM");
  return code;
}

int httpPostRetry(const char* url, const uint8_t* body, size_t len, const char* ctype) {
  const uint16_t wait[] = {2000, 5000, 10000};
  int code = -1;
  for (uint8_t k = 0; k < SEND_RETRIES; k++) {
    code = httpPost(url, body, len, ctype);
    if (code >= 200 && code < 300) return code;
    delay(wait[k]);
    if (code == -1 && !modemAlive()) { modemReset(); modemBegin(); modemAttach(30000); }
  }
  return code;
}

bool httpGet(const char* url, char* out, size_t n) {
  if (!url || !url[0]) return false;
  atCmd("AT+HTTPTERM");
  if (!atCmd("AT+HTTPINIT")) return false;
  bool ok = false; out[0] = 0;
  do {
    if (!atCmd(String("AT+HTTPPARA=\"URL\",\"") + url + "\"")) break;
    if (!atCmd("AT+HTTPACTION=0", "+HTTPACTION:", 20000)) break;
    if (!atCmd("AT+HTTPREAD=0,64", "+HTTPREAD:", 5000)) break;
    int i = gResp.indexOf("\n", gResp.indexOf("+HTTPREAD:"));    // nội dung nằm sau dòng +HTTPREAD
    if (i >= 0) { strncpy(out, gResp.c_str() + i + 1, n - 1); out[n - 1] = 0; ok = true; }
  } while (0);
  atCmd("AT+HTTPTERM");
  return ok;
}
