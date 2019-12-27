#include <M5StickC.h>

HardwareSerial sensor(2);

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  sensor.begin(9600, SERIAL_8N1, 33, 32);
}

bool validate_checksum(byte data[9]) {
  return byte(~(data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7])) ==  data[8];
}

void loop() {
  delay(1000);

  const byte req[] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  sensor.write(req, sizeof(req));

  byte res[9];
  int nread = sensor.readBytes(res, sizeof(res));
  if (nread != 9) {
    Serial.println("error: no response");
    return;
  }
  if (res[0] != 0xff) {
    Serial.println("error: unexpected start byte");
    return;
  }
  if (res[1] != 0x86) {
    Serial.println("error: unexpected sensor #");
    return;
  }
  if (!validate_checksum(res)) {
    Serial.println("error: invalid checksum");
    return;
  }

  int val = res[2] * 256 + res[3];
  Serial.print("value: ");
  Serial.println(val);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("CO2: %4d ppm", val);
}
