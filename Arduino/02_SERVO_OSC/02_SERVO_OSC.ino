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

// WiFi
char ssid[] = "RobotWlan";
char pass[] = "RobotsDreamAllDayLongUnderPressure";

// UDP/OSC
WiFiUDP Udp;
const unsigned int localPort = 8888;

// Servo
Servo myservo;
const int servoPin = 15;
int currentAngle = 90;

int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// Expect: /servo/1 <0..180> (int or float)
void servo1(OSCMessage &msg) {
  int angle;

  if (msg.isInt(0)) {
    angle = msg.getInt(0);
  }
  else if (msg.isFloat(0)) {
    angle = (int)(msg.getFloat(0) + 0.5f);  // round float to int
  }
  else {
    Serial.println("servo1: expected int or float at arg0");
    return;
  }

  // Only accept 0..180 (either clamp or reject — choose one)
  // Option A (recommended): clamp to safe range
  angle = clampInt(angle, 0, 180);

  // Option B: reject out of range (uncomment to use)
  /*
  if (angle < 0 || angle > 180) {
    Serial.print("servo1: out of range: ");
    Serial.println(angle);
    return;
  }
  */

  if (angle != currentAngle) {
    currentAngle = angle;
    myservo.write(currentAngle);
    Serial.print("Servo angle: ");
    Serial.println(currentAngle);
  }
}

void setup() {
  Serial.begin(115200);

  // Servo init
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 500, 2500);
  myservo.write(currentAngle);

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

  Serial.println("Send OSC: /servo/1 <0..180>");
}

void loop() {
  OSCBundle bundle;
  int size = Udp.parsePacket();

  if (size <= 0) return;

  while (size--) {
    bundle.fill(Udp.read());
  }

  if (bundle.hasError()) {
    Serial.print("OSC error: ");
    Serial.println(bundle.getError());
    return;
  }

  bundle.dispatch("/servo/1", servo1);
}
