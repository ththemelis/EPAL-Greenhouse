//  https://randomnerdtutorials.com/esp32-web-server-websocket-sliders/
//  https://randomnerdtutorials.com/esp32-websocket-server-arduino/
//  https://randomnerdtutorials.com/esp32-websocket-server-sensor/
//  https://m1cr0lab-esp32.github.io/remote-control-with-websocket/web-ui-design/
// https://medium.com/@predragdavidovic10/native-dual-range-slider-html-css-javascript-91e778134816

#include <WiFi.h>
#include <WiFiMulti.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include "SparkFunBME280.h"
#include "TickTwo.h"

String getSensorReadings();
void checkValves();

TickTwo readingsTimer(getSensorReadings, 15000);  // Δημιουργία αντικειμένου για την λήψη τιμών σε τακτά διαστήματα
TickTwo valvesTimer(checkValves, 15000);    // // Δημιουργία αντικειμένου για έλεγχο των βανών σε τακτά διαστήματα

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
//String message = "";

BME280 bme;   // Δημιουργία αντικειμένου για τον αισθητήρα BME280
void initBME() {  // Συνάρτηση ενεργοποίησης του αισθητήρα BME280
  Wire.begin();
  if (!bme.beginI2C()) {
    Serial.println("Ο αισθητήρας BME280 δεν βρέθηκε (Έλεγχος της καλωδίωσης)");
    while (1);
  }
  bme.setFilter(4);
  bme.setTempOverSample(4);
  bme.setHumidityOverSample(4);
  bme.setPressureOverSample(0);

  bme.setMode(MODE_NORMAL);
}

float airTemperature, airHumidity;
JSONVar sensorReadings;   // Μεταβλητή JSON για την αποθήκευση των μετρήσεων
String getSensorReadings() {  // Λήψη τιμών από τους αισθητήρες και επιστροφή τους με την μορφή JSON
  airTemperature = bme.readTempC();
  airHumidity = bme.readFloatHumidity();
  sensorReadings["airTemperature"] = String(airTemperature);
  sensorReadings["airHumidity"] =  String(airHumidity);
  sensorReadings["gndHumidity"] = String(1);

  notifyClients(JSON.stringify(sensorReadings));
  Serial.println(sensorReadings);
  return JSON.stringify(sensorReadings);
}

JSONVar limitValues;
float airTempLimit_floor=22.0;
float airTempLimit_ceil=32.0;
int airHumLimit_floor=60;
int airHumLimit_ceil=90;
int gndHumLimit_floor=60;
int gndHumLimit_ceil=90;
String getLimitValues() {   // Λήψη των ρυθμίσεων και επιστροφή τους με την μορφή JSON
  limitValues["airTempLimit_floor"] = String(airTempLimit_floor);
  limitValues["airTempLimit_ceil"] = String(airTempLimit_ceil);
  limitValues["airHumLimit_floor"] = String(airHumLimit_floor);
  limitValues["airHumLimit_ceil"] = String(airHumLimit_ceil);
  limitValues["gndHumLimit_floor"] = String(gndHumLimit_floor);
  limitValues["gndHumLimit_ceil"] = String(gndHumLimit_ceil);

  Serial.println(JSON.stringify(limitValues));
  return JSON.stringify(limitValues);
}

JSONVar valveValues;
bool valve1 = 0;
bool valve2 = 0;
bool valve3 = 0;
bool valve4 = 0;
bool valve5 = 0;
String getValveValues() {   // Λήψη της κατάστασης των βανών και επιστροφή τους με την μορφή JSON
  valveValues["valve1"] = String(valve1);
  valveValues["valve2"] = String(valve2);
  valveValues["valve3"] = String(valve3);
  valveValues["valve4"] = String(valve4);
  valveValues["valve5"] = String(valve5);

  Serial.println(JSON.stringify(valveValues));
  return JSON.stringify(valveValues);
}

JSONVar operationValue;
bool operation = 0;   // Χειροκίνητος τρόπος λειτουργίας
String getOperationValue() {   // Λήψη της κατάστασης των βανών και επιστροφή τους με την μορφή JSON
  operationValue["operation"] = String(operation);

  Serial.println(JSON.stringify(operationValue));
  return JSON.stringify(operationValue);
}

void openAllValves() {  // Ενεργοποίηση όλων των βανών
  ledState = 1;
  valve1 = 1;
  digitalWrite (ledPin, 1);  
}

void closeAllValves() { // Απενεργοποίηση όλων των βανών
  ledState = 0;
  valve1 = 0;
  digitalWrite (ledPin, 0);
}

void checkValves() {  // Έλεγχος των βανών ανάλογα με τις τιμές των αισθητήρων και των ορίων που έχουμε θέσει
  if (airTempLimit_floor > airTemperature && operation) {
    openAllValves();
  } else {
    closeAllValves();
  }
  notifyClients(getValveValues());
}

void notifyClients(String readings) {   // Ενημέρωση των client για αλλαγές στην ιστοσελίδα
  ws.textAll(readings);
}

void initSPIFFS() {   // Αρχικοποίηση του χώρου αποθήκευσης SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Δεν ήταν δυνατή η προσάρτηση του SPIFFS");
  }
  Serial.println("Το SPIFFS προσαρτήθηκε με επιτυχία");
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;

    if (message.indexOf("1o") >= 0) {
      operation=!operation;
      notifyClients(getOperationValue());
    }
    if (message.indexOf("1v") >= 0) {
      valve1=!valve1;
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      notifyClients(getValveValues());
    }
    if (message.indexOf("2v") >= 0) {
      valve2=!valve2;
      notifyClients(getValveValues());
    }
    if (message.indexOf("3v") >= 0) {
      valve3=!valve3;
      notifyClients(getValveValues());
    }
    if (message.indexOf("4v") >= 0) {
      valve4=!valve4;
      notifyClients(getValveValues());
    }
    if (message.indexOf("5v") >= 0) {
      valve5=!valve5;
      notifyClients(getValveValues());
    }
    if (message.indexOf("1l") >= 0) {
      airTempLimit_floor = message.substring(2).toFloat();
      notifyClients(getLimitValues());
    }
    if (message.indexOf("2l") >= 0) {
      airTempLimit_ceil = message.substring(2).toFloat();
      notifyClients(getLimitValues());
    }  
    if (message.indexOf("3l") >= 0) {
      airHumLimit_floor = message.substring(2).toFloat();
      notifyClients(getLimitValues());
    }
    if (message.indexOf("4l") >= 0) {
      airHumLimit_ceil = message.substring(2).toFloat();
      notifyClients(getLimitValues());
    }  
    if (message.indexOf("5l") >= 0) {
      gndHumLimit_floor = message.substring(2).toFloat();
      notifyClients(getLimitValues());
    } 
    if (message.indexOf("6l") >= 0) {
      gndHumLimit_ceil = message.substring(2).toFloat();
      notifyClients(getLimitValues());
    } 
    if (message.indexOf("getReadings") >= 0) {
      notifyClients(getSensorReadings());      
    }
    if (message.indexOf("getValveValues") >= 0) {
      notifyClients(getValveValues());      
    }
    if (message.indexOf("getLimitValues") >= 0) {
      notifyClients(getLimitValues());      
    }
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
  pinMode(valve2, OUTPUT);
  pinMode(valve3, OUTPUT);
  pinMode(valve4, OUTPUT);
  pinMode(valve5, OUTPUT);
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
  valvesTimer.start();
}

void loop() {
  readingsTimer.update();
  valvesTimer.update();
  ws.cleanupClients();
}
