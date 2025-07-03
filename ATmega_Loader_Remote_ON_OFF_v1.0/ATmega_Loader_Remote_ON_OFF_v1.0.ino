//#define SerialRs232 Serial2
#define SerialEsp32 Serial1
#define SerialGps Serial3
#define addressEepromCN 15

#include <ArduinoJson.h>
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <EEPROM.h>

TinyGPSPlus gps;
time_t prevDisplay = 0;

char datex[10], timex[10];
int UTC_offset = 8;
String timeZone_str = "8";

String cn_str = "";
String sn_str = "";

String dateGps_str, timeGps_str, dateTimeGps_str, lat_str, lng_str, alt_str, spd_str, hdg_str, sat_str;
float lat_float, lng_float, alt_float, spd_float, hdg_float, sat_float;

String packet;

unsigned long lastMillisEsp32, lastMillisDebug;

void(*rst) (void) = 0;

void setup() {
  Serial.begin(115200); // Debug
  SerialEsp32.begin(9600); // ESP32
  SerialGps.begin(9600); // GPS
  setupEeprom();
}

void loop() {
  loopGps();
  receiveEsp32();
  sendEsp32();

  if (millis() - lastMillisDebug > 2000) {
    lastMillisDebug = millis();

    Serial.println("\nDate: " + dateGps_str);
    Serial.println("Time: " + timeGps_str);
    Serial.println("Latitude: " + lat_str);
    Serial.println("Longitude: " + lng_str);
    Serial.println("Altitude: " + alt_str);
    Serial.println("Speed: " + spd_str);
    Serial.println("Heading: " + hdg_str);
    Serial.println("Satelite: " + sat_str);
  }
}
