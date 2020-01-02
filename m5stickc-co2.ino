#include <M5StickC.h>


class MHZ19B {
private:
  HardwareSerial serial_;
  String error_message_;

public:
  MHZ19B(HardwareSerial serial) : serial_(serial) {}
  MHZ19B(int uart_nr) : serial_(HardwareSerial(uart_nr)) {}

  void begin(int8_t rxPin, int8_t txPin) {
    this->serial_.begin(9600, SERIAL_8N1, rxPin, txPin);
  }

  void end() { this->serial_.end(); }

  void setTimeout(unsigned long timeout) { this->serial_.setTimeout(timeout); }

  unsigned long getTimeout() { return this->serial_.getTimeout(); }

  int read_co2() {
    const byte req[] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    this->serial_.write(req, sizeof(req));

    byte res[9];
    int nread = this->serial_.readBytes(res, sizeof(res));
    if (nread < 9) {
      this->error_message_ = "no response";
      return -1;
    }
    if (res[0] != 0xff) {
      this->error_message_ = "unexpected start byte";
      return -1;
    }
    if (res[1] != 0x86) {
      this->error_message_ = "unexpecetd sensor number";
      return -1;
    }
    if (!validate_checksum(res)) {
      this->error_message_ = "invalid checksum";
      return -1;
    }

    return res[2] * 256 + res[3];
  }

  int zero_point_calibration() {
    const byte req[] = {0xff, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};
    this->serial_.write(req, sizeof(req));
    return 0;
  }

  String get_last_error() {
    return this->error_message_;
  }

private:
  bool validate_checksum(byte data[9]) {
    return byte(~(data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7]) + 1) ==  data[8];
  }
};

MHZ19B sensor(2);

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  sensor.begin(33, 32);
}

void loop() {
  M5.update();

  if (M5.BtnB.wasPressed()) {
    sensor.zero_point_calibration();
  }

  int val = sensor.read_co2();
  if (val < 0) {
    goto error;
  }
  Serial.print("value: ");
  Serial.println(val);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("CO2: %4d ppm", val);
  delay(1000);
  return;

error:
  String error_msg = sensor.get_last_error();
  Serial.print("error: ");
  Serial.println(error_msg.c_str());
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(RED);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("error: %s", error_msg.c_str());
  delay(1000);
  return;
}
