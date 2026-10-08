# Nodo IoT seguro con ESP32: plan técnico

## 1. Objetivo

Proyecto de portfolio (GitHub + Upwork) que demuestre firmware ESP32 **de producción** con ESP-IDF, no con Arduino. Cubre las palabras clave más pedidas en Upwork: ESP32 (40 % de las ofertas de embebidos), BLE, Wi-Fi, MQTT, OTA, bajo consumo y FreeRTOS.

Se presenta junto a `cra-embedded-linux-demo` con un mensaje común: **arranque y actualizaciones firmadas y SBOM, del microcontrolador a Linux**.

**Criterio de éxito.** El vídeo o GIF del README muestra cuatro cosas:
1. el nodo se aprovisiona por BLE desde el móvil;
2. publica telemetría por MQTT con TLS y el OLED muestra los datos;
3. una OTA firmada se instala, mientras que una imagen sin firmar o corrupta se rechaza y vuelve a la versión anterior;
4. el consumo en *deep sleep* aparece medido.

## 1b. Flujo de trabajo y documentos

- **Desarrollo:** el código se escribe en Linux y se sube a GitHub; la CI lo compila en cada push.
- **Pruebas con la placa:** en el PC con Windows, flasheando el binario de la CI desde el navegador o con ESP-IDF instalado. Los logs se ven con el monitor serie del IDE de Arduino.

| Documento | Contenido |
|---|---|
| [WINDOWS.md](WINDOWS.md) | Drivers, flasheo web, ESP-IDF en Windows, monitor serie, problemas típicos |
| [MONTAJE.md](MONTAJE.md) | Cableado de cada módulo del kit y pines a evitar |
| [PRUEBAS.md](PRUEBAS.md) | Lista de comprobaciones en la placa para cada sesión, con los logs esperados |

## 2. Decisiones de diseño

| Decisión | Motivo |
|---|---|
| ESP-IDF v5.5, C, sin el core de Arduino | Es lo que piden los clientes que pagan bien. Arduino es justo lo que el proyecto quiere superar. |
| Drivers propios como componentes | Demuestra bajo nivel: RMT para tiempos estrictos, la API nueva de I2C master, ISR con cola. |
| Pines por Kconfig | Cada kit cablea distinto; nada fijo en el código. |
| Tabla de particiones con 2 OTA desde el inicio | La sesión 3 no obliga a reparticionar. |
| Firma de imágenes **sin** Secure Boot por hardware | Secure Boot y Flash Encryption queman eFuses de forma irreversible. Se documenta la configuración de producción sin aplicarla. |
| Broker Mosquitto en Docker (AWS IoT Core opcional) | Reproducible para cualquiera; AWS como extra porque aparece en ofertas. |
| CI con `espressif/esp-idf-ci-action` | Compila en cada push sin instalar nada en local. |

## 3. Sesiones (~4 h cada una)

| Sesión | Entregable | Tareas |
|---|---|---|
| 0 ✅ | Esqueleto | Estructura ESP-IDF, Kconfig, `status_led`, sensor de presencia (PIR), stubs de `dht11` y `ssd1306`, CI de compilación, README |
| 1 🔧 | Drivers | DHT11 con RMT (TX pulso de inicio, RX 83 pulsos, checksum); SSD1306 con `i2c_master` (frame buffer de 1 KB, fuente 8×8); comprobar en la placa y hacer foto del montaje |
| 2 | Conectividad | `network_provisioning` por BLE (security 1), MQTT con TLS (`esp-mqtt`), topics `node/<id>/telemetry` y `node/<id>/event`, reconexión; LED con los estados |
| 3 | OTA | `esp_https_ota`, imágenes firmadas (`CONFIG_SECURE_SIGNED_APPS_NO_SECURE_BOOT`), rollback (`CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE`) con autocomprobación tras arrancar; probar imagen sin firmar y corrupta |
| 4 | Bajo consumo y pruebas | *Deep sleep* con temporizador y PIR (`ext0`, activo a nivel alto) como fuentes de despertar; medir la corriente (ver nota); tests Unity de los decodificadores (DHT11, fuente); ejecución en QEMU de Espressif dentro de la CI |
| 5 | Evidencias y portfolio | SBOM con `esp-idf-sbom`, informe de CVE en la CI; README final con diagrama y GIF; entrada de portfolio en Upwork |

## 4. Riesgos y cómo evitarlos

- **eFuses:** no ejecutar `espefuse.py burn_*` ni activar Secure Boot o Flash Encryption en la placa del kit. Si se quiere demostrar, usar una placa de repuesto.
- **Medición de consumo:** el conversor USB-serie y el regulador de la DevKit consumen varios mA. Lo honesto es medir el módulo alimentándolo directamente, o indicarlo en el README.
- **DHT11:** necesita al menos 1 s entre lecturas, y su precisión es de ±2 °C y ±5 % de humedad relativa. No venderlo como sensor de precisión.
- **Secretos:** credenciales y claves fuera del repo (`.gitignore` ya excluye `*.pem` y `*.key`).

## 5. Qué se cuenta en Upwork

Entrada de portfolio con el esquema problema → stack → resultado:
> Production-style ESP32 firmware (ESP-IDF, FreeRTOS): custom RMT/I2C drivers, BLE provisioning, MQTT/TLS, signed OTA with rollback, deep sleep at X µA, CI + SBOM.

Abre la puerta a encargos pequeños para conseguir valoraciones: corregir firmware ESP32, portar de Arduino a ESP-IDF, añadir OTA o MQTT a un proyecto existente.
