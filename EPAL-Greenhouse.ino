//  https://randomnerdtutorials.com/esp32-web-server-websocket-sliders/
//  https://randomnerdtutorials.com/esp32-websocket-server-arduino/
//  https://randomnerdtutorials.com/esp32-websocket-server-sensor/
//  https://m1cr0lab-esp32.github.io/remote-control-with-websocket/web-ui-design/

// Αλλαγή της toggle
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
String getValveValues();

TickTwo readingsTimer(getSensorReadings, 15000);  // Δημιουργία αντικειμένου για την λήψη τιμών σε τακτά διαστήματα
TickTwo valvesTimer(getValveValues, 10000);

bool ledState = 0;
#define ledPin 2

WiFiMulti wifiMulti;    // Δημιουργία αντικειμένου για το WiFi
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

JSONVar sensorReadings;   // Μεταβλητή JSON για την αποθήκευση των μετρήσεων
JSONVar valveValues;
String message = "";
String valve1 = "0";
String valve2 = "0";
String valve3 = "0";
String valve4 = "0";
String valve5 = "0";

BME280 bme;   // Δημιουργία αντικειμένου για τον αισθητήρα BME280
void initBME() {  // Συνάρτηση ενεργοποίησης του αισθητήρα BME280
  Wire.begin();
  if (!bme.beginI2C()) {
    Serial.println("Ο αισθητήρας BME280 δεν βρέθηκε (Έλεγχος της καλωδίωσης)");
    while (1);
  }
}

String getSensorReadings() {  // Λήψη τιμών από τους αισθητήρες και επιστροφή τους με την μορφή JSON
  float temp = bme.readTempC();
  float hum = bme.readFloatHumidity();
  sensorReadings["temperature"] = String(temp);
  sensorReadings["air-humidity"] =  String(hum);
  sensorReadings["gnd-humidity"] = String(1);

  String jsonString = JSON.stringify(sensorReadings);
  notifyClients(jsonString);
  Serial.println(jsonString);
  if (temp>22.00) {
    valveValues["valve2"] = String(1);
    String jsonValve = JSON.stringify(valveValues);
    notifyClients(jsonValve);
  } else {
    valveValues["valve2"] = String(0);
    String jsonValve = JSON.stringify(valveValues);
    notifyClients(jsonValve);    
  }
  return sensorReadings;
}

String getValveValues() {   // Λήψη της κατάστασης των βανών και επιστροφή τους με την μορφή JSON
  valveValues["valve1"] = String(valve1);
  valveValues["valve2"] = String(valve2);
  valveValues["valve3"] = String(valve3);
  valveValues["valve4"] = String(valve4);
  valveValues["valve5"] = String(valve5);

  String jsonString = JSON.stringify(valveValues);
  Serial.println(jsonString);
  return jsonString;
}

void initSPIFFS() {   // Αρχικοποίηση του χώρου αποθήκευσης SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Δεν ήταν δυνατή η προσάρτηση του SPIFFS");
  }
  Serial.println("Το SPIFFS προσαρτήθηκε με επιτυχία");
}

void notifyClients(String readings) {
  ws.textAll(readings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;

    if (message.indexOf("1b") >= 0) {
      valve1 = message.substring(2);
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      notifyClients(getValveValues());
    }
    if (message.indexOf("2b") >= 0) {
      valve2 = message.substring(2);
      notifyClients(getValveValues());
    }
    if (message.indexOf("3b") >= 0) {
      valve3 = message.substring(2);
      notifyClients(getValveValues());
    }
    if (message.indexOf("4b") >= 0) {
      valve4 = message.substring(2);
      notifyClients(getValveValues());
    }
    if (message.indexOf("5b") >= 0) {
      valve5 = message.substring(2);
      notifyClients(getValveValues());
    }    
    if (message.indexOf("getReadings") >= 0) {
      notifyClients(getSensorReadings());      
    }
    // Check if the message is "getReadings"
//    if (strcmp((char*)data, "getReadings") == 0) {  //if it is, send current sensor readings
//      String sensorReadings = getSensorReadings();
//      //Serial.print(sensorReadings);
//      notifyClients(sensorReadings);
//    }
//    if (strcmp((char*)data, "getValveValues") == 0) {  //if it is, send current sensor readings
//      String valveReadings = getValveValues();
//      //Serial.print(sensorReadings);
//      notifyClients(valveReadings);
//    }
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
//  Serial.println (sensorReadings["temperature"]);
  ws.cleanupClients();
}
