# Montaje del kit (LAFVIN ESP32 Basic Starter Kit)

Placa: ESP32 DevKit con módulo ESP-32S (ESP32 clásico, 4 MB de flash). Los pines por defecto se cambian con `idf.py menuconfig` → *IoT node hardware*.

> **Regla general: alimenta todos los módulos a 3V3, no a 5V (VIN).** Los GPIO del ESP32 no toleran 5 V: un módulo alimentado a 5 V devuelve señales de 5 V que pueden dañar el pin.

## Conexiones

| Módulo | Pin del módulo | Pin de la placa | Sesión |
|---|---|---|---|
| LED de estado | (integrado en la placa) | GPIO2 | 0 |
| Sensor IR de obstáculos | VCC | 3V3 | 0 |
| | GND | GND | 0 |
| | OUT | **GPIO27** | 0 |
| DHT11 | VCC (+) | 3V3 | 1 |
| | DATA (S / OUT) | **GPIO4** | 1 |
| | GND (−) | GND | 1 |
| OLED SSD1306 0,96" (I2C) | VCC | 3V3 | 1 |
| | GND | GND | 1 |
| | SCL | **GPIO22** | 1 |
| | SDA | **GPIO21** | 1 |

## Notas por módulo

- **Sensor IR:** su salida va a 0 cuando detecta un obstáculo (se enciende su LED de "obstacle"). El potenciómetro azul ajusta la distancia de detección; empieza a media vuelta.
- **DHT11:**
  - Si es **módulo de 3 pines** (sobre una placa pequeña), ya lleva la resistencia de pull-up.
  - Si es el **sensor suelto de 4 pines** (azul, con rejilla), mirando la rejilla de frente: 1 = VCC, 2 = DATA, 3 = sin conexión, 4 = GND. Pon una resistencia de **10 kΩ entre DATA y 3V3**.
  - Necesita al menos 1 s entre lecturas (el firmware lee cada 5 s).
- **OLED:** la dirección I2C habitual es **0x3C**; algunos módulos usan 0x3D (configurable en menuconfig). El orden de pines varía entre fabricantes: **lee la serigrafía** antes de cablear, no te fíes del orden de esta tabla.

## Pines que conviene no usar

- GPIO6–11: conectados a la flash interna.
- GPIO0, GPIO2, GPIO12, GPIO15: pines de arranque. GPIO2 se usa para el LED, sin problema; no conectes nada que fuerce GPIO12 a nivel alto al arrancar.
- GPIO34–39: solo entrada y sin pull-up interno.

## Foto del montaje

Haz una foto del montaje terminado en la sesión 1 y guárdala como `docs/img/montaje.jpg`. Irá en el README y en el portfolio.
