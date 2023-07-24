#include "esphome.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

static const char *TAG = "ADXL345";

class ADXL345Sensor : public PollingComponent {
  public:
    Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
    BinarySensor *activity = new BinarySensor();

    ADXL345Sensor() : PollingComponent(1000) { }

    void setup() override {
      accel.begin();

      uint8_t pctrl = accel.readRegister(ADXL345_REG_POWER_CTL);

      if (!(pctrl & (1 << 3)) || true) {
        ESP_LOGI(TAG, "configuring ADXL");

        // offsets
        accel.writeRegister(ADXL345_REG_OFSX, 0);
        accel.writeRegister(ADXL345_REG_OFSY, 0);
        accel.writeRegister(ADXL345_REG_OFSZ, 0);

        accel.setDataRate(ADXL345_DATARATE_12_5_HZ);
        accel.setRange(ADXL345_RANGE_2_G);

        // 10 * 62.5mg
        accel.writeRegister(ADXL345_REG_THRESH_ACT, 10);
        // 2 * 62.5mg
        accel.writeRegister(ADXL345_REG_THRESH_INACT, 2);
        // time inactive 5s
        accel.writeRegister(ADXL345_REG_TIME_INACT, 15);
        // ac-coupled, all axis enabled
        accel.writeRegister(ADXL345_REG_ACT_INACT_CTL, 255);

        // activity to INT1, else INT2
        accel.writeRegister(ADXL345_REG_INT_MAP, 239);

        // disable all interrupts
        accel.writeRegister(ADXL345_REG_INT_ENABLE, 0);

        enable_inactivity_interrupt(true);

        // bypass FIFO
        accel.writeRegister(ADXL345_REG_FIFO_CTL, 0);

      }
      ESP_LOGI(TAG, "Setting power ctl.");
      // link mode 32, auto_sleep 16, measure 8 enable
      accel.writeRegister(ADXL345_REG_POWER_CTL, 32 + 16 + 8);
      //
    }

    void enable_activity_interrupt(bool enabled)
    {
      ESP_LOGI(TAG, "activity interrupt: %u", enabled);
      uint8_t int_enable = accel.readRegister(ADXL345_REG_INT_ENABLE);
      if (enabled) {
        int_enable = int_enable | (1 << 4);
      } else {
        int_enable = int_enable & ~(1 << 4);
      }
      accel.writeRegister(ADXL345_REG_INT_ENABLE, int_enable);
    }

    void enable_inactivity_interrupt(bool enabled)
    {
      ESP_LOGI(TAG, "inactivity interrupt: %u", enabled);
      uint8_t int_enable = accel.readRegister(ADXL345_REG_INT_ENABLE);
      if (enabled) {
        int_enable = int_enable | (1 << 3);
      } else {
        int_enable = int_enable & ~(1 << 3);
      }
      accel.writeRegister(ADXL345_REG_INT_ENABLE, int_enable);
    }

    void update() override {
      uint8_t interrupts = accel.readRegister(ADXL345_REG_INT_SOURCE);
      ESP_LOGI(TAG, "int src: %u", interrupts);
      bool act = false;
      bool detected = false;
      if (interrupts & (1 << 4)) {
        ESP_LOGI(TAG, "int activity");
        act = true;
        detected = true;
      }
      if (interrupts & (1 << 3)) {
        ESP_LOGI(TAG, "int intactivity");
        act = false;
        detected = true;
      }
      if (detected) {
        activity->publish_state(act);
      }
      enable_activity_interrupt(false);
    }

};
