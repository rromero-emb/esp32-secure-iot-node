# Probar el firmware desde Windows

Este proyecto es **ESP-IDF**, no Arduino: el IDE de Arduino **no lo compila**. Aun así, en Windows hay dos formas de probarlo, y el monitor serie del IDE de Arduino sigue sirviendo para ver los logs.

| Opción | Qué instalas | Cuándo usarla |
|---|---|---|
| **A. Flasheo web del binario de la CI** | Nada (solo Chrome o Edge) | Probar cada versión que se sube al repo |
| **B. ESP-IDF en Windows** | ESP-IDF v5.5 (~2 GB) | Sesiones con muchos cambios seguidos (drivers, depuración) |

## 0. Preparación (una sola vez)

1. **Cable USB de datos.** Muchos cables de los kits solo cargan. Si Windows no hace ningún sonido al conectar la placa, prueba con otro.
2. **Driver USB-serie.** Abre el *Administrador de dispositivos* → *Puertos (COM y LPT)*. Debe aparecer algo como `Silicon Labs CP210x (COM3)` o `USB-SERIAL CH340 (COM4)`. Si aparece como dispositivo desconocido, instala el driver del chip:
   - CP210x: Silicon Labs, "CP210x USB to UART Bridge VCP Drivers".
   - CH340: WCH, "CH341SER".
3. **Apunta el número de puerto COM.** Lo usarás en todos los pasos.
4. **Montaje:** ver [MONTAJE.md](MONTAJE.md). Para la sesión 0 basta con el sensor IR.

## A. Flashear el binario que compila la CI (sin instalar nada)

Cada push a GitHub compila el firmware y guarda el resultado como *artifact*.

1. Entra en GitHub con la cuenta **rromero-emb** (los artifacts solo se descargan con sesión iniciada).
2. Repo → pestaña **Actions** → la ejecución más reciente en verde → sección **Artifacts** → descarga `firmware-esp32` (un .zip).
3. Descomprímelo. Dentro está **`merged-firmware.bin`**: bootloader, tabla de particiones y aplicación en un solo fichero.
4. Abre en **Chrome o Edge** el flasheador web oficial de Espressif: <https://espressif.github.io/esptool-js/>
5. *Baudrate* 460800 → **Connect** → elige el puerto COM de la placa.
   - Si no conecta: mantén pulsado el botón **BOOT** de la placa mientras pulsas Connect y suéltalo cuando empiece.
6. En *Flash Address* pon **`0x0`**, selecciona `merged-firmware.bin` y pulsa **Program**.
   - La primera vez, pulsa antes **Erase Flash** para empezar limpio.
7. Al terminar, pulsa el botón **EN** (reset) de la placa.

> El flasheador web no toca los eFuses: solo escribe la flash. Es seguro para la placa del kit.

## Ver los logs con el monitor serie del IDE de Arduino

1. Cierra el flasheador web (o pulsa *Disconnect*): **el puerto COM solo lo puede usar un programa a la vez**.
2. IDE de Arduino → *Herramientas* → *Puerto* → el COM de la placa.
3. *Herramientas* → *Monitor serie*, a **115200 baudios**.
4. Pulsa **EN** en la placa para ver el arranque completo.

También vale PuTTY (conexión *Serial*, 115200) o el monitor de `idf.py` de la opción B.

## B. Desarrollar con ESP-IDF en Windows

Necesario cuando hay que compilar, flashear y probar muchas veces seguidas (sesión 1 en adelante).

1. Instala **ESP-IDF v5.5** con el instalador oficial para Windows. Sigue la guía *Get Started → Windows* de <https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/get-started/>.
   - Alternativa cómoda: **VS Code** con la extensión **ESP-IDF** de Espressif, que instala lo mismo desde un asistente.
2. Clona el repo en una ruta **corta y sin espacios** (ESP-IDF da problemas con rutas largas):
   ```powershell
   git clone https://github.com/rromero-emb/esp32-secure-iot-node.git C:\dev\esp32-secure-iot-node
   ```
3. Abre la consola **"ESP-IDF 5.5 PowerShell"** que crea el instalador y ejecuta:
   ```powershell
   cd C:\dev\esp32-secure-iot-node
   idf.py set-target esp32
   idf.py build
   idf.py -p COM3 flash monitor
   ```
   Cambia `COM3` por tu puerto. Para salir del monitor: **Ctrl+]**.
4. Para traer los últimos cambios: `git pull`.

**Si haces commits desde Windows:** configura el repo con la cuenta personal **rromero-emb**, nunca con la del trabajo:
```powershell
git config user.name "Roberto Romero Justiniano"
git config user.email "romerojustiniano.roberto@gmail.com"
```

## Flujo de trabajo entre los dos equipos

1. En Linux (con Claude Code) se escribe el código y se hace push.
2. La CI de GitHub lo compila (3–5 min).
3. En Windows: `git pull` + `idf.py flash monitor` (opción B) o flasheo web del artifact (opción A).
4. Se copia la salida del monitor serie y se pega en la conversación para corregir lo que falle.

La lista de comprobaciones de cada sesión está en [PRUEBAS.md](PRUEBAS.md).

## Problemas típicos

| Síntoma | Causa probable | Solución |
|---|---|---|
| No aparece ningún puerto COM | Cable solo de carga o falta el driver | Otro cable; instalar CP210x o CH340 |
| `Failed to connect` / `Timed out waiting for packet header` | La placa no entra en modo descarga | Mantener **BOOT** pulsado al conectar |
| `Access denied` / puerto ocupado | El monitor de Arduino u otro programa tiene el puerto abierto | Cerrarlo antes de flashear |
| Caracteres basura en el monitor | Velocidad incorrecta | 115200 baudios |
| Reinicios en bucle con `Brownout detector was triggered` | Alimentación USB insuficiente | Otro puerto USB o cable más corto; no alimentar periféricos a 5 V desde la placa |
