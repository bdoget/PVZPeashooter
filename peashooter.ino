#include <WiFi.h>
#include <ESP32Servo.h>

// Wi-Fi credentials
const char* ssid = "ESP32-Network";
const char* password = "Esp32-Password";

WiFiServer server(80);
String header;

// Onboard LED pin for ESP32-S3 DevKit
const int ledPin = 48;

// H-Bridge motor control pins
const int motorFL_1 = 16;
const int motorFL_2 = 17;
const int enableFL = 18;

const int motorFR_1 = 37;
const int motorFR_2 = 36;
const int enableFR = 35;

const int motorBL_1 = 19;
const int motorBL_2 = 20;
const int enableBL = 21;

const int motorBR_1 = 10;
const int motorBR_2 = 11;
const int enableBR = 12;

// Single Servo
Servo waveServo;
const int waveServoPin = 2;
bool shouldWave = false;

// PWM
const int speedPWM = 200;
const int freq = 1000;
const int resolution = 8;

// State
String driveCommand = "stop";

void setup() {
  Serial.begin(115200);

  int pins[] = {
    motorFL_1, motorFL_2, motorFR_1, motorFR_2,
    motorBL_1, motorBL_2, motorBR_1, motorBR_2
  };
  for (int i = 0; i < 8; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Servo setup
  waveServo.setPeriodHertz(50);
  waveServo.attach(waveServoPin, 500, 2400);
  waveServo.write(0);

  // Wi-Fi
  WiFi.softAP(ssid, password);
  Serial.println("WiFi started");
  Serial.print("IP: "); Serial.println(WiFi.softAPIP());
  server.begin();
}

void stopMotors() {
  digitalWrite(motorFL_1, LOW); digitalWrite(motorFL_2, LOW);
  digitalWrite(motorFR_1, LOW); digitalWrite(motorFR_2, LOW);
  digitalWrite(motorBL_1, LOW); digitalWrite(motorBL_2, LOW);
  digitalWrite(motorBR_1, LOW); digitalWrite(motorBR_2, LOW);
  ledcWrite(0, 0); ledcWrite(1, 0); ledcWrite(2, 0); ledcWrite(3, 0);
}

void moveForward() {
  digitalWrite(motorFL_1, HIGH); digitalWrite(motorFL_2, LOW);
  digitalWrite(motorFR_1, HIGH); digitalWrite(motorFR_2, LOW);
  digitalWrite(motorBL_1, HIGH); digitalWrite(motorBL_2, LOW);
  digitalWrite(motorBR_1, HIGH); digitalWrite(motorBR_2, LOW);
  ledcWrite(0, speedPWM); ledcWrite(1, speedPWM); ledcWrite(2, speedPWM); ledcWrite(3, speedPWM);
}

void moveBackward() {
  digitalWrite(motorFL_1, LOW); digitalWrite(motorFL_2, HIGH);
  digitalWrite(motorFR_1, LOW); digitalWrite(motorFR_2, HIGH);
  digitalWrite(motorBL_1, LOW); digitalWrite(motorBL_2, HIGH);
  digitalWrite(motorBR_1, LOW); digitalWrite(motorBR_2, HIGH);
  ledcWrite(0, speedPWM); ledcWrite(1, speedPWM); ledcWrite(2, speedPWM); ledcWrite(3, speedPWM);
}

void turnLeft() {
  digitalWrite(motorFL_1, LOW); digitalWrite(motorFL_2, HIGH);
  digitalWrite(motorFR_1, HIGH); digitalWrite(motorFR_2, LOW);
  digitalWrite(motorBL_1, LOW); digitalWrite(motorBL_2, HIGH);
  digitalWrite(motorBR_1, HIGH); digitalWrite(motorBR_2, LOW);
  ledcWrite(0, speedPWM); ledcWrite(1, speedPWM); ledcWrite(2, speedPWM); ledcWrite(3, speedPWM);
}

void turnRight() {
  digitalWrite(motorFL_1, HIGH); digitalWrite(motorFL_2, LOW);
  digitalWrite(motorFR_1, LOW); digitalWrite(motorFR_2, HIGH);
  digitalWrite(motorBL_1, HIGH); digitalWrite(motorBL_2, LOW);
  digitalWrite(motorBR_1, LOW); digitalWrite(motorBR_2, HIGH);
  ledcWrite(0, speedPWM); ledcWrite(1, speedPWM); ledcWrite(2, speedPWM); ledcWrite(3, speedPWM);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String currentLine = "";
    unsigned long timeout = millis();

    while (client.connected() && millis() - timeout < 2000) {
      if (client.available()) {
        char c = client.read();
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Handle commands
            if (header.indexOf("GET /forward") >= 0) driveCommand = "forward";
            else if (header.indexOf("GET /backward") >= 0) driveCommand = "backward";
            else if (header.indexOf("GET /left") >= 0) driveCommand = "right";
            else if (header.indexOf("GET /right") >= 0) driveCommand = "left";
            else if (header.indexOf("GET /stop") >= 0) driveCommand = "stop";
            else if (header.indexOf("GET /wave") >= 0) shouldWave = true;

            // Send HTML UI
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>");
            client.println("<style>body { background-color: green; font-family: sans-serif; text-align: center; }");
            client.println(".button { background-color: teal; border: none; color: white; padding: 20px; font-size: 20px; margin: 10px; cursor: pointer; }</style></head><body>");
            client.println("<h1>PeaShooter</h1>");
            client.println("<p><a href='/forward'><button class='button'>FORWARD</button></a></p>");
            client.println("<p><a href='/left'><button class='button'>LEFT</button></a>");
            client.println("<a href='/right'><button class='button'>RIGHT</button></a></p>");
            client.println("<p><a href='/backward'><button class='button'>BACKWARD</button></a></p>");
            client.println("<p><a href='/stop'><button class='button'>STOP</button></a></p>");
            client.println("<p><a href='/wave'><button class='button'>WAVE</button></a></p>");
            client.println("</body></html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
  }

  // Drive logic + onboard LED
  if (driveCommand == "forward") {
    moveForward();
    digitalWrite(ledPin, HIGH);
  } else if (driveCommand == "backward") {
    moveBackward();
    digitalWrite(ledPin, LOW);
  } else if (driveCommand == "left") {
    turnLeft();
    digitalWrite(ledPin, LOW);
  } else if (driveCommand == "right") {
    turnRight();
    digitalWrite(ledPin, LOW);
  } else {
    stopMotors();
    digitalWrite(ledPin, LOW);
  }

  // WAVE servo sweep with LED blinking
  if (shouldWave) {
    for (int angle = 0; angle <= 180; angle++) {
      waveServo.write(angle);
      digitalWrite(ledPin, (angle % 20 < 10) ? HIGH : LOW);
      delay(5);
    }
    for (int angle = 180; angle >= 0; angle--) {
      waveServo.write(angle);
      digitalWrite(ledPin, (angle % 20 < 10) ? HIGH : LOW);
      delay(5);
    }
    digitalWrite(ledPin, LOW);
    shouldWave = false;
  }

  delay(10);
}