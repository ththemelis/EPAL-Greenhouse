//  https://randomnerdtutorials.com/esp32-web-server-websocket-sliders/
//  https://randomnerdtutorials.com/esp32-websocket-server-arduino/
//  https://randomnerdtutorials.com/esp32-websocket-server-sensor/

// Αλλαγή των κουμπιών (από τις βάνες) για να λειτουργούν όπως οι sliders
// Προσθήκη φίλτρου για αφαίρεση μη αποδεκτών τιμών από τους αισθητήρες (moving average?)

#include <WiFi.h>
#include <WiFiMulti.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include "SparkFunBME280.h"
#include "TickTwo.h"

String getSensorReadings();

TickTwo readingsTimer(getSensorReadings, 30000);

bool ledState = 0;
#define ledPin 2

WiFiMulti wifiMulti;    //
AsyncWebServer server(80);  // Δημιουργία αντικειμένου για τον Web Server (πόρτα 80)
AsyncWebSocket ws("/ws");   // Δημιουργία αντικειμένου WebSocket
void initWiFi() {   // Συνάρτηση ενεργοποίησης και σύνδεσης του WiFi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("argos", "21040672bill");
  wifiMulti.addAP("theo", "21040672bill");
  wifiMulti.addAP(" Galaxy S23", "1234512345qw");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("Συνδέθηκε στο δίκτυο WiFi");
  Serial.print("Διεύθυνση IP: ");
  Serial.println(WiFi.localIP());
}

JSONVar readings;   // Μεταβλητή JSON για την αποθήκευση των μετρήσεων
JSONVar valveValues;
String message = "";
String valve1 = "0";
String valve2 = "0";

// Timer variables
//unsigned long lastTime = 0;
//unsigned long timerDelay = 30000;

BME280 bme;   // Δημιουργία αντικειμένου για τον αισθητήρα BME280
void initBME() {  // Συνάρτηση ενεργοποίησης του αισθητήρα BME280
  Wire.begin();
  if (!bme.beginI2C()) {
    Serial.println("Ο αισθητήρας BME280 δεν βρέθηκε (Έλεγχος της καλωδίωσης)");
    while (1);
  }
}

String getSensorReadings() {  // Get Sensor Readings and return JSON object
  readings["temperature"] = String(bme.readTempC());
  readings["humidity"] =  String(bme.readFloatHumidity());
  readings["pressure"] = String(1);

  String jsonString = JSON.stringify(readings);
  notifyClients(jsonString);  //
  Serial.println(jsonString);
  return jsonString;
}

String getValveValues() {
  valveValues["valve1"] = String(valve1);
  valveValues["valve2"] = String(valve2);

  String jsonString = JSON.stringify(valveValues);
  Serial.println(jsonString);
  return jsonString;
}

void initSPIFFS() {   // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

void notifyClients(String sensorReadings) {
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;

    if (message.indexOf("1b") >= 0) {
      valve1 = message.substring(2);
      //dutyCycle1 = map(sliderValue1.toInt(), 0, 100, 0, 255);
      //Serial.println(dutyCycle1);
      //      Serial.println(valve1);
      //      Serial.print(getValveValues());
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      notifyClients(String(ledState));
      notifyClients(getValveValues());
    }
    if (message.indexOf("2b") >= 0) {
      valve2 = message.substring(2);
      notifyClients(getValveValues());
    }
    // Check if the message is "getReadings"
    if (strcmp((char*)data, "getReadings") == 0) {  //if it is, send current sensor readings
      String sensorReadings = getSensorReadings();
      //Serial.print(sensorReadings);
      notifyClients(sensorReadings);
    }
    if (strcmp((char*)data, "getValveValues") == 0) {  //if it is, send current sensor readings
      String valveReadings = getValveValues();
      //Serial.print(sensorReadings);
      notifyClients(valveReadings);
    }
    //    if (strcmp((char*)data, "toggle") == 0) {
    //      ledState = !ledState;
    //      digitalWrite(ledPin, ledState);
    //      Serial.println(ledState);
    //      notifyClients(String(ledState));
    //    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 0);
  setCpuFrequencyMhz(80);
  initBME();
  initWiFi();
  initSPIFFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/");

  server.begin();   // Start server
  readingsTimer.start();
}

void loop() {
  readingsTimer.update();
  //  if ((millis() - lastTime) > timerDelay) {
  //    String sensorReadings = getSensorReadings();
  //    Serial.println(sensorReadings);
  //    notifyClients(sensorReadings);
  //    lastTime = millis();
  //  }
  ws.cleanupClients();
}