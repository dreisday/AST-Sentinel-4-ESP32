# AST Sentinel 4 ESP32 Intercom Automation

A DIY intercom automation project using an ESP32, MQTT, and Home Assistant to provide remote ability to unlock an AST Sentinel 4 intercom from home assistant and mobile devices

## Features

- Detects doorbell ring from the intercom system
- Exposes sensor and button entities via MQTT discovery
- Sends a push notification with unlock action
- Allows remote unlocking via HA
- Opens a Lovelace view on notification tap, or unlock via action button
- Includes ESP32 Arduino sketch and reusable Home Assistant blueprint

## Parts List

- 3 x Relays - I used [these boards](https://www.bitsboxuk.com/index.php?main_page=product_info&cPath=253&products_id=4071) that include the optocouplers and a jumper to choose high/low triggering. You could use a single 4-way board, or I used a 2-way and a single to better fit in my case.
- 12v to 5v Buck Converter
- ESP32 Dev Board

## Arduino Setup

See `arduino/esp32_intercom.ino` for the sketch. Requires:
- PubSubClient - modified library with MQTT_MAX_PACKET_SIZE 512 is included
- WiFi
- ArduinoJson

## Wiring

![Wiring Schematic](https://raw.githubusercontent.com/dreisday/AST-Sentinel-4-ESP32/refs/heads/main/intercom%20schematic.png)

## Home Assistant

Use the blueprint in `home-assistant/blueprints/automation/` to set up the automation.

## License

This project is licensed under the [Creative Commons Attribution-ShareAlike 4.0 International License](https://creativecommons.org/licenses/by-sa/4.0/).
[![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-sa/4.0/)
