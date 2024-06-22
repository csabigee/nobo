Nobo Panel Heaters ESPHome integration (experimental)
=================================
<p align="center">
  <img width="1000" src="https://en.nobo.no/sites/default/files/styles/background_image_full_width/public/panel_heaters_4.jpg">
</p>

<img align="right" width="180" src="https://nobo.hu/files/manager/files/ECU%202Te.png">

The ``nobo`` panel heater component allows you to use your Nobo panel heaters ([Product selection](https://en.nobo.no/products/panel-heaters)) with ESPHome.
All panel heaters are supported which works with NCU control unit: NCU-1R, NCU-2R, NCU-ER, NCU-2Te, NCU-2T.
This component replaces the NCU control unit with custom hardware based on ESP32, and communicates with the heater via I2C.

<p align="center">
  <img width="400" src="https://github.com/csabigee/nobo/assets/96885207/bae032d4-3c0c-49d0-899d-9c90e8a9ba0c">
  <img width="400" src="https://github.com/csabigee/nobo/assets/96885207/b252c0ee-e4ac-4975-acbd-730e30660cf6">

</p>
<p align="center">
  <em>Nobo Climate Device UI in Home Assistant.</em>
</p>

Nobo Configuration:
------------------------
```yaml
i2c:
  sda: 32
  scl: 33
  scan: False
  frequency: 50kHz

external_components:
  - source:
      type: git
      url: https://github.com/csabigee/nobo
      ref: main
    components: [ nobo ]

climate:
  - platform: nobo
    name: "Nobo Heater LivingRoom"
    nominal_power: 2000 W
    power:
      name: "Nobo Heater Power LivingRoom"
    update_interval: 60s
```

Configuration variables:
------------------------

- **name** (**Required**, string): The name for the temperature sensor.
- **id** (*Optional*, [ID](https://esphome.io/guides/configuration-types#config-id)): Set the ID of this sensor for use in lambdas.
- **nominal_power** (**Required**, float): The rated power of the heater device (from 450-2500 W)
- **update_interval** (*Optional*, [Time](https://esphome.io/guides/configuration-types#config-time)): The interval to check the power sensor. Defaults to ``30s``.
- **power** (*Optional*): Send the instantaneous power
  - **name** (**Required**, string): The name for the power sensor.
  - **id** (*Optional*,  [ID](https://esphome.io/guides/configuration-types#config-id)): Set the ID of this sensor for use in lambdas.

Links:
------------------------
 
[:mag: Reverse engineering the protocol :mag_right:](https://github.com/csabigee/nobo/wiki/Home)

[:hammer_and_wrench: Building the custom thermostat :hammer_and_wrench:](https://www.hackster.io/csabigee/nobo-wireless-thermostat-01f106)

[:zap: PCB design files :zap:](https://github.com/csabigee/nobo-pcb)

[:gift: Enclosure design files :gift:](https://github.com/csabigee/nobo-enclosure)

[:octocat: Software sources :octocat:](https://github.com/csabigee/nobo)
