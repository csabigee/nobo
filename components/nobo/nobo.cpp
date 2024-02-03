#include "nobo.h"
#include "esphome/core/log.h"

namespace esphome {
namespace nobo {

static const char *const TAG = "nobo.climate";

constexpr uint8_t MAGIC1[25] = {0xC2,0x1F,0x73,0xC7,0x1C,0x6F,0xC3,0x18,0x6C,0xC0,0x15,0x69,0xBD,0x12,0x66,0xBA,0xE,0x62,0xB6,0xB,0x5F,0xB3,0x8,0x5C,0xB0};
constexpr uint8_t MAGIC2[25] = {0x31,0x64,0x24,0xE3,0xA3,0x64,0x24,0xE3,0xA3,0x63,0x23,0xE2,0xA2,0x62,0x22,0xE1,0xA2,0x62,0x22,0xE1,0xA1,0x61,0x21,0xE0,0xA0};
constexpr uint8_t MIN_TEMP=6;
constexpr uint8_t MAX_TEMP=30;
constexpr uint8_t TEMPSETOK = 0x17;
constexpr uint32_t SENSOR_UPDATE_INTERVAL = 1000; // the period in ms to read the environment temperature, and heater state from the device
constexpr uint8_t PWM_INTERVAL = 100;   // NOBO heater is doing PWM with 100 sec interval

void NoboClimate::setup() {
  for(uint32_t ii=0; ii<PWM_INTERVAL; ii++) {
    this->heater_states_.push(false);
  }
  this->heater_power_percentage_=0;
  this->heater_power_watt_=0;

  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->to_call(this).perform();
  } else {
    this->current_temperature=0;
    this->target_temperature=15;
    this->mode = climate::CLIMATE_MODE_HEAT;
  }
  this->begin_time_sensors_ = std::chrono::steady_clock::now();
  this->begin_time_state_ = std::chrono::steady_clock::now();
}

void NoboClimate::dump_config() {
  LOG_CLIMATE("", "Nobo Climate", this);
  ESP_LOGCONFIG(TAG, "  Min. Temperature: %d°C", MIN_TEMP);
  ESP_LOGCONFIG(TAG, "  Max. Temperature: %d°C", MAX_TEMP);
  ESP_LOGCONFIG(TAG, "  Supports HEAT: %s", YESNO(true));
  ESP_LOGCONFIG(TAG, "  Nominal power: %f", this->nominal_power_watt_);
  ESP_LOGCONFIG(TAG, "  Update interval: %f", this->update_interval_);
}

bool NoboClimate::set_target_temperature(uint8_t temperature) {
    if(temperature<MIN_TEMP)
        temperature=MIN_TEMP;
    if(temperature>MAX_TEMP)
        temperature=MAX_TEMP;

    union temp {
        uint16_t t16;
        uint8_t t8[2];
    }millicelsius; 
    millicelsius.t16 = (uint16_t)1000*(uint16_t)temperature;

    uint8_t tx_data[8];
    uint8_t rx_data[1];

    if(temperature==MIN_TEMP) {
        tx_data[0]=0x0B;
        tx_data[1]=0x00;
        tx_data[2]=0x00;
    } else {
        tx_data[0]=0x08;
        tx_data[1]=millicelsius.t8[0];
        tx_data[2]=millicelsius.t8[1];
    }
    tx_data[3]=0x00;
    tx_data[4]=MAGIC1[temperature-MIN_TEMP];
    tx_data[5]=MAGIC2[temperature-MIN_TEMP];


    i2c::ErrorCode error = this->write_register(0x01, tx_data, 6, false);
    if(error) {
        ESP_LOGW(TAG, "Failed to set target temperature, I2C command error!");
        return false;
    }
    error = this->read(rx_data, 1);
    if(error) {
        ESP_LOGW(TAG, "Failed to set target temperature, I2C command error!");
        return false;
    }

    return rx_data[0]==TEMPSETOK;
}

bool NoboClimate::get_heater_state() {
    uint8_t rx_data[1];
    i2c::ErrorCode error = this->read_register(0x05, rx_data, 1, false);
    if(error) {
        ESP_LOGW(TAG, "Failed to get heater state, I2C command error!");
        return false;
    }

    return rx_data[0]==0x01;
}

float NoboClimate::get_heater_temperature() {
    uint8_t rx_data[6];
    i2c::ErrorCode error = this->read_register(0x03, rx_data, 6, false);
    if(error) {
        ESP_LOGW(TAG, "Failed to get heater temperature, I2C command error!");
        return 0.0;
    }

    union temp {
      uint16_t t16;
      uint8_t t8[2];
    }millicelsius; 
    millicelsius.t8[0] = rx_data[0];
    millicelsius.t8[1] = rx_data[1];

    return static_cast<float>(millicelsius.t16)/1000.0;
}

void NoboClimate::update_power() {
    bool heater_state=this->get_heater_state();
    this->heater_states_.push(heater_state);
    bool old_heater_state = this->heater_states_.front();
    this->heater_states_.pop();

    if(heater_state) {
      this->heater_power_percentage_++;
    }
    if(old_heater_state) {
      this->heater_power_percentage_--;
    }
    this->heater_power_watt_=this->nominal_power_watt_*this->heater_power_percentage_/static_cast<float>(PWM_INTERVAL);
}

void NoboClimate::update_temp() {
  this->current_temperature = this->get_heater_temperature();
}

void NoboClimate::loop() {
  auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->begin_time_sensors_);
  if(elapsedTime.count()>SENSOR_UPDATE_INTERVAL) {
    this->begin_time_sensors_ = std::chrono::steady_clock::now();
    this->update_power();
    this->update_temp();
  }

  elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->begin_time_state_);
  if(elapsedTime.count()>((this->update_interval_*1000))) {
    this->begin_time_state_ = std::chrono::steady_clock::now();
    this->publish_state();
    if (this->power_sensor_ != nullptr) {
      this->power_sensor_->publish_state(this->heater_power_watt_);
    }
   }
}

void NoboClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value())
    this->mode = *call.get_mode();
  if (call.get_target_temperature().has_value())
    this->target_temperature = *call.get_target_temperature();

  if(call.get_mode()==climate::CLIMATE_MODE_OFF) {
   this->set_target_temperature(MIN_TEMP);  
  } else {
    this->set_target_temperature(static_cast<uint8_t>(this->target_temperature));
  }

  this->publish_state();
}

climate::ClimateTraits NoboClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_HEAT 
  });
  traits.set_supports_two_point_target_temperature(false);
  traits.set_supports_action(false);
  traits.set_visual_min_temperature(static_cast<float>(MIN_TEMP));
  traits.set_visual_max_temperature(static_cast<float>(MAX_TEMP));
  traits.set_visual_target_temperature_step(1.0);
  return traits;
}

}  // namespace nobo
}  // namespace esphome
