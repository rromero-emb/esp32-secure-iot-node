# ESP32 Secure IoT Node

Production-style ESP32 firmware built with **ESP-IDF** (C, FreeRTOS) — not the Arduino core.
A small sensor node that shows the full path from register-level drivers to connected, updatable, auditable firmware:

- **Own drivers** for a DHT11 (timing-critical, on the RMT peripheral), an SSD1306 OLED (I2C) and a PIR motion sensor (GPIO interrupts + debounce)
- **BLE provisioning** of Wi-Fi credentials, **MQTT over TLS**
- **Signed OTA updates with automatic rollback**
- **Deep sleep** with measured current
- **CI** on every push: build and size report today; unit tests and QEMU runs from session 4
- **SBOM + CVE report** for the firmware (Cyber Resilience Act–style evidence)

Companion project: [cra-embedded-linux-demo](https://github.com/rromero-emb/cra-embedded-linux-demo) — the same security story (verified boot, signed updates, SBOM) on embedded Linux.

## Status

| Session | Scope | Status |
|---|---|---|
| 0 | Project skeleton, Kconfig pin map, status LED, presence sensor driver, CI build | ✅ |
| 1 | DHT11 driver on RMT, SSD1306 driver on the I2C master API | 🔧 code done, on-board test pending |
| 2 | BLE Wi-Fi provisioning, MQTT over TLS, events and telemetry | ⏳ |
| 3 | HTTPS OTA with signed images and rollback | ⏳ |
| 4 | Deep sleep + wake-up sources, power measurements, Unity tests in QEMU | ⏳ |
| 5 | SBOM and vulnerability report (`esp-idf-sbom`), docs, demo GIF | ⏳ |

## Hardware

LAFVIN ESP32 Basic Starter Kit: ESP32 DevKit (ESP-32S module, 4 MB flash), DHT11 module, 0.96" SSD1306 OLED (I2C), HC-SR501 PIR motion sensor.

| Signal | Default GPIO | Kconfig option |
|---|---|---|
| Status LED (on-board) | 2 | `NODE_STATUS_LED_GPIO` |
| DHT11 data | 4 | `NODE_DHT11_GPIO` |
| PIR sensor output | 27 | `NODE_PRESENCE_SENSOR_GPIO` |
| OLED SDA / SCL | 21 / 22 | `NODE_I2C_SDA_GPIO` / `NODE_I2C_SCL_GPIO` |

All pins can be changed with `idf.py menuconfig` → *IoT node hardware*, where an active-low IR obstacle module can also be selected instead of the PIR.

## Architecture

```
main/                app_main: init, sampling loop, (later) state machine
components/
  status_led/        LED patterns per device state (esp_timer)
  presence_sensor/   GPIO ISR -> queue -> debounced callback in its own task (PIR or IR)
  dht11/             start pulse with the task asleep, reply captured by the RMT receiver
  ssd1306/           I2C master API, 1 KB frame buffer, 8x8 font, one I2C write per page
```

Drivers are plain ESP-IDF components with a small C API, so they can be unit-tested and reused in other projects.

## Build and flash

Requires [ESP-IDF v5.5](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/get-started/).

```sh
. $IDF_PATH/export.sh
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### Flash a prebuilt image (no toolchain needed)

Every CI run uploads a `firmware-esp32` artifact containing `merged-firmware.bin`.
Flash it at offset `0x0` from Chrome or Edge with Espressif's [web flasher](https://espressif.github.io/esptool-js/), or with esptool:

```sh
esptool.py --chip esp32 -p /dev/ttyUSB0 write_flash 0x0 merged-firmware.bin
```

Then open a serial monitor at 115200 baud.

Spanish working notes: [plan](docs/PLAN.md), [Windows setup](docs/WINDOWS.md), [wiring](docs/MONTAJE.md), [on-board test checklist](docs/PRUEBAS.md).

## Security notes

- Signed OTA images are verified **without enabling hardware Secure Boot** on the dev board: Secure Boot and Flash Encryption burn eFuses irreversibly. The production configuration is documented instead of applied.
- No secrets in the repository: Wi-Fi credentials come from BLE provisioning, keys are generated locally and ignored by git.

## License

MIT — see [LICENSE](LICENSE).
