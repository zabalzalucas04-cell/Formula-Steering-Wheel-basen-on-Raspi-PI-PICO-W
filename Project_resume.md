# 🏎️ Volante SimRacing Custom (Fórmula / GT) - Raspberry Pi Pico W

Firmware de ultra baja latencia y alto rendimiento desarrollado en **Arduino IDE** utilizando la pila USB nativa **Adafruit TinyUSB** para transformar una **Raspberry Pi Pico W (RP2040)** en un controlador USB HID Gamepad nativo de 32 botones.

---

## 📌 Características Principales
* **Conectividad Plug & Play nativa:** Reconocido inmediatamente por Windows como un dispositivo de juegos (`joy.cpl`) y compatible con cualquier simulador (*Assetto Corsa, iRacing, Automobilista, etc.*).
* **Ultra Baja Latencia (0 ms de bloqueo):** Lectura continua sin retardos bloqueantes (`delay`) y sistema de **envío condicional USB** (solo transmite paquetes cuando ocurre un cambio real, evitando saturar el bus).
* **Holgura de Lectura (Button Stretch):** Temporizadores de retención de 40 ms para los botones físicos, asegurando que toques rápidos en las levas nunca se pierdan.
* **Encoder de Doble Nivel con Modificador `Fn`:**
  - **Nivel 1 (Giro libre):** Envía pulsos rápidos de giro (Izq/Der).
  - **Nivel 2 (Selector cíclico de 5 estados: A, B, C, D, E):** Se activa manteniendo presionado el botón `Fn` (Switch 1) y girando al menos $60^\circ$. Posee un bloqueo estricto de 1 solo cambio por pulsación para evitar errores en plena carrera.
* **Comportamiento Cíclico (*Wrap-Around*):** Permite giros infinitos sin desincronizarse.

---

## 🛠️ Distribución de Pines (Pinout Físico)

| Componente | Pines en la Pico W (RP2040) | Función / Mapeo en Windows |
| :--- | :--- | :--- |
| **4 Levas** | `GP0`, `GP1`, `GP2`, `GP3` | Botones 1 al 4 (Entrada retenida) |
| **8 Botones Frontales** | `GP4` a `GP11` | Botones 5 al 12 (Entrada retenida) |
| **Encoder 1 (A / B)** | `GP14`, `GP15` | Giro normal (Botones 13-14) / Selector Nivel 2 (Botones 15-19) |
| **Encoder 2 (A / B)** | `GP16`, `GP17` | Giro normal (Botones 20-21) |
| **Encoder 3 (A / B)** | `GP18`, `GP19` | Giro normal (Botones 22-23) |
| **Switch Encoder 1** | `GP20` | Modificador `Fn` por software (No emite botón constante) |
| **Switch Encoder 2** | `GP21` | Botón 24 (Entrada retenida digital) |
| **Switch Encoder 3** | `GP22` | Botón 25 (Entrada retenida digital) |

*(Nota: Todos los segundos terminales de los botones y switches se conectan a cualquier pin `GND` de la placa).*

---

## ⚙️ Requisitos de Instalación (Arduino IDE)

1. **Gestor de Placas:** Instalar el núcleo **Raspberry Pi Pico/RP2040** por Earle F. Philhower.
2. **Pila USB (CRÍTICO):** En el Arduino IDE, ir a `Herramientas` > `USB Stack` > Seleccionar ***Adafruit TinyUSB***.
3. **Librerías:** Instalar `Adafruit TinyUSB Library` desde el gestor de librerías.

---

## 🚀 Cómo crear tu repositorio en GitHub
1. Entra a [GitHub](https://github.com) y haz clic en **New repository**.
2. Dale un nombre (ej: `pico-simracing-wheel`).
3. Selecciona **Public**, marca la opción **Add a README file** y haz clic en **Create repository**.
4. Sube estos dos archivos (`README.md` y el código fuente `.ino`) dentro del repositorio. ¡Listo!