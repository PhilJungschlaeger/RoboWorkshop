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

// -------- Servo --------
Servo myservo;
const int servoPin = 15;     // your servo pin
int currentAngle = 90;

// Helper: clamp int to [min,max]
int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// OSC handler: expects /servo/1 <int|float>
// - If float is 0..1  -> mapped to 0..180
// - If float/int is 0..180 -> used directly
void servo1(OSCMessage &msg) {
  int angle = currentAngle;

  if (msg.isFloat(0)) {
    float v = msg.getFloat(0);

    // If sender uses normalized 0..1, map it
    if (v >= 0.0f && v <= 1.0f) {
      angle = (int)(v * 180.0f + 0.5f);
    } else {
      angle = (int)(v + 0.5f);
    }
  } 
  else if (msg.isInt(0)) {
    angle = msg.getInt(0);
  } 
  else {
    Serial.println("servo1: unsupported OSC type");
    return;
  }

  angle = clampInt(angle, 0, 180);
  currentAngle = angle;
  myservo.write(currentAngle);

  Serial.print("Servo angle set to: ");
  Serial.println(currentAngle);
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

  Serial.println("Send OSC: /servo/1 <0..180>  (or normalized 0..1)");
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
    } else {
      Serial.print("OSC error: ");
      Serial.println(bundle.getError());
    }
  }
}
