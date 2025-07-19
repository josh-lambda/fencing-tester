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

// Define Relays
#define RELAY_1_PIN 0
#define RELAY_2_PIN 1
#define RELAY_3_PIN 3
#define RELAY_4_PIN 2

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

int weaponMode = 0;    // 0, 1, 2; EPPE, FOIL, SABRE
int testMode = 0;      // 0, 1, 1; RESISTANCE, WEAPON, GENERATOR
int weaponStatus = 0;  // 0, 1, 2, 4; None, Off Target, On Target, Both

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

  pinMode(VOLTAGE_SENSE_1, INPUT);
  pinMode(VOLTAGE_SENSE_2, INPUT);
  pinMode(VOLTAGE_SENSE_2, INPUT);

  pinMode(WEAPON_TEST_1, INPUT);
  pinMode(WEAPON_TEST_3, INPUT);
  pinMode(LAME);

  // Setup Program
  getWeapon();  // Check which weapon is selected

  // Setup Display
  display.begin(0x3D, true);  // Address 0x3D default
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
  switch (weaponMode) {
    case 0:

      break;
    case 1:
      break;
    case 2:
      break;
  }
}

// ***** General Helpers *****
void getWeapon(void) {
  if (digitalRead(WEAPON_SELECT_A) == HIGH) {
    weaponMode = 0;  // EPPE
  } else if (digitalRead(WEAPON_SELECT_B) == HIGH) {
    weaponMode = 2;  // SABRE
  } else {
    weaponMode = 1;  // FOIL
  }
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
}

void measureResistance() {
  // Get reistances
  float res1 = calcResistance(analougeRead(VOLTAGE_SENSE_1));
  float res2 = calcResistance(analougeRead(VOLTAGE_SENSE_2));
  float res3 = calcResistance(analougeRead(VOLTAGE_SENSE_3));

  // Check resistances
  bool resPass1 = checkResistance(res1);
  bool resPass2 = checkResistance(res2);
  bool resPass3 = checkResistance(res3);
}

// ***** Weapon Helpers *****
void weaponMode() {
  // Set relay configs
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH);  // Can he either; HIGH gives 3 N/Cs instead of 1 N/C
}

int getWeaponStatus() {
  // For Eppe
  if (weaponMode == 0) {
    if (digitalRead(WEAPON_TEST_1) == HIGH) {
      weaponStatus = 0;  // Normal state
    } else {
      if (digitalRead(WEAPON_TEST_3) == HIGH) {
        weaponStatus = 0;  // Guard or piste hit
      } else {
        weaponStatus = 2;  // Hit
      }
    }
    // For Foil & Eppe
    else {
      // If tip is pressed
      if (digitalRead(WEAPON_TEST_3) == HIGH) {
        if (digitalRead(LAME) == HIGH) {
          weaponStatus = 2;  // HIT
        } else {
          weaponStatus = 1;  // Off target
        }
      } else {
        weaponStatus = 0;  // Normal state
      }
    }
  }
}

// ***** Generator Helper *****
void genMode() {
  // Set relay configs
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH);
}

// Foil & Sabre signals
void offTarget() {
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, HIGH); // Pin 2-3
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH); // Revise after fixing issue
}

void onTarget() {
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, HIGH); // Pin 2-3
  digitalWrite(RELAY_3_PIN, LOW);
  // Do lame after fixing issue
}

void noLight() {
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH); // Revise after fixing issue
}

// Eppe signals
void eppeNormal() {
  digitalWrite(RELAY_1_PIN, HIGH); // Pin 2-1
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH); // Revise after fixing issue
}

void eppeHit() {
  digitalWrite(RELAY_1_PIN, LOW); // Pin 2-1
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, HIGH); // Revise after fixing issue
}
