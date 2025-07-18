#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Define Screen Details
#define SDA 6
#define SCL 7
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels
#define OLED_RESET -1     // can set an oled reset pin if desired
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

// Define Voltage Sense Pins
#define VOLTAGE_SENSE_1 18
#define VOLTAGE_SENSE_2 19
#define VOLTAGE_SENSE_3 20

int weaponMode = 0;

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

  // Setup Program
  getWeapon(); // Check which weapon is selected

  // Setup Display
  display.begin(0x3D, true); // Address 0x3D default
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void getWeapon(void) {
  if (digitalRead(WEAPON_SELECT_A) == HIGH){
    weaponMode = 0; // EPPE
  } else if (digitalRead(WEAPON_SELECT_B) == HIGH){
    weaponMode = 2; // SABRE
  } else {
    weaponMode = 1; // FOIL
  }
}
