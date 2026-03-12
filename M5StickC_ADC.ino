#include <M5StickC.h>

// M5StickC potentiometer reader for Arduino IDE.
//
// Wiring:
//   GND   -> potentiometer GND
//   3V3   -> potentiometer VCC
//   G36   -> potentiometer VALUE (wiper)
//
// Notes for Arduino IDE:
//   - Board: "M5Stick-C"
//   - Upload speed / serial speed: 115200
//   - Library Manager: install "M5StickC"
//
// Warning:
//   - Do not power the potentiometer from the Grove connector's 5V pin.
//     Use a 3.3V pin from the expansion header so the ADC input stays in range.

constexpr uint8_t kPotPin = 36;   // GPIO36 / ADC1_CH0 on M5StickC.
constexpr uint8_t kLedPin = 10;   // M5StickC built-in red LED.
constexpr uint8_t kLedOn = LOW;   // The built-in LED is active-low.
constexpr uint8_t kLedOff = HIGH;
constexpr uint32_t kBaudRate = 115200;
constexpr uint32_t kSamplesPerSecond = 10;
constexpr int32_t kOutputMin = -10;
constexpr int32_t kOutputMax = 10;
constexpr uint16_t kBackgroundColor = TFT_BLACK;
constexpr uint16_t kValueColor = TFT_YELLOW;

uint32_t nextSampleAtMs = 0;
uint16_t lastReading = 0;
int32_t lastValue = 999;

void waitForSerialMonitor(uint32_t timeoutMs) {
  const uint32_t startMs = millis();

  while (!Serial && (millis() - startMs < timeoutMs)) {
    delay(10);
  }
}

void printReading() {
  Serial.println(readingToValue(lastReading));
}

int32_t readingToValue(uint16_t reading) {
  const int32_t span = kOutputMax - kOutputMin;
  return (static_cast<int32_t>(reading) * span + 2047L) / 4095L + kOutputMin;
}

void drawReading(int32_t value) {
  if (value == lastValue) {
    return;
  }

  char text[5];
  snprintf(text, sizeof(text), "%ld", static_cast<long>(value));

  M5.Lcd.fillScreen(kBackgroundColor);
  M5.Lcd.setTextColor(kValueColor, kBackgroundColor);
  M5.Lcd.drawCentreString(text, M5.Lcd.width() / 2, 8, 7);

  lastValue = value;
}

void setup() {
  M5.begin(true, true, false);
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(kBackgroundColor);
  M5.Lcd.setTextDatum(TC_DATUM);

  Serial.begin(kBaudRate);
  waitForSerialMonitor(5000);

  analogReadResolution(12);
#if defined(ESP32)
  analogSetAttenuation(ADC_11db);
#endif

  pinMode(kPotPin, INPUT);
  pinMode(kLedPin, OUTPUT);
  digitalWrite(kLedPin, kLedOff);

  delay(500);
  drawReading(0);
  nextSampleAtMs = millis();
}

void loop() {
  const uint32_t now = millis();
  const uint32_t intervalMs =
      1000UL / (kSamplesPerSecond == 0 ? 1 : kSamplesPerSecond);

  if (now >= nextSampleAtMs) {
    lastReading = analogRead(kPotPin);
    drawReading(readingToValue(lastReading));
    printReading();
    nextSampleAtMs = now + intervalMs;
  }
}
