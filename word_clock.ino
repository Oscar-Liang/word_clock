/*

*/

#include <Wire.h>
#include <RTClib.h>

#define BUTTON_CYCLE_SPEED 6000
#define CYCLE_SPEED 10000

#define ADDRESS_7SEG 0x71

#define BYTE_RESET_DISPLAY 0x76
#define BYTE_DECIMAL_CONTROL 0x77
#define BYTE_CURSOR_CONTROL 0x79
#define BYTE_COLON 0b00010000

#define PIN_HR_UP A0
#define PIN_HR_DOWN A1
#define PIN_MIN_UP A2
#define PIN_MIN_DOWN A3
#define PIN_DISP_HR_BIT_0 0
#define PIN_DISP_HR_BIT_1 1
#define PIN_DISP_HR_BIT_2 2
#define PIN_DISP_HR_BIT_3 3
#define PIN_DISP_MIN_FIVE 4
#define PIN_DISP_MIN_TEN 5
#define PIN_DISP_MIN_FIFTEEN 6
#define PIN_DISP_MIN_TWENTY 7
#define PIN_DISP_MIN_HALF 8
#define PIN_DISP_WORD_OCLOCK 9
#define PIN_DISP_WORD_PAST 10
#define PIN_DISP_WORD_TO 11

#define FLAGS_FIVE 0b11110
#define FLAGS_TEN 0b11101
#define FLAGS_FIFTEEN 0b11011
#define FLAGS_TWENTY 0b10111
#define FLAGS_TWENTYFIVE 0b10110
#define FLAGS_HALF 0b01111
#define FLAGS_NONE 0b11111
#define FLAGS_OCLOCK 0b110
#define FLAGS_PAST 0b101
#define FLAGS_TO 0b011

RTC_DS1307 rtc;
DateTime now;
int cycles = 0;
const TimeSpan TIME_HR_UP = TimeSpan(0, 1, 0, 0);
const TimeSpan TIME_HR_DOWN = TimeSpan(0, -1, 0, 0);
const TimeSpan TIME_MIN_UP = TimeSpan(0, 0, 1, 0);
const TimeSpan TIME_MIN_DOWN = TimeSpan(0, 0, -1, 0);

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");

  pinMode(PIN_HR_UP, INPUT);
  pinMode(PIN_HR_DOWN, INPUT);
  pinMode(PIN_MIN_UP, INPUT);
  pinMode(PIN_MIN_DOWN, INPUT);

  pinMode(PIN_DISP_HR_BIT_0, OUTPUT);
  pinMode(PIN_DISP_HR_BIT_1, OUTPUT);
  pinMode(PIN_DISP_HR_BIT_2, OUTPUT);
  pinMode(PIN_DISP_HR_BIT_3, OUTPUT);
  pinMode(PIN_DISP_MIN_FIVE, OUTPUT);
  pinMode(PIN_DISP_MIN_TEN, OUTPUT);
  pinMode(PIN_DISP_MIN_FIFTEEN, OUTPUT);
  pinMode(PIN_DISP_MIN_TWENTY, OUTPUT);
  pinMode(PIN_DISP_MIN_HALF, OUTPUT);
  pinMode(PIN_DISP_WORD_OCLOCK, OUTPUT);
  pinMode(PIN_DISP_WORD_PAST, OUTPUT);
  pinMode(PIN_DISP_WORD_TO, OUTPUT);

  // Since the transistor logic turns on the LEDs
  // when the signal is LOW, turn off the LEDs here
  // by sending HIGH signals
  digitalWrite(PIN_DISP_MIN_FIVE, HIGH);
  digitalWrite(PIN_DISP_MIN_TEN, HIGH);
  digitalWrite(PIN_DISP_MIN_FIFTEEN, HIGH);
  digitalWrite(PIN_DISP_MIN_TWENTY, HIGH);
  digitalWrite(PIN_DISP_MIN_HALF, HIGH);
  digitalWrite(PIN_DISP_WORD_OCLOCK, HIGH);
  digitalWrite(PIN_DISP_WORD_PAST, HIGH);
  digitalWrite(PIN_DISP_WORD_TO, HIGH);

  if (!rtc.begin()){
    Serial.println("Couldn't find RTC");
    while(1);
  }
  
  rtc.adjust(DateTime(2023, 1, 1, 0, 0, 0));
/*  
  Wire.begin(); // Join I2C bus as master
  
  Wire.beginTransmission(ADDRESS_7SEG);
  Wire.write(BYTE_RESET_DISPLAY); // Reset display 
  Wire.endTransmission();

  setColon(true);
*/
}

void loop() {
  /*
    cycles = (cycles + 1) % CYCLE_SPEED;
    if (cycles == 0) {
      writeTimeToDisplay();
    }
    checkButtons();
  */

  cycles = (cycles + 1) % CYCLE_SPEED;
  if (cycles == 0) {
    checkTime();
    rtc.adjust(now + TIME_MIN_UP);
  }
  // checkButtons();
}

void checkTime() {
  now = rtc.now();
  int minute = now.minute();
  int hour = (setMinute(minute) + now.hour()) % 12;
  setHour(hour);
}

void checkButtons() {
  static int buttonCycles = 0;
  bool hrUp = digitalRead(PIN_HR_UP);
  bool hrDown = digitalRead(PIN_HR_DOWN);
  bool minUp = digitalRead(PIN_MIN_UP);
  bool minDown = digitalRead(PIN_MIN_DOWN);
  if (hrUp || hrDown || minUp || minDown) {
    buttonCycles = (buttonCycles + 1) % BUTTON_CYCLE_SPEED;
    if (buttonCycles == 1) {
      now = rtc.now();
      if (hrUp) {
        rtc.adjust(now + TIME_HR_UP);
      }
      if (hrDown) {
        rtc.adjust(now + TIME_HR_DOWN);
      }
      if (minUp) {
        rtc.adjust(now + TIME_MIN_UP);
      }
      if (minDown) {
        rtc.adjust(now + TIME_MIN_DOWN);
      }
      writeTimeToDisplay();
    }
  }
  else {
    buttonCycles = 0;
  }
}

// Returns 1 if minutes are 35 or higher, otherwise returns 0
// Ex. 4:45 would be displayed as QUARTER TO FIVE, so we add
// 1 to make it FIVE instead of FOUR.
int setMinute(int minute) {
  if (minute >= 55) {
    lightMinutes(FLAGS_FIVE, FLAGS_TO);
    Serial.print("FIVE ");
    Serial.print("TO ");
  }
  else if (minute >= 50) {
    lightMinutes(FLAGS_TEN, FLAGS_TO);
    Serial.print("TEN ");
    Serial.print("TO ");
  }
  else if (minute >= 45) {
    lightMinutes(FLAGS_FIFTEEN, FLAGS_TO);
    Serial.print("QUARTER ");
    Serial.print("TO ");
  }
  else if (minute >= 40) {
    lightMinutes(FLAGS_TWENTY, FLAGS_TO);
    Serial.print("TWENTY ");
    Serial.print("TO ");
  }
  else if (minute >= 35) {
    lightMinutes(FLAGS_TWENTYFIVE, FLAGS_TO);
    Serial.print("TWENTY FIVE ");
    Serial.print("TO ");
  }
  else if (minute >= 30) {
    lightMinutes(FLAGS_HALF, FLAGS_PAST);
    Serial.print("HALF ");
    Serial.print("PAST ");
  }
  else if (minute >= 25) {
    lightMinutes(FLAGS_TWENTYFIVE, FLAGS_PAST);
    Serial.print("TWENTY FIVE ");
    Serial.print("PAST ");
  }
  else if (minute >= 20) {
    lightMinutes(FLAGS_TWENTY, FLAGS_PAST);
    Serial.print("TWENTY ");
    Serial.print("PAST ");
  }
  else if (minute >= 15) {
    lightMinutes(FLAGS_FIFTEEN, FLAGS_PAST);
    Serial.print("FIFTEEN ");
    Serial.print("PAST ");
  }
  else if (minute >= 10) {
    lightMinutes(FLAGS_TEN, FLAGS_PAST);
    Serial.print("TEN ");
    Serial.print("PAST ");
  }
  else if (minute >= 5) {
    lightMinutes(FLAGS_FIVE, FLAGS_PAST);
    Serial.print("FIVE ");
    Serial.print("PAST ");
  }
  else {
    lightMinutes(FLAGS_NONE, FLAGS_OCLOCK);
    Serial.print("O'CLOCK ");
  }
  return (minute >= 35) ? 1 : 0;
}

// Hours are set using a 4 to 16 binary decoder
int setHour(uint8_t hour) {

  digitalWrite(PIN_DISP_HR_BIT_0, (hour & 0b0001));
  digitalWrite(PIN_DISP_HR_BIT_1, (hour & 0b0010) >> 1);
  digitalWrite(PIN_DISP_HR_BIT_2, (hour & 0b0100) >> 2);
  digitalWrite(PIN_DISP_HR_BIT_3, (hour & 0b1000) >> 3);
  Serial.println(hour);
}

void setColon(bool on) {
  Wire.beginTransmission(ADDRESS_7SEG);
  Wire.write(BYTE_DECIMAL_CONTROL);
  if (on) {
    Wire.write(BYTE_COLON);
  }
  else {
    Wire.write(0);
  }
  Wire.endTransmission();
}

void lightMinutes(uint8_t minute_flags, uint8_t word_flags) {
  
  digitalWrite(PIN_DISP_MIN_FIVE, (minute_flags & 0b00001));
  digitalWrite(PIN_DISP_MIN_TEN, (minute_flags & 0b00010) >> 1);
  digitalWrite(PIN_DISP_MIN_FIFTEEN, (minute_flags & 0b00100) >> 2);
  digitalWrite(PIN_DISP_MIN_TWENTY, (minute_flags & 0b01000) >> 3);
  digitalWrite(PIN_DISP_MIN_HALF, (minute_flags & 0b10000) >> 4);

  digitalWrite(PIN_DISP_WORD_OCLOCK, (word_flags & 0b001));
  digitalWrite(PIN_DISP_WORD_PAST, (word_flags & 0b010) >> 1);
  digitalWrite(PIN_DISP_WORD_TO, (word_flags & 0b100) >> 2);
}

void writeTimeToDisplay() {
  now = rtc.now();
  writeNumberToDisplay(now.hour() * 100 + now.minute());
}

void writeNumberToDisplay(int number) {
  Wire.beginTransmission(ADDRESS_7SEG);
  Wire.write(number / 1000);
  number %= 1000;
  Wire.write(number / 100);
  number %= 100;
  Wire.write(number / 10);
  number %= 10;
  Wire.write(number);
  Wire.endTransmission();
}
