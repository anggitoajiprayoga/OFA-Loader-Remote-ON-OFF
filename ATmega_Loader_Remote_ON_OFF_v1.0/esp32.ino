void sendEsp32() {
  if (millis() - lastMillisEsp32 > 1500 and dateTimeGps_str.length() > 0 and lat_str.length() > 0 and lng_str.length() > 0) {
    String message = "";
    DynamicJsonDocument obj(1024);

    obj["dateTime"] = dateTimeGps_str;
    obj["timeZone"] = "+" + timeZone_str + ":00";
    obj["lat"]      = lat_float;
    obj["lng"]      = lng_float;
    obj["alt"]      = alt_float;
    obj["spd"]      = spd_float;
    obj["hdg"]      = hdg_float;
    obj["sat"]      = sat_float;
    
    serializeJson(obj, message);
    SerialEsp32.println(message);
    Serial.println("## send ESP32: " + message);
    Serial.println("## length message: " + String(message.length()));
    lastMillisEsp32 = millis();
  }
}

void receiveEsp32() {
  while (SerialEsp32.available()) {
    packet = SerialEsp32.readStringUntil('\n');
    Serial.println("## esp32 packet: " + packet);

    DynamicJsonDocument buffer(1024);
    String input = packet;
    deserializeJson(buffer, input);
    JsonObject obj = buffer.as<JsonObject>();

    cn_str = obj["cn"].as<String>();
    timeZone_str = obj["timeZone"].as<String>();

    if (timeZone_str.toInt() == 7 or timeZone_str.toInt() == 8 or timeZone_str.toInt() == 9){
      UTC_offset = timeZone_str.toInt();
    }
    else {
      UTC_offset = 8;
    }
    
    checkCn();
  }
}

void checkCn() {
  String cnx = readStringFromEEPROM(addressEepromCN);
  int pos = cnx.indexOf('*');
  String cnx_str = cnx.substring(0, pos);
  cn_str.trim();
  cnx_str.trim();

  if (cnx_str != cn_str) {
    writeStringToEEPROM(addressEepromCN, cnx);

    cnx = readStringFromEEPROM(addressEepromCN);
    cn_str = cnx.substring(0, pos);
    Serial.println("## change cn from ESP32: " + cn_str);
  }
}
