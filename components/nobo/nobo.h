#pragma once

#include <queue>
#include <chrono>
#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace nobo {

class NoboClimate : public climate::Climate, public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  
   bool set_target_temperature(uint8_t temperature);
   bool get_heater_state();
   float get_heater_temperature();
   void update_power();
   void update_temp();

   void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
   void set_nominal_power_watt(float nominal_power_watt) { nominal_power_watt_ = nominal_power_watt; }
   void set_update_interval(float update_interval) { update_interval_ = update_interval; }

 protected:
   void loop() override;
   void control(const climate::ClimateCall &call) override;
   climate::ClimateTraits traits() override;

  sensor::Sensor *power_sensor_{nullptr};
   float nominal_power_watt_;
   float update_interval_;
   std::queue<bool> heater_states_;
   float heater_power_percentage_;
   float heater_power_watt_;
   std::chrono::steady_clock::time_point begin_time_sensors_;
   std::chrono::steady_clock::time_point begin_time_state_;
};

}  // namespace nobo
}  // namespace esphome
