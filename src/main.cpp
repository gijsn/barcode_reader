#include <Arduino.h>

#define MAX_LEVEL 1024
#define HIGH_LEVEL 850
#define LOW_LEVEL 500
#define NOTHING 200
#define MARGIN 100

#define MAX_BARCODE_TIME 1000

uint16_t barcode = UINT16_MAX;

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
uint16_t bar_counter = 0;
uint32_t barcode_timeout = 0;

uint32_t time = 0;
#define DELAY 1

void check_barcode() {
  // check first 13 bits only
  uint16_t tmp_barcode = barcode & 0b1111111111111;
  // check where is the start
  Serial.print("barcode: ");
  for (int i = 12; i >= 0; i--) {
    Serial.print((tmp_barcode >> i) & 1);
  }
  Serial.print(" ");
  if (bar_counter < 13) {
    Serial.println("Not enough bars seen");
    return;
  }
  if ((tmp_barcode & 0b111) == 0) {
    // first three bits are zero
    // could be valid barcode
    // swap order

    Serial.print("valid ");
  } else if (((tmp_barcode >> 10) & 0b111) == 0) {
    // last three bits are zero
    // could be valid barcode
    Serial.print("reversed ");
    uint16_t reversed_barcode = 0;
    for (int i = 0; i < 13; i++) {
      if (tmp_barcode & (1 << i)) {
        reversed_barcode |= 1 << ((13 - 1) - i);
      }
    }
    tmp_barcode = reversed_barcode;
  } else {
    Serial.print(" not valid");
  }

  tmp_barcode = tmp_barcode >> 3;
  for (int i = 12; i >= 0; i--) {
    Serial.print((tmp_barcode >> i) & 1);
  }
  Serial.print(" ");

  // Verify checksum bits
  uint8_t count = 0;
  for (int i = 3; i < 10; i++) {
    if (((tmp_barcode >> i) & 0b1) == 1) {
      count++;
    }
  }
  Serial.print(count);
  count %= 2;
  if (((tmp_barcode >> 7) & 0b1) == count) {
    Serial.print(" pass1");
    if ((((tmp_barcode >> 8) & 0b1) ^ count) == 1) {
      Serial.print(" pass2");
      Serial.print(tmp_barcode >> 9);
      if (((tmp_barcode >> 9) & 0b1) == 1) {
        Serial.print(" xor valid");
        barcode = UINT16_MAX;
        bar_counter = 0;
        barcode_timeout = millis();
      }
    }
  }
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (time + DELAY > millis()) {
    return;
  }
  if (barcode_timeout + MAX_BARCODE_TIME < millis() && bar_counter > 0) {
    Serial.println("reset bar count");
    bar_counter = 0;
    barcode = UINT16_MAX;
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
      } else if (min_value < HIGH_LEVEL) {
        // small stripe
        // Serial.println("0");
        barcode = barcode << 1;
      }
      barcode_timeout = millis();
      bar_counter++;
      check_barcode();
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
