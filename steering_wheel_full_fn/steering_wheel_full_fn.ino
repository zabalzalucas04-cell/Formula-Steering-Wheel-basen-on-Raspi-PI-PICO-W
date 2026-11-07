#include <Adafruit_TinyUSB.h>

// Descriptor HID personalizado para 32 botones (Report ID 1 + 4 bytes)
uint8_t const desc_hid_report[] = {
  0x05, 0x01,  // Usage Page (Generic Desktop Ctrls)
  0x09, 0x05,  // Usage (Game Pad)
  0xA1, 0x01,  // Collection (Application)
  0x85, 0x01,  //   Report ID (1)
  0x05, 0x09,  //   Usage Page (Button)
  0x19, 0x01,  //   Usage Minimum (Button 1)
  0x29, 0x20,  //   Usage Maximum (Button 32)
  0x15, 0x00,  //   Logical Minimum (0)
  0x25, 0x01,  //   Logical Maximum (1)
  0x75, 0x01,  //   Report Size (1)
  0x95, 0x20,  //   Report Count (32)
  0x81, 0x02,  //   Input (Data, Var, Abs)
  0xC0         // End Collection
};

Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);

struct WheelReport {
  uint8_t report_id;
  uint8_t buttons[4];
};

WheelReport gp_report;
uint8_t last_sent_buttons[4] = {0, 0, 0, 0};

// Clase RotaryEncoder optimizada por cuadratura
class RotaryEncoder {
  uint8_t pinA, pinB;
  uint8_t last_state;
  long count;
public:
  RotaryEncoder(uint8_t a, uint8_t b) {
    pinA = a; pinB = b;
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    last_state = (digitalRead(pinA) << 1) | digitalRead(pinB);
    count = 0;
  }
  
  void tick() {
    uint8_t s = (digitalRead(pinA) << 1) | digitalRead(pinB);
    if (s != last_state) {
      if ((last_state == 0 && s == 1) || (last_state == 1 && s == 3) || 
          (last_state == 3 && s == 2) || (last_state == 2 && s == 0)) {
        count++;
      } else if ((last_state == 0 && s == 2) || (last_state == 2 && s == 3) || 
                 (last_state == 3 && s == 1) || (last_state == 1 && s == 0)) {
        count--;
      }
      last_state = s;
    }
  }
  
  long read() { return count / 4; }
  void resetCount() { count = 0; }
};

// Configuración de Pines (Hardware intacto)
const int buttonPins[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }; // GP0 a GP11 (12 Botones físicos/levas)
const int switchPins[3] = { 20, 21, 22 };                           // GP20 (Fn), GP21, GP22

RotaryEncoder enc1(14, 15); // Encoder 1 (Doble nivel + Fn)
RotaryEncoder enc2(16, 17); // Encoder 2 (Normal)
RotaryEncoder enc3(18, 19); // Encoder 3 (Normal)

// Temporizadores de holgura para los 12 botones físicos (evita perder pulsos rápidos)
unsigned long buttonReleaseTimer[12] = {0};

// Variables para Encoder 1 (Doble Nivel con Fn y restricción estricta)
long oldEnc1Pos = 0;
unsigned long enc1TimerNorm = 0;

int f1_state = 0;                  // 5 Estados (0 a 4 -> A, B, C, D, E)
const int max_f1_states = 5;       
unsigned long f1Timer = 0;
int activeF1Button = -1;

// Variables de control estricto para el modo Fn
bool fnWasPressed = false;
bool stateChangedThisPress = false;
long encoderBasePos = 0;

// Variables para Encoders 2 y 3
long oldEnc2Pos = 0;
long oldEnc3Pos = 0;
unsigned long encTimer2 = 0;
unsigned long encTimer3 = 0;

void setup() {
  usb_hid.begin();

  for (int i = 0; i < 12; i++) pinMode(buttonPins[i], INPUT_PULLUP);
  for (int i = 0; i < 3; i++) pinMode(switchPins[i], INPUT_PULLUP);

  gp_report.report_id = 1;
  memset(gp_report.buttons, 0, sizeof(gp_report.buttons));
  memset(last_sent_buttons, 0, sizeof(last_sent_buttons));

  while (!TinyUSBDevice.mounted()) {
    delay(10);
  }
}

void loop() {
  // 1. Muestreo continuo sin retardo
  enc1.tick();
  enc2.tick();
  enc3.tick();

  // 2. Lectura de los 12 Botones Físicos con sistema de holgura (40ms mínimo activos)
  for (int i = 0; i < 12; i++) {
    int byte_idx = i / 8;
    int bit_idx = i % 8;
    if (digitalRead(buttonPins[i]) == LOW) {
      gp_report.buttons[byte_idx] |= (1 << bit_idx);
      buttonReleaseTimer[i] = millis() + 40; // Asegura visibilidad de toques rápidos
    } else {
      if (millis() > buttonReleaseTimer[i]) {
        gp_report.buttons[byte_idx] &= ~(1 << bit_idx);
      }
    }
  }

  // 3. Lectura de los Switches 2 y 3 (GP21 y GP22) -> El GP20 (switch 1) ahora es Fn y no se reporta
  for (int i = 1; i < 3; i++) { // Empieza en 1 para omitir GP20
    int btn_num = 23 + i; // Botones 24 y 25
    int bit_abs = btn_num - 1;
    if (digitalRead(switchPins[i]) == LOW) {
      gp_report.buttons[bit_abs / 8] |= (1 << (bit_abs % 8));
    } else {
      gp_report.buttons[bit_abs / 8] &= ~(1 << (bit_abs % 8));
    }
  }

  // --- 4. ENCODER 1: LÓGICA DE DOBLE NIVEL CON CONTROL FN Y 60° ---
  long newEnc1 = enc1.read();
  bool isFnPressed = (digitalRead(switchPins[0]) == LOW); // GP20 actúa como modificador Fn

  if (isFnPressed) {
    // --- GESTIÓN DEL MODO SELECTOR (NIVEL 2) ---
    if (!fnWasPressed) {
      // Acaba de ser presionado el botón Fn: capturamos posición base y habilitamos 1 solo cambio
      fnWasPressed = true;
      stateChangedThisPress = false;
      encoderBasePos = newEnc1;
    }

    // Si aún no se ha realizado el cambio de estado para esta pulsación de Fn
    if (!stateChangedThisPress) {
      long diffFromBase = newEnc1 - encoderBasePos;
      
      // Umbral de 60° (aprox 3 cuentas de encoder)
      if (abs(diffFromBase) >= 3) {
        if (diffFromBase > 0) {
          f1_state++;
        } else {
          f1_state--;
        }

        // Ciclo continuo (Wrap-around: A -> B -> C -> D -> E -> A...)
        if (f1_state < 0) f1_state = max_f1_states - 1;
        else if (f1_state >= max_f1_states) f1_state = 0;

        activeF1Button = 15 + f1_state; // Mapeado a Botones 15 a 19 (Estados A al E)
        f1Timer = millis();
        
        // BLOQUEO ESTRICTO: Marcamos que ya cambió 1 estado; no permitirá más hasta soltar Fn
        stateChangedThisPress = true; 
      }
    }
    oldEnc1Pos = newEnc1; // Sincroniza posición
  } 
  else {
    // --- GESTIÓN DE USO NORMAL (NIVEL 1) CUANDO NO SE PRESIONA FN ---
    fnWasPressed = false;          // Resetea bandera al soltar Fn
    stateChangedThisPress = false;

    if (newEnc1 != oldEnc1Pos) {
      long diff = newEnc1 - oldEnc1Pos;
      if (diff > 0) {
        gp_report.buttons[1] |= (1 << 4); // Botón 13
      } else {
        gp_report.buttons[1] |= (1 << 5); // Botón 14
      }
      enc1TimerNorm = millis();
      oldEnc1Pos = newEnc1;
    }
  }

  // Apagado del Nivel 1 a los 60ms
  if (millis() - enc1TimerNorm > 60) {
    gp_report.buttons[1] &= ~((1 << 4) | (1 << 5));
  }

  // Apagado y limpieza estricta del Nivel 2 (Estados A al E) con pulso de 60ms
  for (int b = 15; b <= 19; b++) {
    int bit_abs = b - 1;
    gp_report.buttons[bit_abs / 8] &= ~(1 << (bit_abs % 8));
  }
  if (activeF1Button != -1 && (millis() - f1Timer < 60)) {
    int bit_abs_f1 = activeF1Button - 1;
    gp_report.buttons[bit_abs_f1 / 8] |= (1 << (bit_abs_f1 % 8));
  } else if (millis() - f1Timer >= 60) {
    activeF1Button = -1;
  }

  // --- 5. ENCODERS 2 Y 3 (PULSOS NORMALES CON PULSO DE 60ms) ---
  long newEnc2 = enc2.read();
  long newEnc3 = enc3.read();

  // Encoder 2 -> Botones 20 y 21
  if (newEnc2 != oldEnc2Pos) {
    if (newEnc2 > oldEnc2Pos) gp_report.buttons[2] |= (1 << 3);  // Botón 20
    else gp_report.buttons[2] |= (1 << 4);                      // Botón 21
    encTimer2 = millis();
    oldEnc2Pos = newEnc2;
  }
  if (millis() - encTimer2 > 60) {
    gp_report.buttons[2] &= ~((1 << 3) | (1 << 4));
  }

  // Encoder 3 -> Botones 22 y 23
  if (newEnc3 != oldEnc3Pos) {
    if (newEnc3 > oldEnc3Pos) gp_report.buttons[2] |= (1 << 5);  // Botón 22
    else gp_report.buttons[2] |= (1 << 6);                      // Botón 23
    encTimer3 = millis();
    oldEnc3Pos = newEnc3;
  }
  if (millis() - encTimer3 > 60) {
    gp_report.buttons[2] &= ~((1 << 5) | (1 << 6));
  }

  // --- 6. ENVÍO CONDICIONAL USB (OPTIMIZACIÓN MÁXIMA DE LATENCIA) ---
  bool hasChanged = false;
  for (int i = 0; i < 4; i++) {
    if (gp_report.buttons[i] != last_sent_buttons[i]) {
      hasChanged = true;
      break;
    }
  }

  if (hasChanged) {
    usb_hid.sendReport(1, &gp_report.buttons, sizeof(gp_report.buttons));
    memcpy(last_sent_buttons, gp_report.buttons, 4);
  }
}
