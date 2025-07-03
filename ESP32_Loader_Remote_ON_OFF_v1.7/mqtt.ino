void pub(String topic, String _jsonDataString) {
  if (mqtt.connected()) {
    char topicChar[topic.length() + 1];
    char _jsonDataStringChar[_jsonDataString.length() + 1];
    topic.toCharArray(topicChar, sizeof(topicChar));
    _jsonDataString.toCharArray(_jsonDataStringChar, sizeof(_jsonDataStringChar));

    mqtt.publish(topicChar, _jsonDataStringChar, false);
    Serial.println("\n" + topic + "\n" + _jsonDataString);
    Serial.println(_jsonDataString.length());
  }
}

boolean mqttConnect() {
  Serial.print("Connecting to MQTT broker...");
  if (!mqtt.connect(cn.c_str(), username.c_str(), password.c_str())) {
    int state = mqtt.state();
    Serial.println("Failed to connect, state: " + String(state));

    switch (state) {
      case -4: Serial.println("Error: MQTT_CONNECTION_TIMEOUT"); break;
      case -3: Serial.println("Error: MQTT_CONNECTION_LOST"); break;
      case -2: Serial.println("Error: MQTT_CONNECT_FAILED"); break;
      case -1: Serial.println("Error: MQTT_DISCONNECTED"); break;
      case  1: Serial.println("Error: MQTT_CONNECT_BAD_PROTOCOL"); break;
      case  2: Serial.println("Error: MQTT_CONNECT_BAD_CLIENT_ID"); break;
      case  3: Serial.println("Error: MQTT_CONNECT_UNAVAILABLE"); break;
      case  4: Serial.println("Error: MQTT_CONNECT_BAD_CREDENTIALS"); break;
      case  5: Serial.println("Error: MQTT_CONNECT_UNAUTHORIZED"); break;
      default: Serial.println("Error: Unknown error"); break;
    }
    return false;
  }

  mqtt.subscribe(("equipment/operated/" + sn).c_str());
  mqtt.subscribe(("equipment/status/" + cn).c_str());
  mqtt.subscribe("fms/loader/remote");
  mqtt.subscribe(("fms/loader/remote/" + cn).c_str());

  Serial.println("Connected to broker.");
  return mqtt.connected();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String pay = parsing_subscribe(topic, payload, length);
  Serial.println("Data Payload Subscribe : " + pay);

  if (pay == "restart") {
    pub("fms/loader/device/callback/" + cn, "restrat device");
    Serial.println("restart device");
    delay(2000);
    ESP.restart();
  }

  String topicStr = String(topic);
  if (topicStr == "fms/loader/remote" || topicStr == "fms/loader/remote/" + cn) {
    pub("fms/loader/device/callback/" + cn, "device message received");
    mqtt_parse(pay);
  }

  if (pay == "update") {
    pub("fms/loader/device/callback/" + cn, "update OTA proccess");
    Serial.println("Update Firmware by OTA");
    delay(2000);
    ota();
  }

  if (topicStr == "equipment/operated/" + sn) {
    DynamicJsonDocument doc(512);
    if (deserializeJson(doc, pay)) {
      Serial.println("[MQTT] Json parse error...!!!");
    } 
    else {
      Serial.println("[MQTT] Json parse success.");
      String allow_start = doc["allowStart"].as<String>();
      allow_start.trim();
      Serial.println("[MQTT] allowStart: " + allow_start);

      if (allow_start == "true") {
        digitalWrite(RELAY, HIGH);
        JsonObject data = doc["data"];
        Serial.println("[MQTT] operatedBy: " + doc["operatedBy"].as<String>());
        Serial.println("[MQTT] nrp: " + data["nrp"].as<String>());
        Serial.println("[MQTT] nama: " + data["nama"].as<String>());
        Serial.println("[MQTT] jabatan: " + data["jabatan"].as<String>());
        Serial.println("[MQTT] posisi: " + data["posisi"].as<String>());
      } 
      else {
        digitalWrite(RELAY, LOW);
      }
    }
  } else if (topicStr == "equipment/status/" + cn) {
    DynamicJsonDocument doc(512);
    if (deserializeJson(doc, pay)) {
      Serial.println("[MQTT] Json parse error...!!!");
    }
    else {
      Serial.println("[MQTT] Json parse success.");
      String status_precise = doc["statusPrecise"].as<String>();
      status_precise.trim();
      Serial.println("[MQTT] statusPrecise: " + status_precise);

      if (status_precise == "true") {
        Serial2.println("{\"sta_cas\":true}");
        Serial.println(">> send atmega: true");
      } 
      else if (status_precise == "false") {
        Serial2.println("{\"sta_cas\":false}");
        Serial.println(">> send atmega: false");
      }
    }
  }
}

String parsing_subscribe(char* topic, byte* payload, unsigned int length) {
  String result;
  for (unsigned int i = 0; i < length; i++) {
    result += (char)payload[i];
  }
  return result;
}

bool connectToNetwork(byte max_attempts = 3) {
  byte count_network = 0;
  bool connected = modem.waitForNetwork();

  while (!connected && count_network < max_attempts) {
    Serial.println("fail, count return: " + String(++count_network));
    connected = modem.waitForNetwork();
  }

  if (connected) {
    Serial.println("Connected to provider.");
    Serial.println("Signal Quality: " + String(modem.getSignalQuality()));
  } 
  else {
    Serial.println("Not Connect to provider.");
  }

  Serial.println();
  return connected;
}

bool connectToGPRS(byte max_attempts = 3) {
  byte count_gprs = 0;
  bool connected = modem.gprsConnect("internet", "", "");

  while (!connected && count_gprs < max_attempts) {
    Serial.println("fail, count return: " + String(++count_gprs));
    connected = modem.gprsConnect("internet", "", "");
  }

  if (connected) {
    Serial.println("Connected to GPRS: " + String(apn));
  } 
  else {
    Serial.println("Not Connect to GPRS: " + String(apn));
  }

  Serial.println();
  return connected;
}

void connectToMQTT(byte max_attempts = 3) {
  mqtt.setServer((char*) broker.c_str(), 1883);
  mqtt.setCallback(callback);

  Serial.println("Connecting to MQTT Broker: " + String(broker));
  Serial.println("username: " + username);
  Serial.println("password: " + password);

  byte count_mqtt = 0;
  while (!mqttConnect() && count_mqtt < max_attempts) {
    Serial.println("fail, count return: " + String(++count_mqtt));
  }

  Serial.print("mqtt state: " + String(mqtt.state()));
  Serial.println();
}

void reconnect() {
  connectToNetwork();
  Serial.println("Connecting to GPRS network.");
  connectToGPRS();
  connectToMQTT();
}

void setup_mqtt() {
  Serial.println();
  Serial.println("Searching for provider.");
  connectToNetwork();
  Serial.println("Connecting to GPRS network.");
  connectToGPRS();
  connectToMQTT();
}

void loop_mqtt() {
  static int count_reconnect = 0;
  static int count_rst = 0;

  if (!mqtt.connected()) {
    rssi = "unknown";
    Serial.println("count reconnect: " + String(count_reconnect));
    Serial.println("count rst: " + String(count_rst));
    mqtt_state = "Not_Connect";
    esp_task_wdt_reset();

    if (count_reconnect < 3) {
      reconnect();
      count_reconnect++;
    }
    else {
      if (++count_reconnect > 3000) {
        count_reconnect = 0;
        count_rst++;
      }
    }

    if (count_rst == 3) {
      Serial.println("\nRestart System !!!\n");
      delay(3000);
      ESP.restart();
    }
  }
  else {
    rssi = String(-113 + (2 * modem.getSignalQuality()));
    mqtt.loop();
    mqtt_state = "Connect";

    static unsigned long prev_mqtt = 0;
    if (millis() - prev_mqtt > interval_mqtt.toInt() * 1000) {
      if (jsonLiveLocation != "null" && lat_float != 0 && lng_float != 0) {
        Serial.println("live locations valid");
        pub("fms/liveLocations", jsonLiveLocation);
        pub("fms/loader/liveLocations/" + cn, jsonLiveLocation);
        loggingLiveLoc();
      }
      else {
        Serial.println("live locations invalid");
      }

      String configdevice = config_device();
      if (configdevice != "null") {
        pub("fms/loader/config/" + cn, configdevice);
        delay(150);
      }
      prev_mqtt = millis();
    }
    delay(100);
  }
}
