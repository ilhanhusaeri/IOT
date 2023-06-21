#include "painlessMesh.h"
#include <ArduinoJson.h>
//#include <DHT.h>

// WiFi Credentials
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

int led;
int led_status = 0;
int board_status = 0;

int mq_pin = 22; // Pin yang digunakan untuk sensor MQ

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// Needed for painless library
void receivedCallback( uint32_t from, String &msg )
{
  //Deserializing
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  board_status = doc["board"];
  led = doc["pin"];
  led_status = doc["status"];
  //Serial.print("Data "); Serial.println(led_status_1);
  if (board_status == 1) {
    digitalWrite(led, led_status);
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);

  pinMode(mq_pin, INPUT); // Mengatur pin sensor MQ sebagai input

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP);  // set before init() so that you can see startup messages
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void loop() {
  // Membaca nilai sensor MQ
  int mq_value = analogRead(mq_pin);

  // Mengubah nilai sensor MQ menjadi objek JSON
  StaticJsonDocument<100> jsonDoc;
  jsonDoc["mq"] = mq_value;
  String jsonData;
  serializeJson(jsonDoc, jsonData);

  // Mengirim data sensor MQ ke seluruh node dalam jaringan mesh
  mesh.sendBroadcast(jsonData);

  mesh.update();
}
