#define LED_BUILTIN 2   // common for ESP32 boards

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  delay(500); // give serial time to start

  Serial.println("ESP32 Blink test started");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("1: LED is HIGH (ON)");
  delay(1000);

  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("2: LED is LOW (OFF)");
  delay(1000);
}
