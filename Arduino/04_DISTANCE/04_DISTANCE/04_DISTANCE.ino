/**********************************************************************
  Filename    : Ultrasonic OSC Broadcast
  Description : Measure distance with HC-SR04 and broadcast via OSC
**********************************************************************/

#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

// ---------- WiFi ----------
const char* ssid     = "RobotWlan";
const char* password = "RobotsDreamAllDayLongUnderPressure";

// ---------- OSC ----------
WiFiUDP Udp;
const uint16_t OSC_PORT = 6666;
IPAddress broadcastIP(255, 255, 255, 255);

// ---------- Ultrasonic ----------
#define trigPin 13
#define echoPin 14
#define MAX_DISTANCE 700

float timeOut = MAX_DISTANCE * 60;
int soundVelocity = 340; // m/s

// ---------- Timing ----------
unsigned long lastSend = 0;
const unsigned long sendInterval = 100; // ms

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("Broadcasting OSC /distance on UDP port 6666");
}

// ---------- Loop ----------
void loop() {
  if (millis() - lastSend >= sendInterval) {
    lastSend = millis();

    float distance = getSonar();

    // Serial output
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // OSC message
    OSCMessage msg("/distance");
    msg.add(distance);

    Udp.beginPacket(broadcastIP, OSC_PORT);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
  }
}

// ---------- Ultrasonic function ----------
float getSonar() {
  unsigned long pingTime;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  pingTime = pulseIn(echoPin, HIGH, timeOut);

  if (pingTime == 0) return -1.0; // no echo

  return (float)pingTime * soundVelocity / 2 / 10000;
}
