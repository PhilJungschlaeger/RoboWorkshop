// Install Library: OSC by Adrian Freed

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#include <ESP32Servo.h>

// -------- WiFi credentials --------
char ssid[] = "RobotWlan";
char pass[] = "RobotsDreamAllDayLongUnderPressure";

// -------- UDP/OSC --------
WiFiUDP Udp;
const unsigned int localPort = 8888;

// -------- Servo pins (your definitions) --------
const int servoPin_1 = 13;
const int servoPin_2 = 14;
const int servoPin_3 = 33;
const int servoPin_4 = 32;

// How many servos you actually use (1..4)
const int NUM_SERVOS = 4;

// Servo objects + state
Servo servos[NUM_SERVOS];
int currentAngle[NUM_SERVOS] = {90, 90, 90, 90};

// Helper: clamp int to [min,max]
int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// Convert OSC arg -> angle 0..180
int oscArgToAngle(OSCMessage &msg, int idx, int fallback) {
  int angle = fallback;

  if (msg.isFloat(idx)) {
    float v = msg.getFloat(idx);

    // If sender uses normalized 0..1, map it
    if (v >= 0.0f && v <= 1.0f) {
      angle = (int)(v * 180.0f + 0.5f);
    } else {
      angle = (int)(v + 0.5f);
    }
  }
  else if (msg.isInt(idx)) {
    angle = msg.getInt(idx);
  }
  else {
    // unsupported type, keep fallback
    return fallback;
  }

  return clampInt(angle, 0, 180);
}

void setServo(int i, int angle) {
  if (i < 0 || i >= NUM_SERVOS) return;

  angle = clampInt(angle, 0, 180);
  currentAngle[i] = angle;
  servos[i].write(angle);

  Serial.print("Servo ");
  Serial.print(i + 1);
  Serial.print(" angle set to: ");
  Serial.println(angle);
}

// --- OSC handlers (library dispatch wants a function pointer, easiest is 4 wrappers) ---
void servo1(OSCMessage &msg) { setServo(0, oscArgToAngle(msg, 0, currentAngle[0])); }
void servo2(OSCMessage &msg) { if (NUM_SERVOS >= 2) setServo(1, oscArgToAngle(msg, 0, currentAngle[1])); }
void servo3(OSCMessage &msg) { if (NUM_SERVOS >= 3) setServo(2, oscArgToAngle(msg, 0, currentAngle[2])); }
void servo4(OSCMessage &msg) { if (NUM_SERVOS >= 4) setServo(3, oscArgToAngle(msg, 0, currentAngle[3])); }

void setup() {
  Serial.begin(115200);

  // Servo init
  // Typical servo: 50 Hz, pulse 500-2500 us
  const int pins[4] = { servoPin_1, servoPin_2, servoPin_3, servoPin_4 };

  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].setPeriodHertz(50);
    servos[i].attach(pins[i], 500, 2500);
    servos[i].write(currentAngle[i]);
  }

  // WiFi init
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
#if defined(ESP32)
  WiFi.setSleep(false);
#endif
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // UDP listen
  Udp.begin(localPort);
  Serial.print("Listening for OSC on UDP port ");
  Serial.println(localPort);

  Serial.println("Send OSC:");
  Serial.println("  /servo/1 <0..180> (or normalized 0..1)");
  Serial.println("  /servo/2 <0..180> (or normalized 0..1)");
  Serial.println("  /servo/3 <0..180> (or normalized 0..1)");
  Serial.println("  /servo/4 <0..180> (or normalized 0..1)");
}

void loop() {
  OSCBundle bundle;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }

    if (!bundle.hasError()) {
      bundle.dispatch("/servo/1", servo1);
      bundle.dispatch("/servo/2", servo2);
      bundle.dispatch("/servo/3", servo3);
      bundle.dispatch("/servo/4", servo4);
    } else {
      Serial.print("OSC error: ");
      Serial.println(bundle.getError());
    }
  }
}
