// XIAO ESP32-S3 potentiometer reader for Arduino IDE.
//
// Wiring:
//   GND   -> potentiometer GND
//   3V3   -> potentiometer VCC
//   A10   -> potentiometer VALUE (wiper)
//
// Notes for Arduino IDE:
//   - Board: "XIAO_ESP32S3" (or equivalent Seeed XIAO ESP32-S3 board entry)
//   - USB CDC On Boot: Enabled

constexpr uint8_t kPotPin = A10;
constexpr uint8_t kLedPin = 21;  // XIAO ESP32-S3 built-in LED.
constexpr uint32_t kBaudRate = 115200;
constexpr uint32_t kSamplesPerSecond = 10;  // Default: 2 readings per second.
constexpr uint32_t kLedPulseWidthMs = 50;

uint32_t nextSampleAtMs = 0;
uint32_t ledPulseEndsAtMs = 0;
uint16_t lastReading = 0;

void waitForSerialMonitor(uint32_t timeoutMs) {
  const uint32_t startMs = millis();

  while (!Serial && (millis() - startMs < timeoutMs)) {
    delay(10);
  }
}

void printReading() {
  const uint32_t percent = (static_cast<uint32_t>(lastReading) * 100UL + 2047UL) / 4095UL;
  Serial.println(percent);
}

void setup() {
  Serial.begin(kBaudRate);
  waitForSerialMonitor(5000);

  analogReadResolution(12);
#if defined(ESP32)
  analogSetAttenuation(ADC_11db);
#endif

  pinMode(kPotPin, INPUT);
  pinMode(kLedPin, OUTPUT);
  digitalWrite(kLedPin, LOW);

  delay(500);
  nextSampleAtMs = millis();
}

void loop() {
  const uint32_t now = millis();
  const uint32_t intervalMs = 1000UL / (kSamplesPerSecond == 0 ? 1 : kSamplesPerSecond);

  if (now >= nextSampleAtMs) {
    lastReading = analogRead(kPotPin);
    printReading();
    digitalWrite(kLedPin, HIGH);
    ledPulseEndsAtMs = now + kLedPulseWidthMs;
    nextSampleAtMs = now + intervalMs;
  }

  if (ledPulseEndsAtMs != 0 && now >= ledPulseEndsAtMs) {
    digitalWrite(kLedPin, LOW);
    ledPulseEndsAtMs = 0;
  }
}
