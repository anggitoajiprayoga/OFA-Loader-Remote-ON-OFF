void main_setup() {
  if (en_lte == "true") {
    setup_sim7600();
  }
  if (en_lte == "true" and en_mqtt == "true") {
    setup_mqtt();
  }
}

void main_loop() {
  alternator = analogRead(p_alternator);

  if (engine_status == "" or engine_status == "null") {
    engine_status = "engine off";
  }

  if (engine_status == "engine running") {
    if (date_time_str != "") {
      if (date_time_start == "") {
        date_time_start = date_time_str;
        date_time_stop = "";
      }
    }
  }

  if (alternator > 1000 and engine_status == "engine off") {
    date_time_start = date_time_str;
    writeFile(SPIFFS, "/date_time_start.txt", date_time_start.c_str());
    writeFile(SPIFFS, "/date_time_stop.txt", "");
    date_time_stop = "";
    eng_state = "engine_running";
    engine_status = "engine running";
    status_remote = "engine_run";
    writeFile(SPIFFS, "/eng_state.txt", eng_state.c_str());
    String get_live_locations = jsonLiveLocation;
    pub("fms/loader/device/callback/" + cn, "engine_run ");
    pub("fms/liveLocations", get_live_locations);
    pub("fms/loader/liveLocations/" + cn, get_live_locations);
  }
  else if (alternator < 1000 and engine_status == "engine running") {
    if (date_time_start == "") {
      date_time_start = date_time_str;
    }
    date_time_stop = date_time_str;
    writeFile(SPIFFS, "/date_time_stop.txt", date_time_stop.c_str());
    eng_state = "engine_off";
    engine_status = "engine off";
    status_remote = "engine_off";
    writeFile(SPIFFS, "/eng_state.txt", eng_state.c_str());
    String get_live_locations = jsonLiveLocation;
    pub("fms/loader/device/callback/" + cn, "engine_off ");
    pub("fms/liveLocations", get_live_locations);
    pub("fms/loader/liveLocations/" + cn, get_live_locations);
  }

  if (en_lte == "true" and en_mqtt == "true") {
    loop_mqtt();
  }

  if (en_update == true) {
    //    vTaskSuspend(TaskHandle_AtmegaESP);
    static unsigned long prev_update;
    if ((millis() - prev_update) > 300000) {
      pub("ofa/device/callback/" + cn, "retry update OTA proccess");
      Serial.println("Update Firmware by OTA");
      delay(2000);
      ota();
      prev_update = millis();
    }
  }
  //  else {
  //    vTaskResume(TaskHandle_AtmegaESP);
  //  }

  if (firm_ver != xfirm_ver) {
    pub("ofa/device/callback/" + cn, "update OTA done");
    writeFile(SPIFFS, "/xfirm_ver.txt", firm_ver.c_str());
    xfirm_ver = readFile(SPIFFS, "/xfirm_ver.txt");
  }

  static unsigned long prev_rst;
  static unsigned int count_reset;
  if ((millis() - prev_rst) > 1000) {
    count_reset++;
    prev_rst = millis();
  }

  if (count_reset > 10800) {
    Serial.println("schedule device restart");
    delay(2000);
    ESP.restart();
  }

  static unsigned long prev_send_atmega;
  if ((millis() - prev_send_atmega) > 3000) {
    send_atmega();
    prev_send_atmega = millis();
  }
}

void mqtt_parse(String input) {
  Serial.println();
  Serial.println("-----MQTT Payload Parse-----");
  Serial.println();
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, input);
  JsonObject obj = doc.as<JsonObject>();

  String xsn  = obj["sn"].as<String>();
  String xcn  = obj["cn"].as<String>();
  String xtime_zone  = obj["time_zone"].as<String>();
  String xno_simcard  = obj["no_simcard"].as<String>();
  String xen_lte  = obj["en_lte"].as<String>();
  String xen_mqtt  = obj["en_mqtt"].as<String>();
  String xbroker  = obj["broker"].as<String>();
  String xusername  = obj["username"].as<String>();
  String xpassword  = obj["password"].as<String>();
  String xinterval_mqtt  = obj["interval_mqtt"].as<String>();
  String xserver_ota  = obj["server_ota"].as<String>();
  String xresource_ota  = obj["resource_ota"].as<String>();

  String xcrank  = obj["crank"].as<String>();
  String xshutdown  = obj["shutdown"].as<String>();

  if (xcrank != "null" and xcrank != "") {
    if (xcrank == "true") {
      if (analogRead(p_alternator) > 1000) {
        Serial.println("crank trigger failed");
        status_remote = "crank trigger failed";
        String get_live_locations = jsonLiveLocation;
        pub("fms/loader/device/callback/" + cn, "crank trigger failed");
        pub("fms/liveLocations", get_live_locations);
        pub("fms/loader/liveLocations/" + cn, get_live_locations);
      }
      else {
        Serial.println("crank trigger success");
        status_remote = "crank trigger process";
        relay_crank();
        String get_live_locations = jsonLiveLocation;
        pub("fms/loader/device/callback/" + cn, "crank trigger process");
        pub("fms/liveLocations", get_live_locations);
        pub("fms/loader/liveLocations/" + cn, get_live_locations);
      }
      xcrank = "";
    }
  }

  if (xshutdown != "null" and xshutdown != "") {
    if (xshutdown == "true") {
      if (analogRead(p_alternator) > 1000) {
        Serial.println("eng_off trigger");
        status_remote = "engine_off process";
        String get_live_locations = jsonLiveLocation;
        pub("fms/loader/device/callback/" + cn, "engine_off process");
        pub("fms/liveLocations", get_live_locations);
        pub("fms/loader/liveLocations/" + cn, get_live_locations);
        relay_shutdown();
      }
      else {
        Serial.println("eng_off trigger failed");
        status_remote = "engine_off trigger failed";
        String get_live_locations = jsonLiveLocation;
        pub("fms/loader/device/callback/" + cn, "engine_off triger failed");
        pub("fms/liveLocations", get_live_locations);
        pub("fms/loader/liveLocations/" + cn, get_live_locations);
      }
      xshutdown = "";
    }
  }

  if (xsn != "null" and xsn != "") {
    sn = xsn;
    writeFile(SPIFFS, "/sn.txt", xsn.c_str());
  }
  if (xcn != "null" and xcn != "") {
    cn = xcn;
    writeFile(SPIFFS, "/cn.txt", xcn.c_str());
  }
  if (xtime_zone != "null" and xtime_zone != "") {
    time_zone_int = xtime_zone.toInt();
    timeZone = "+" + String(time_zone_int) + ":00";
    writeFile(SPIFFS, "/time_zone.txt", xtime_zone.c_str());
  }
  if (xno_simcard != "null" and xno_simcard != "") {
    no_simcard = xno_simcard;
    writeFile(SPIFFS, "/no_simcard.txt", xno_simcard.c_str());
  }
  if (xen_lte != "null" and xen_lte != "") {
    en_lte = xen_lte;
    writeFile(SPIFFS, "/en_lte.txt", xen_lte.c_str());
  }
  if (xen_mqtt != "null" and xen_mqtt != "") {
    en_mqtt = xen_mqtt;
    writeFile(SPIFFS, "/en_mqtt.txt", xen_mqtt.c_str());
  }
  if (xbroker != "null" and xbroker != "") {
    broker = xbroker;
    writeFile(SPIFFS, "/broker.txt", xbroker.c_str());
  }
  if (xusername != "null" and xusername != "") {
    username = xusername;
    writeFile(SPIFFS, "/username.txt", xusername.c_str());
  }
  if (xpassword != "null" and xpassword != "") {
    password = xpassword;
    writeFile(SPIFFS, "/password.txt", xpassword.c_str());
  }
  if (xinterval_mqtt != "null" and xinterval_mqtt != "") {
    interval_mqtt = xinterval_mqtt;
    writeFile(SPIFFS, "/interval_mqtt.txt", xinterval_mqtt.c_str());
  }
  if (xserver_ota != "null" and xserver_ota != "") {
    server_ota = xserver_ota;
    writeFile(SPIFFS, "/server_ota.txt", xserver_ota.c_str());
  }
  if (xresource_ota != "null" and xresource_ota != "") {
    resource_ota = xresource_ota;
    writeFile(SPIFFS, "/resource_ota.txt", xresource_ota.c_str());
    String config = config_device();
    Serial.println(config);
  }
}

bool check_json(String data) {
  bool json_status = false;
  if (data.indexOf('{') != -1 and data.indexOf('}') != -1) {
    json_status = true;
    Serial.println("JSON lengkap");
  } else {
    Serial.println("JSON tidak lengkap");
  }
  return json_status;
}

void relay_crank() {
  digitalWrite(RELAY2, LOW);
  delay(3000);
  digitalWrite(RELAY2, HIGH);
}

void relay_shutdown() {
  digitalWrite(RELAY1, LOW);
  delay(5000);
  digitalWrite(RELAY1, HIGH);
}
