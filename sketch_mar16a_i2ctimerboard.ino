#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// PIN DEFINITIONS
#define PIN_POT        1    // RV1 - Minutes potentiometer
#define PIN_BTN_SEC    9    // SW1 - Seconds button
#define PIN_BTN_START  10   // SW2 - Start button
#define PIN_BTN_STOP   17   // SW3 - Stop/End button
#define PIN_LED_GREEN  5    // D1  - Green LED (via Q1)
#define PIN_LED_RED    6    // D2  - Red LED (via Q2)
#define PIN_LED_WHITE1 7    // D3  - White LED left (via Q3)
#define PIN_LED_WHITE2 8    // D4  - White LED right (via Q4)
#define PIN_BUZZER     18   // BZ1 - Buzzer (via Q5)
#define PIN_SDA        11   // J1  - OLED SDA
#define PIN_SCL        12   // J1  - OLED SCL

// DEBOUNCE
#define DEBOUNCE_DELAY 200
unsigned long lastDebounceStart = 0;
unsigned long lastDebounceStop  = 0;
unsigned long lastDebounceSec   = 0;

// TIMER STATE
enum State { IDLE, RUNNING };
State currentState = IDLE;

int setMinutes = 0;
int setSeconds = 0;
unsigned long timerDuration  = 0;
unsigned long timerStartMs   = 0;
unsigned long remainingTime  = 0;

// SOFTWARE CLOCK
unsigned long clockBase = 0;
const int START_HOUR = 12;
const int START_MIN  = 0;
const int START_SEC  = 0;

//  HELPERS

void getClockTime(int &h, int &m, int &s, bool &isPM) {
  unsigned long elapsed = (millis() - clockBase) / 1000
                        + START_HOUR * 3600
                        + START_MIN  * 60
                        + START_SEC;
  int totalHours = (elapsed / 3600) % 24;
  s    = elapsed % 60;
  m    = (elapsed / 60) % 60;
  isPM = totalHours >= 12;
  h    = totalHours % 12;
  if (h == 0) h = 12;
}

void flashLED(int pin, int durationMs) {
  digitalWrite(pin, HIGH);
  delay(durationMs);
  digitalWrite(pin, LOW);
}

void soundBuzzer() {
  for (int i = 0; i < 3; i++) {
    tone(PIN_BUZZER, 1000, 300);
    delay(400);
  }
  noTone(PIN_BUZZER);
}

//  DISPLAY FUNCTIONS

void printTwoDigit(int val) {
  if (val < 10) display.print("0");
  display.print(val);
}

void displayClock() {
  int h, m, s;
  bool isPM;
  getClockTime(h, m, s, isPM);

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, 8);
  printTwoDigit(h);
  display.print(":");
  printTwoDigit(m);
  display.print(":");
  printTwoDigit(s);
  display.setTextSize(1);
  display.print(isPM ? " PM" : " AM");

  display.drawLine(0, 34, 128, 34, SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(10, 42);
  display.print("Set: ");
  printTwoDigit(setMinutes);
  display.print("m  ");
  printTwoDigit(setSeconds);
  display.print("s");

  display.setCursor(10, 54);
  display.print("START to begin");

  display.display();
}

void displayTimer() {
  int mins = (remainingTime / 1000) / 60;
  int secs = (remainingTime / 1000) % 60;

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(40, 4);
  display.print("TIMER");

  display.setTextSize(3);
  display.setCursor(14, 20);
  printTwoDigit(mins);
  display.print(":");
  printTwoDigit(secs);

  display.setTextSize(1);
  display.setCursor(28, 56);
  display.print("STOP to cancel");

  display.display();
}

void displayDone() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(24, 24);
  display.print("DONE!");
  display.display();
}

//  SETUP

void setup() {
  pinMode(PIN_BTN_SEC,    INPUT_PULLUP);
  pinMode(PIN_BTN_START,  INPUT_PULLUP);
  pinMode(PIN_BTN_STOP,   INPUT_PULLUP);
  pinMode(PIN_LED_GREEN,  OUTPUT);
  pinMode(PIN_LED_RED,    OUTPUT);
  pinMode(PIN_LED_WHITE1, OUTPUT);
  pinMode(PIN_LED_WHITE2, OUTPUT);
  pinMode(PIN_BUZZER,     OUTPUT);

  // White LEDs on immediately as power indicators
  digitalWrite(PIN_LED_WHITE1, HIGH);
  digitalWrite(PIN_LED_WHITE2, HIGH);

  // Initialize I2C with correct SDA and SCL pins
  Wire.begin(PIN_SDA, PIN_SCL);

  // Try both I2C addresses
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      // OLED not found — blink white LEDs as error signal
      while (true) {
        digitalWrite(PIN_LED_WHITE1, HIGH);
        digitalWrite(PIN_LED_WHITE2, HIGH);
        delay(300);
        digitalWrite(PIN_LED_WHITE1, LOW);
        digitalWrite(PIN_LED_WHITE2, LOW);
        delay(300);
      }
    }
  }

  display.clearDisplay();
  display.display();

  clockBase = millis();
}

//  LOOP

void loop() {
  unsigned long now = millis();

  int potValue = analogRead(PIN_POT);
  setMinutes   = map(potValue, 0, 4095, 0, 99);

  if (digitalRead(PIN_BTN_SEC) == LOW && (now - lastDebounceSec > DEBOUNCE_DELAY)) {
    lastDebounceSec = now;
    if (currentState == IDLE) {
      setSeconds = (setSeconds + 1) % 60;
    }
  }

  if (digitalRead(PIN_BTN_START) == LOW && (now - lastDebounceStart > DEBOUNCE_DELAY)) {
    lastDebounceStart = now;
    if (currentState == IDLE && (setMinutes > 0 || setSeconds > 0)) {
      timerDuration = ((unsigned long)setMinutes * 60 + setSeconds) * 1000;
      timerStartMs  = millis();
      currentState  = RUNNING;
      flashLED(PIN_LED_GREEN, 200);
    }
  }

  if (digitalRead(PIN_BTN_STOP) == LOW && (now - lastDebounceStop > DEBOUNCE_DELAY)) {
    lastDebounceStop = now;
    if (currentState == RUNNING) {
      currentState = IDLE;
      flashLED(PIN_LED_RED, 200);
    }
  }

  if (currentState == RUNNING) {
    unsigned long elapsed = millis() - timerStartMs;
    if (elapsed >= timerDuration) {
      currentState = IDLE;
      displayDone();
      soundBuzzer();
      delay(2000);
    } else {
      remainingTime = timerDuration - elapsed;
      displayTimer();
    }
  } else {
    displayClock();
  }
}