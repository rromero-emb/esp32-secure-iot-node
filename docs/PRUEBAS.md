# Comprobaciones en la placa por sesión

Cómo flashear y ver los logs desde Windows: [WINDOWS.md](WINDOWS.md). Montaje: [MONTAJE.md](MONTAJE.md).

Cuando algo no coincida, copia **toda la salida del monitor serie desde el arranque** (pulsa EN) y pégala en la conversación.

## Sesión 0: esqueleto

Montaje: solo el PIR (VCC → **VIN**, GND → GND, OUT → GPIO27). Espera ~1 minuto tras enchufar antes de probarlo.

- [ ] Al arrancar aparece la versión:
  ```
  I (...) node: esp32-secure-iot-node 0.1.0 (ESP-IDF v5.5...)
  ```
- [ ] Avisos esperados, porque los drivers son de la sesión 1:
  ```
  W (...) node: DHT11 unavailable: ESP_ERR_NOT_SUPPORTED
  W (...) node: OLED unavailable: ESP_ERR_NOT_SUPPORTED
  ```
- [ ] El LED azul de la placa (GPIO2) parpadea a **1 Hz** (estado IDLE).
- [ ] Al moverte delante del PIR:
  ```
  I (...) node: PIR sensor: motion
  ```
  y, unos segundos después de quedarte quieto (lo que marque el potenciómetro Tx):
  ```
  I (...) node: PIR sensor: idle
  ```
- [ ] Un solo par motion/idle por movimiento (el antirrebote de 30 ms funciona). Si salen varios seguidos, apúntalo.
- [ ] No hay reinicios (`rst:` repetidos, `Guru Meditation`, `Task watchdog`).

## Sesión 1: drivers DHT11 y OLED

Montaje: todo el kit (ver tabla de MONTAJE.md).

- [ ] Ya no aparecen los avisos `unavailable`. Si sale `OLED unavailable: ESP_ERR_NOT_FOUND`, la pantalla no responde en 0x3C: revisa SDA/SCL (no estén cruzados) y prueba 0x3D en menuconfig.
- [ ] Cada 5 s:
  ```
  I (...) node: T=23 C RH=45 %
  ```
  con valores razonables para la habitación (el DHT11 tiene ±2 °C y ±5 % HR).
- [ ] Al soplar sobre el DHT11, la humedad sube en la siguiente lectura.
- [ ] El OLED muestra la versión del firmware, la temperatura y la humedad, y se actualiza cada 5 s.
- [ ] Desconectando el DHT11 en marcha sale `DHT11 read failed: ESP_ERR_TIMEOUT` y el resto sigue funcionando; al reconectarlo vuelven las lecturas.
- [ ] Si salen errores `ESP_ERR_INVALID_CRC` o `ESP_ERR_INVALID_RESPONSE` de vez en cuando con el sensor conectado, apunta cuántos por cada 20 lecturas.
- [ ] Foto del montaje → `docs/img/montaje.jpg`.

## Sesión 2: BLE y MQTT

- [ ] El LED parpadea rápido (PROVISIONING) mientras no hay Wi-Fi configurada.
- [ ] La app **ESP BLE Provisioning** (Android/iOS, de Espressif) encuentra el nodo y le pasa la Wi-Fi.
- [ ] El LED queda fijo (CONNECTED) y la telemetría llega al broker MQTT con TLS.
- [ ] Tras un reinicio se conecta solo, sin volver a aprovisionar.
- [ ] Apagando el router, reconecta solo al volver.

## Sesión 3: OTA firmada

- [ ] Una imagen firmada nueva se instala y arranca con la versión nueva.
- [ ] Una imagen **sin firmar** se rechaza y el nodo sigue en la versión actual.
- [ ] Una imagen **corrupta** (cortada a medias) se rechaza.
- [ ] Una imagen que falla su autocomprobación tras arrancar vuelve sola a la versión anterior (rollback).
- [ ] **No** se ejecuta nada de `espefuse` ni se activa Secure Boot / Flash Encryption.

## Sesión 4: bajo consumo

- [ ] El nodo entra en *deep sleep* y se despierta por temporizador y por el PIR.
- [ ] Medida de corriente anotada, indicando cómo se midió (la DevKit entera consume varios mA por el conversor USB-serie y el regulador).

## Sesión 5: evidencias

- [ ] GIF o vídeo corto con: aprovisionamiento, datos en el OLED y en MQTT, OTA aceptada y rechazada.
