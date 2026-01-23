#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <ESP32Servo.h>

// WiFi
const char* ssid = "RobotWlan";
const char* pass = "RobotsDreamAllDayLongUnderPressure";

// UDP / OSC
WiFiUDP Udp;
const uint16_t localPort = 8888;

// Servo
Servo myservo;
const int servoPin = 15;

static int clamp180(int v) {
  if (v < 0) return 0;
  if (v > 180) return 180;
  return v;
}

void setup() {
  Serial.begin(115200);

  // Servo init
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 500, 2500);
  myservo.write(90);

  // WiFi init
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, pass);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // UDP listen
  Udp.begin(localPort);
  Serial.print("Listening OSC UDP port ");
  Serial.println(localPort);
  Serial.println("Expect: /servo/1 <0..180>");
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize <= 0) return;

  OSCMessage msg;
  while (packetSize--) msg.fill(Udp.read());

  if (msg.hasError()) {
    Serial.print("OSC error: ");
    Serial.println(msg.getError());
    return;
  }

  if (!msg.fullMatch("/servo/1")) return;

  int angle;
  if (msg.isInt(0))       angle = msg.getInt(0);
  else if (msg.isFloat(0)) angle = (int)(msg.getFloat(0) + 0.5f);
  else return;

  angle = clamp180(angle);
  myservo.write(angle);

  Serial.print("Servo angle: ");
  Serial.println(angle);
}
