# AST Sentinel 4 ESP32 Intercom Automation

A DIY intercom automation project using an ESP32 and Home Assistant to provide remote ability to unlock an AST Sentinel 4 intercom from Home Assistant and mobile devices.

Two interchangeable firmware options are provided - pick one:

| | [`mqtt/`](mqtt/) | [`esphome/`](esphome/) |
|---|---|---|
| Framework | Arduino | ESP-IDF (via ESPHome) |
| HA integration | MQTT + MQTT discovery | Native ESPHome API |
| Setup | Hand-written sketch, `secrets.h` | ESPHome YAML, `secrets.yaml` |
| Requires a broker | Yes | No |

Both options drive the same relay wiring and expose the same intercom status/unlock functionality to Home Assistant; they differ only in how the ESP32 talks to Home Assistant.

### Which one should I use?

- **New build, no strong opinion?** Use **ESPHome**. It's less code to maintain, doesn't need a broker, gets device connectivity tracking for free, and OTA updates are built in.
- **Already run an MQTT broker and want the device on it** (e.g. for use outside Home Assistant, or an existing MQTT-based dashboard)? Use **MQTT**.
- **Want to hand-tune the C++ logic** (e.g. the stretch goals in the To-Do list below, like 2-way audio)? The **MQTT/Arduino** sketch gives you a normal `.ino` file to extend; the ESPHome YAML is more declarative and better suited to the sensor/relay/timing logic it currently implements.

## Features

- Detects doorbell ring from the intercom system
- Exposes sensor and button entities to Home Assistant
- Sends a push notification with unlock action
- Allows remote unlocking via HA
- Opens a Lovelace view on notification tap, or unlock via action button

## To-Do

- Play pre-recorded audio through intercom when door is unlocked
- Implement 2-way audio
- Interface ESP32 directly with data U/D pins to remove need for relays

## Parts List

- 3 x Relays - I used [these boards](https://www.bitsboxuk.com/index.php?main_page=product_info&cPath=253&products_id=4071) that include the optocouplers and a jumper to choose high/low triggering. You could use a single 4-way board, or I used a 2-way and a single to better fit in my case.
- 12v to 5v Buck Converter
- ESP32 Dev Board

## Wiring

![Wiring Schematic](https://raw.githubusercontent.com/dreisday/AST-Sentinel-4-ESP32/refs/heads/main/intercom%20schematic.png)

Both firmware options use the same pins:

| Signal | GPIO |
|---|---|
| Ring detect (input) | 13 |
| Answer relay (output) | 12 |
| Unlock relay (output) | 14 |

## Option A: MQTT (Arduino)

Firmware: [`mqtt/esp32_intercom.ino`](mqtt/esp32_intercom.ino)
Blueprint: [`intercom_notify_and_unlock_mqtt.yaml`](homeassistant/blueprints/automation/intercom_notify_and_unlock_mqtt.yaml)

### Prerequisites
- Arduino IDE (or arduino-cli) with the `esp32` board package installed
- An MQTT broker reachable from the ESP32 (e.g. Mosquitto add-on in Home Assistant). Certificate-based broker authentication is not currently supported.
- The MQTT integration set up in Home Assistant, pointed at the same broker
- Arduino libraries: `PubSubClient`, `WiFi` (bundled with the ESP32 core), `ArduinoJson`

### Setup
1. Copy [`mqtt/example_secrets.h`](mqtt/example_secrets.h) to `mqtt/secrets.h` (gitignored) and fill in `WIFI_SSID`, `WIFI_PASSWORD`, `MQTT_SERVER`, `MQTT_PORT`, `MQTT_USER`, `MQTT_PASS`.
2. Optionally change the `deviceName` constant near the top of `esp32_intercom.ino` - this is the name shown for the device in Home Assistant.
3. Open `mqtt/esp32_intercom.ino` in the Arduino IDE, select your ESP32 board/port, and upload.
4. Watch the Serial Monitor (115200 baud) to confirm it connects to WiFi and the broker.
5. In Home Assistant, the device and its entities appear automatically via MQTT discovery - no manual entity configuration needed.
6. Go to **Settings > Automations & Scenes > Blueprints > Import Blueprint** and import [`intercom_notify_and_unlock_mqtt.yaml`](homeassistant/blueprints/automation/intercom_notify_and_unlock_mqtt.yaml), then create an automation from it, selecting the entities from step 5.

### What gets exposed to Home Assistant
- `binary_sensor` - connectivity status (via MQTT LWT: online/offline)
- `sensor` - intercom status (`idle` / `ringing` / `answered` / `unlocked`)
- `button` - Unlock Intercom

## Option B: ESPHome (native API)

Firmware: [`esphome/esp32_intercom.yaml`](esphome/esp32_intercom.yaml)
Blueprint: [`intercom_notify_and_unlock_esphome.yaml`](homeassistant/blueprints/automation/intercom_notify_and_unlock_esphome.yaml)

Built with the ESP-IDF framework and the Home Assistant native API instead of MQTT - no broker required, and device connectivity is tracked automatically by the ESPHome integration (no LWT/availability entity needed).

### Prerequisites
- [ESPHome](https://esphome.io/) (CLI, Home Assistant add-on, or dashboard) - no Arduino IDE or extra libraries needed, ESPHome manages the build

### Setup
1. Copy [`esphome/secrets.yaml.example`](esphome/secrets.yaml.example) to `esphome/secrets.yaml` (gitignored) and fill in `wifi_ssid`, `wifi_password`, `ap_password`, `api_encryption_key` (see the comments in that file for how to generate one), and `ota_password`.
2. Optionally adjust the `device_name` / `friendly_name` substitutions at the top of `esp32_intercom.yaml` - this is the name shown for the device in Home Assistant.
3. Compile and flash over USB for the first install:
   ```
   esphome run esphome/esp32_intercom.yaml
   ```
   Subsequent updates can be pushed over-the-air the same way, once the device is on WiFi.
4. In Home Assistant, go to **Settings > Devices & Services** - the device should be auto-discovered by the ESPHome integration (or add it manually by IP if discovery is disabled on your network). You'll be prompted for the `api_encryption_key` from step 1.
5. Go to **Settings > Automations & Scenes > Blueprints > Import Blueprint** and import [`intercom_notify_and_unlock_esphome.yaml`](homeassistant/blueprints/automation/intercom_notify_and_unlock_esphome.yaml), then create an automation from it, selecting the entities from step 4.

### What gets exposed to Home Assistant
- `sensor` - Intercom Status (`idle` / `ringing` / `answered` / `unlocked`)
- `button` - Unlock Intercom
- Device connectivity (shown on the device page in **Settings > Devices & Services**, tracked automatically - not a separate entity)

## License

This project is licensed under the [Creative Commons Attribution-ShareAlike 4.0 International License](https://creativecommons.org/licenses/by-sa/4.0/).
[![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-sa/4.0/)
