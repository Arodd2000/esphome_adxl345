#include "esphome.h"

class BarcodeUART : public Component, public UARTDevice {
 public:
  BarcodeUART(UARTComponent *parent) : UARTDevice(parent) {}

  TextSensor *barcode = new TextSensor();

  void setup() override {
    // nothing to do here
  }

  void loop() override {
    const int max_line_length = 80;

    static char buffer[max_line_length] = {0x00};
    static int pos;

    static bool start_detected = false;

    while (available()) {
      char c = read();

      if (!start_detected && c != 0x02) {
        continue;
      }

      if (c == 0x02) {
        start_detected = true;
        pos = 0;
        continue;
      }

      if (c == 0x03) {
        buffer[pos] = 0x00;
        barcode->publish_state({});
        barcode->publish_state(buffer);

        start_detected = false;
        continue;
      }

      buffer[pos] = c;
      pos++;
    }
  }
};
