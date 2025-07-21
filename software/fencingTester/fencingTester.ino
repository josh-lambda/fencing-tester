#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Define Screen Details
#define SDA 6
#define SCL 7
#define SCREEN_WIDTH 128   // OLED display width, in pixels
#define SCREEN_HEIGHT 128  // OLED display height, in pixels
#define OLED_RESET -1      // can set an oled reset pin if desired
Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);
#define SCREEN_TOP 16

// Define Relays
#define RELAY_1_PIN 0
#define RELAY_2_PIN 1
#define RELAY_3_PIN 3
#define RELAY_4_PIN 2
#define RELAY_5_PIN 21

// Define buttons & switches
#define BTN_USE 5
#define WEAPON_SELECT_A 10
#define WEAPON_SELECT_B 11

// Define Weapon Test Pins
#define WEAPON_TEST_1 8
#define WEAPON_TEST_3 9
#define LAME 4

// Define Voltage Sense Pins
#define VOLTAGE_SENSE_1 18
#define VOLTAGE_SENSE_2 19
#define VOLTAGE_SENSE_3 20

// ENUMS
enum Weapon {
  EPPE,
  FOIL,
  SABRE
};

enum Mode {
  RESISTANCE,
  WEAPON,
  GENERATOR
};

enum Light {
  NONE,
  OFF_TARGET,
  HIT,
  GUARD
};

// Status variables
Weapon weaponMode = FOIL;
Mode testMode = RESISTANCE;
Light weaponStatus = NONE;
bool inputRequest = false;  // Has the user requested input?
int genCycle = 0;

// Key Functions
void setup() {
  Serial.begin(9600);

  // Set PinModes
  pinMode(BTN_USE, INPUT);
  pinMode(WEAPON_SELECT_A, INPUT);
  pinMode(WEAPON_SELECT_B, INPUT);

  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  pinMode(RELAY_4_PIN, OUTPUT);
  pinMode(RELAY_5_PIN, OUTPUT);

  pinMode(VOLTAGE_SENSE_1, INPUT);
  pinMode(VOLTAGE_SENSE_2, INPUT);
  pinMode(VOLTAGE_SENSE_2, INPUT);

  pinMode(WEAPON_TEST_1, INPUT);
  pinMode(WEAPON_TEST_3, INPUT);
  pinMode(LAME, INPUT);

  // Setup Program
  getWeapon();  // Check which weapon is selected

  // Setup Display
  display.begin(0x3D, true);  // Address 0x3D default
  display.display();
  delay(2000);
  display.clearDisplay();

  // Setup interupts
  attachInterrupt(BTN_USE, buttonPressed, RISING);
  attachInterrupt(WEAPON_SELECT_A, weaponChange, CHANGE);
  attachInterrupt(WEAPON_SELECT_B, weaponChange, CHANGE);
}

void loop() {
  if (inputRequest) {
    testMode = Mode((int(testMode) + 1) % 3);  // Loop through the modes
    inputRequest = false;

    // Set relays and variables correctly
    switch (testMode) {
      case RESISTANCE:
        resistanceMode();
        break;
      case WEAPON:
        weaponModeSet();
        break;
      case GENERATOR:
        genMode();
        break;
    }
  }

  switch (testMode) {
    case RESISTANCE:
      resUpdate();
      break;
    case WEAPON:
      weaponUpdate();
      break;
    case GENERATOR:
      genUpdate();
      break;
  }
}

// ***** General Helpers *****
void getWeapon(void) {
  if (digitalRead(WEAPON_SELECT_A) == HIGH) {
    weaponMode = EPPE;  // EPPE
  } else if (digitalRead(WEAPON_SELECT_B) == HIGH) {
    weaponMode = SABRE;  // SABRE
  } else {
    weaponMode = FOIL;  // FOIL
  }
}

void buttonPressed(void) {
  inputRequest = true;
}

void weaponChange(void) {
  getWeapon();  // Update the selected weapon
}

void drawLamp(Light status) {
  switch (status) {
    case NONE:
      break;
    case OFF_TARGET:
      display.drawRect(2, SCREEN_TOP + 2, 124, 40, SH110X_WHITE);
      break;
    case HIT:
      display.fillRect(2, SCREEN_TOP + 2, 124, 40, SH110X_WHITE);
      break;
  }
}

void drawHeaders() {
  display.setCursor(0, 0);
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2);

  switch (testMode) {
    case RESISTANCE:
      display.print("RESISTANCE");
      break;
    case WEAPON:
      display.print("WEAPON TEST");
      break;
    case GENERATOR:
      display.print("TEST SIGNALS");
      break;
  }

  display.setCursor(0, 90);
  switch (weaponMode) {
    case FOIL:
      display.print("FOIL");
      break;
    case EPPE:
      display.print("EPPE");
      break;
    case SABRE:
      display.print("SABRE");
      break;
  }
}

void clearScreen(){
  display.clearDisplay();
  drawHeaders();
}

// ***** Resistance Helpers *****
float calcResistance(int voltageReading) {
  float voltage = voltageReading * (3.3 / 1023.0);  // Convert analouge read to voltage
  float resistance = 1 * ((3.3 / voltage) - 1.0);   // Calc resistance from voltage divider
  return resistance;
}

bool checkResistance(float resistance) {
  if (resistance <= 1) {
    return true;
  } else {
    return false;
  }
}

void resistanceMode() {
  // Set relay configs
  digitalWrite(RELAY_1_PIN, HIGH);
  digitalWrite(RELAY_2_PIN, HIGH);
  digitalWrite(RELAY_3_PIN, HIGH);
  digitalWrite(RELAY_4_PIN, LOW);
  digitalWrite(RELAY_5_PIN, LOW);
}

void resUpdate() {
  // Get reistances
  bool res1 = checkResistance(calcResistance(analogRead(VOLTAGE_SENSE_1)));
  bool res2 = checkResistance(calcResistance(analogRead(VOLTAGE_SENSE_2)));
  bool res3 = checkResistance(calcResistance(analogRead(VOLTAGE_SENSE_3)));

  // Check resistances
  bool res[3] = { res1, res2, res3 };

  int offset = 0;

  clearScreen();
  for (int i = 0; i < 3; i++) {
    if (res[i]) {
      display.fillCircle(21 + offset, SCREEN_TOP + 19, 20, SH110X_WHITE);
    } else {
      display.drawTriangle(2 + offset, SCREEN_TOP + 40, 21 + offset, SCREEN_TOP, 42 + offset, SCREEN_TOP + 40, SH110X_WHITE);  // Equilateral triangle within 40x40 box
    }
    offset += 42;
  }
  delay(100);
}


// ***** Weapon Helpers *****
void weaponModeSet() {
  // Set relay configs
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH);
  digitalWrite(RELAY_5_PIN, LOW);
}

void getWeaponStatus() {
  // For Eppe
  if (weaponMode == EPPE) {
    if (digitalRead(WEAPON_TEST_1) == HIGH) {
      weaponStatus = NONE;
    } else {
      if (digitalRead(WEAPON_TEST_3) == HIGH) {
        weaponStatus = NONE;  // Guard or piste hit
      } else {
        weaponStatus = HIT;
      }
    }
  } else {  // For Foil & Sabre
    // If tip is pressed
    if (digitalRead(WEAPON_TEST_3) == HIGH) {
      if (digitalRead(LAME) == HIGH) {
        weaponStatus = HIT;
      } else {
        weaponStatus = OFF_TARGET;
      }
    } else {
      weaponStatus = NONE;
    }
  }
}


void weaponUpdate() {
  getWeaponStatus();
  clearScreen();

  drawLamp(weaponStatus);

  delay(50);
}

// ***** Generator Helpers *****
void genMode() {
  // Set relay configs
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH);
  digitalWrite(RELAY_5_PIN, LOW);
}

void genUpdate() {
  clearScreen();

  if (weaponMode == EPPE) {
    switch (genCycle) {
      case 0:
        eppeNormal();
        drawLamp(NONE);
        break;
      case 1:
        eppeHit();
        drawLamp(HIT);
        break;
    }

    genCycle++;
    genCycle = genCycle % 2;
  } else {  // Foil & Sabre
    switch (genCycle) {
      case 0:
        noLight();
        drawLamp(NONE);
        break;
      case 1:
        offTarget();
        drawLamp(OFF_TARGET);
        break;
      case 2:
        onTarget();
        drawLamp(HIT);
        break;
    }

    genCycle++;
    genCycle = genCycle % 3;
  }

  delay(3000);
}

// Foil & Sabre signals
void offTarget() {
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, HIGH);  // Pin 2-3
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH);  // Lame
  digitalWrite(RELAY_5_PIN, LOW);   // Lame
}

void onTarget() {
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, HIGH);  // Pin 2-3
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH);  // Lame
  digitalWrite(RELAY_5_PIN, HIGH);  // Lame
}

void noLight() {
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH);  // Lame
  digitalWrite(RELAY_5_PIN, LOW);   // Lame
}

// Eppe signals
void eppeNormal() {
  digitalWrite(RELAY_1_PIN, HIGH);  // Pin 2-1
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH);  // Lame
  digitalWrite(RELAY_5_PIN, LOW);   // Lame
}

void eppeHit() {
  digitalWrite(RELAY_1_PIN, LOW);  // Pin 2-1
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH);  // Lame
  digitalWrite(RELAY_5_PIN, LOW);   // Lame
}
