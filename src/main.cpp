#include <Arduino.h>

#define MAX_LEVEL 1024
#define HIGH_LEVEL 850
#define LOW_LEVEL 500
#define NOTHING 200
#define MARGIN 100

uint16_t barcode = 0;
void enter_buffer(bool input);
void setup() {
  // put your setup code here, to run once:
  pinMode(A0, INPUT);
  Serial.begin(115200);
  Serial.println("Starting reading sensor");
}
uint8_t value = 255;
bool value_changed = false;
uint16_t min_value = 1024;
bool reading_bit = false;
bool seen_white = false;

uint32_t time = 0;
#define DELAY 1

void check_barcode() {
  // check first 13 bits only
  uint16_t tmp_barcode = barcode & 0b1111111111111;
  // check where is the start
  Serial.print("barcode: ");
  for (int i = 0; i < 16; i++) {
    Serial.print((barcode >> i) & 1);
  }
  Serial.println();
  return;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (time + DELAY > millis()) {
    return;
  }
  time = millis();
  uint16_t sensor_value = analogRead(A0);
  // Serial.println(sensor_value);
  // return;
  if (sensor_value < NOTHING) {
    // we see no card, resetting values
    reading_bit = false;
    min_value = MAX_LEVEL;
    seen_white = false;
  } else if (sensor_value > (HIGH_LEVEL + MARGIN)) {
    if (!seen_white) {
      Serial.println("Starting code");
      seen_white = true;
    }
    // we see only white, check if we were reading a bit
    if (reading_bit) {
      Serial.print(min_value);
      Serial.print(" ");
      if (min_value < LOW_LEVEL) {
        // wide stripe
        // Serial.println("1");
        barcode = barcode << 1 | 1;
        check_barcode();
      } else if (min_value < HIGH_LEVEL) {
        // small stripe
        // Serial.println("0");
        barcode = barcode << 1;
        check_barcode();
      }
      // reset values
      reading_bit = false;
      min_value = MAX_LEVEL;
    }
  } else {
    // there is a card, and the current piece is not white
    if (seen_white) {
      reading_bit = true;
      min_value = min(min_value, sensor_value);
    }
  }
}
