# 🕹️ Guía de Configuración e Interpretación de Botones (SimRacing Firmware)

Esta guía detalla los pasos exactos para configurar el entorno de compilación e interpretar el mapeo de botones USB detectado por el sistema operativo Windows y los simuladores de carreras (*Assetto Corsa*, *iRacing*, *F1*, etc.).

---

## ⚙️ 1. Configuración del Entorno de Desarrollo (Arduino IDE)

Para garantizar la correcta compilación de los descriptores HID nativos, sigue estos pasos:

1. **Instalar el Administrador de Placas:**
   * Abre Arduino IDE y ve a `Archivo` > `Preferencias`.
   * En *Gestor de URLs Adicionales de Tarjetas*, añade:  
     `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
   * Ve a `Herramientas` > `Placa` > `Gestor de Tarjetas`, busca **Raspberry Pi Pico/RP2040** por *Earle F. Philhower* e instálalo.

2. **Instalar Librería USB:**
   * Ve a `Herramientas` > `Administrar Librerías...`
   * Busca e instala **Adafruit TinyUSB Library**.

3. **Ajustes de Selección en el Menú `Herramientas`:**
   * **Placa:** `Raspberry Pi Pico / Pico W`
   * **CPU Speed:** `133 MHz` (o `200 MHz` si deseas *overclocking* estable).
   * **USB Stack (CRÍTICO):** `Adafruit TinyUSB` *(Si no seleccionas esta opción, el compilador no reconocerá los comandos USB HID)*.

---

## 🗺️ 2. Mapeo de Entradas en el Panel de Control de Windows (`joy.cpl`)

Para probar la respuesta del volante antes de entrar al simulador:
1. Presiona la combinación de teclas `Win + R`.
2. Escribe `joy.cpl` y presiona `Enter`.
3. Selecciona tu dispositivo de juego y haz clic en **Propiedades**.

### Tabla Completa de Asignación de 32 Botones:

| Número de Botón (Windows) | Componente Físico / Acción | Tipo de Pulsación |
| :---: | :--- | :--- |
| **Botón 1** | Leva Superior Izquierda (`GP0`) | Retenida (LOW activo) |
| **Botón 2** | Leva Inferior Izquierda (`GP1`) | Retenida (LOW activo) |
| **Botón 3** | Leva Superior Derecha (`GP2`) | Retenida (LOW activo) |
| **Botón 4** | Leva Inferior Derecha (`GP3`) | Retenida (LOW activo) |
| **Botones 5 al 12** | Botones Frontales (`GP4` a `GP11`) | Retenida (LOW activo) |
| **Botón 13** | Encoder 1 - Giro Izquierda (Nivel 1) | Pulso temporal de 60 ms |
| **Botón 14** | Encoder 1 - Giro Derecha (Nivel 1) | Pulso temporal de 60 ms |
| **Botón 15** | Selector F1 - Estado A (Posición 1) | Pulso de 60 ms (Con `Fn` + Giro 60°) |
| **Botón 16** | Selector F1 - Estado B (Posición 2) | Pulso de 60 ms (Con `Fn` + Giro 60°) |
| **Botón 17** | Selector F1 - Estado C (Posición 3) | Pulso de 60 ms (Con `Fn` + Giro 60°) |
| **Botón 18** | Selector F1 - Estado D (Posición 4) | Pulso de 60 ms (Con `Fn` + Giro 60°) |
| **Botón 19** | Selector F1 - Estado E (Posición 5) | Pulso de 60 ms (Con `Fn` + Giro 60°) |
| **Botón 20** | Encoder 2 - Giro Izquierda | Pulso temporal de 60 ms |
| **Botón 21** | Encoder 2 - Giro Derecha | Pulso temporal de 60 ms |
| **Botón 22** | Encoder 3 - Giro Izquierda | Pulso temporal de 60 ms |
| **Botón 23** | Encoder 3 - Giro Derecha | Pulso temporal de 60 ms |
| **N/A** | Switch Encoder 1 (`GP20`) | Reservado (Tecla modificadora `Fn`) |
| **Botón 24** | Switch Encoder 2 (`GP21`) | Retenida (Pulsador interno) |
| **Botón 25** | Switch Encoder 3 (`GP22`) | Retenida (Pulsador interno) |
