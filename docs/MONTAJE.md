# Montaje del kit (LAFVIN ESP32 Basic Starter Kit)

Placa: ESP32 DevKit con módulo ESP-32S (ESP32 clásico, 4 MB de flash). Los pines por defecto se cambian con `idf.py menuconfig` → *IoT node hardware*.

> **Regla general: alimenta los módulos a 3V3, no a 5V (VIN).** Los GPIO del ESP32 no toleran 5 V: un módulo alimentado a 5 V puede devolver señales de 5 V que dañan el pin.
> **Única excepción: el PIR HC-SR501**, que necesita 5 V (VIN) para funcionar, pero tiene su propio regulador y su salida es de 3,3 V.

## Conexiones

| Módulo | Pin del módulo | Pin de la placa | Sesión |
|---|---|---|---|
| LED de estado | (integrado en la placa) | GPIO2 | 0 |
| PIR HC-SR501 | VCC | **VIN (5 V)** | 0 |
| | OUT | **GPIO27** | 0 |
| | GND | GND | 0 |
| DHT11 | VCC (+) | 3V3 | 1 |
| | DATA (S / OUT) | **GPIO4** | 1 |
| | GND (−) | GND | 1 |
| OLED SSD1306 0,96" (I2C) | VCC | 3V3 | 1 |
| | GND | GND | 1 |
| | SCL | **GPIO22** | 1 |
| | SDA | **GPIO21** | 1 |

## Notas por módulo

- **PIR HC-SR501** (cúpula blanca):
  - Los pines van bajo la cúpula, con la serigrafía tapada: quita la cúpula con cuidado para leer VCC / OUT / GND.
  - Su salida pasa a 1 al detectar movimiento y se queda en 1 un tiempo fijado por el potenciómetro **Tx** (time). Gíralo del todo a la izquierda para el mínimo (~3 s). El otro potenciómetro, **Sx**, es la sensibilidad (alcance).
  - Jumper de modo: **H** (repetible: la salida sigue en 1 mientras haya movimiento) es el recomendado; **L** da un solo pulso.
  - Tras alimentarlo necesita **~1 minuto** para estabilizarse: en ese tiempo puede dar disparos falsos.
  - Después de cada detección queda ciego unos 2-3 s.
  - Si en tu kit hubiera un sensor IR de obstáculos (activo a nivel bajo, a 3V3), selecciónalo en `menuconfig` → *Presence sensor type*.
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
