#define TINY_GSM_MODEM_SIM7600

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <Update.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>
#include "FS.h"
#include "SD.h"
#include "SPIFFS.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define RXD1 27
#define TXD1 26
#define PKEY 14
#define RST 12
#define PROTOCOL SERIAL_8N1
#define SerialAT Serial1
#define RELAY 0
#define RELAY1 4
#define RELAY2 15
#define p_alternator 39
#define SerialAtmega2560 Serial2
#define WDT_TIMEOUT 60
#define SD_CS 5

String type = "PC";
String version = "Firm v1.7 - OFA Remote";
String firm_ver = "v1.7 Remote";
String xfirm_ver;
String type_board = "OFA Board";
String type_str = "PC";
String timeZone = "+8:00";
String sn_dev;

const char apn[]      = "Internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

String http_username;
String http_password;
String ssid_wifi_client;
String pass_wifi_client;
String ssid_wifi_ap;
String pass_wifi_ap;
String freesp, usedstr, totalstr;

bool en_update;
String webpage = "";
bool shouldReboot = false;
String listFiles(bool ishtml = false);

String server_ota, resource_ota;
const int port_ota = 80;

const String default_ssid = "";
const String default_wifipassword = "SS6";
const String default_httpuser = "admin";
const String default_httppassword = "admin";
const int default_webserverporthttp = 80;

String ssid, packet, jsonLiveLocation;
String wifipassword;
String httpuser;
String httppassword;
int webserverporthttp;

String sn, cn;
String ind;
String no_simcard;
String en_lte, en_mqtt, broker, username, password, mqtt_state, interval_mqtt;
String date_time_str;
String time_zone_str;
String lat_str;
String lng_str;
String alt_str;
String spd_str;
String hdg_str;
String sat_str;
String imei, rssi;
float lat_float, lng_float, alt_float, spd_float, hdg_float, sat_float;
String lastFileDate = "", eng_state = "engine_off", engine_status = "engine off", status_remote = "not available";
int alternator, time_zone_int;
String date_time_start, date_time_stop;

static unsigned long last_live_locations;
static unsigned long config_dev;
static unsigned long last_provider;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WiFiServer serverAP(80);
TaskHandle_t TaskHandle_AtmegaESP;
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

String payload() {
  String jsonString = "";
  DynamicJsonDocument buffer(2048);
  JsonObject obj            = buffer.to<JsonObject>();
  obj["sn"]                 = sn;
  obj["cn"]                 = cn;
  obj["dateTime"]           = date_time_str;
  obj["timeZone"]           = timeZone;
  obj["lat"]                = lat_float;
  obj["lng"]                = lng_float;
  obj["alt"]                = alt_float;
  obj["hdg"]                = hdg_float;
  obj["spd"]                = spd_float;
  obj["sat"]                = sat_float;
  serializeJson(obj, jsonString);
  return jsonString;
}

String config_device() {
  String jsonString = "";
  DynamicJsonDocument buffer(1024);
  JsonObject obj = buffer.to<JsonObject>();
  obj["sn"] = sn;
  obj["cn"] = cn;
  obj["en_lte"] = en_lte;
  obj["en_mqtt"] = en_mqtt;
  obj["broker"] = broker;
  obj["username"] = username;
  obj["password"] = password;
  obj["interval_mqtt"] = interval_mqtt;
  obj["server_ota"] = server_ota;
  obj["resource_ota"] = resource_ota;
  obj["firm_version"] = firm_ver;
  obj["no_simcard"]   = no_simcard;
  serializeJson(obj, jsonString);
  return jsonString;
}

void device_communication(void *parameter) {
  while (true) {
    if (en_update == false) {
      receiveAtmega2560();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void notifyClients(String payload) {
  ws.textAll(payload);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final and info->index == 0 and info->len == len and info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.println((char*)data);
    mqtt_parse(String((char*)data));
    String get_payload = payload();
    notifyClients(get_payload);
    if (strcmp((char*)data, "eng_off") == 0) {
      Serial.println("eng_off trigger");
      relay_shutdown();
    }
    else if (strcmp((char*)data, "crank") == 0) {
      if (analogRead(p_alternator) > 1000) {
        Serial.println("crank trigger failed");
      }
      else {
        Serial.println("crank trigger success");
        relay_crank();
      }
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
      Serial.printf("WebSocket client #%u pong received\n", client->id());
      break;
    case WS_EVT_ERROR:
      Serial.println("WebSocket error occurred. Restarting ESP32...");
      delay(2000);
      ESP.restart();
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  Serial.begin(115200);
  SerialAtmega2560.begin(9600);
  pinMode(p_alternator, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  setup_spiffs();
  send_atmega();
  setup_wifi();
  setup_sd_card();
  setup_webserver();
  initWebSocket();
  main_setup();
  xTaskCreate(device_communication, "Atmega ESP", 4096, NULL, 1, &TaskHandle_AtmegaESP);
  esp_task_wdt_init(WDT_TIMEOUT, true); 
  esp_task_wdt_add(NULL); 
}

void loop() {
  receiveAtmega2560();
  loop_wifi();
  main_loop();
  esp_task_wdt_reset();
}
