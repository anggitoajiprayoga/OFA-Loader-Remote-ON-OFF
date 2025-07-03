void receiveAtmega2560() {
  while (SerialAtmega2560.available()) {
    packet = SerialAtmega2560.readStringUntil('\n');
    Serial.println("\n## atmega2560 packet: " + packet);
    Serial.println("## length: " + String(packet.length()));

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, packet);

    if (error) {
      Serial.print("⚠️ JSON Parsing Error: ");
      Serial.println(error.c_str());
      return;  
    }

    JsonObject obj = doc.as<JsonObject>();

    String xdate_time_str = obj["dateTime"].as<String>();
    String xtime_zone_str = obj["timeZone"].as<String>();
    float xlat_float = obj["lat"].as<float>();
    float xlng_float = obj["lng"].as<float>();
    float xalt_float = obj["alt"].as<float>();
    float xspd_float = obj["spd"].as<float>();
    float xhdg_float = obj["hdg"].as<float>();
    float xsat_float = obj["sat"].as<float>();

    int validCount = 0;

    if (!isnan(xlat_float) && xlat_float != 0) validCount++;
    if (!isnan(xlng_float) && xlng_float != 0) validCount++;
    if (!isnan(xalt_float) && xalt_float != 0) validCount++;
    if (!isnan(xspd_float) && xspd_float != 0) validCount++;
    if (!isnan(xhdg_float) && xhdg_float != 0) validCount++;
    if (!isnan(xsat_float) && xsat_float != 0) validCount++;

    if (validCount > 0) {
      if (xdate_time_str != "" && xdate_time_str != "null") {
        date_time_str = xdate_time_str;
      }

      if (xtime_zone_str != "" && xtime_zone_str != "null") {
        time_zone_str = xtime_zone_str;
      }

      if (!isnan(xlat_float) && xlat_float != 0) lat_float = xlat_float;
      if (!isnan(xlng_float) && xlng_float != 0) lng_float = xlng_float;
      if (!isnan(xalt_float) && xalt_float != 0) alt_float = xalt_float;
      if (!isnan(xspd_float) && xspd_float != 0) spd_float = xspd_float;
      if (!isnan(xhdg_float) && xhdg_float != 0) hdg_float = xhdg_float;
      if (!isnan(xsat_float) && xsat_float != 0) sat_float = xsat_float;
    }

    writeFile(SPIFFS, "/date_time_str.txt", date_time_str.c_str());

    //******************************** data live location ********************************
    jsonLiveLocation = "";
    StaticJsonDocument<1024> bufLiveLocation;
    JsonObject objLiveLocation = bufLiveLocation.to<JsonObject>();

    objLiveLocation["cn"]         = cn;
    objLiveLocation["sn"]         = sn;
    objLiveLocation["type"]       = type_str;
    objLiveLocation["dateTime"]   = date_time_str;
    objLiveLocation["timeZone"]   = timeZone;
    objLiveLocation["lat"]        = lat_float;
    objLiveLocation["lng"]        = lng_float;
    objLiveLocation["alt"]        = alt_float;
    objLiveLocation["spd"]        = spd_float;
    objLiveLocation["hdg"]        = hdg_float;
    objLiveLocation["rssi"]       = get_signal_quality();
    objLiveLocation["alternator"] = String(alternator);
    objLiveLocation["status_engine"] = engine_status;
    objLiveLocation["status_remote"] = status_remote;
    objLiveLocation["date_time_start"] = date_time_start;
    objLiveLocation["date_time_stop"] = date_time_stop;
    objLiveLocation["firm"]       = firm_ver;
    serializeJson(objLiveLocation, jsonLiveLocation);
    //serializeJson(objLiveLocation, Serial);
  }
}

void send_atmega() {
  String message = "";
  DynamicJsonDocument obj(1024);

  obj["cn"] = cn;
  obj["timeZone"] = time_zone_int;

  serializeJson(obj, message);
  SerialAtmega2560.println(message);
}
