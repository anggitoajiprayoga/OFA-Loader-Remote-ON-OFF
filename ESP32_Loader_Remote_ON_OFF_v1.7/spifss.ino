void setup_spiffs() {
  Serial.println("\nInitializin SPIFFS.");
  if (!SPIFFS.begin(true)) {
    Serial.println("\nFailed initializin SPIFFS.");
  }
  else {
    Serial.println("Succes initializing SPIFFS.");
  }

  cn = readFile(SPIFFS, "/cn.txt");
  sn = readFile(SPIFFS, "/sn.txt");

  if (cn == "" and sn == "")
  {
    data_default();
    delay(200);
  }

  cn = readFile(SPIFFS, "/cn.txt");
  sn = readFile(SPIFFS, "/sn.txt");
  time_zone_int = readFile(SPIFFS, "/time_zone.txt").toInt();
  timeZone = "+" + String(time_zone_int) + ":00";
  http_username = readFile(SPIFFS, "/http_username.txt");
  http_password = readFile(SPIFFS, "/http_password.txt");
  ssid_wifi_ap = cn;
  ssid_wifi_ap =  readFile(SPIFFS, "/cn.txt");
  pass_wifi_ap =  readFile(SPIFFS, "/pass_wifi_ap.txt");
  sn_dev = readFile(SPIFFS, "/sn_dev.txt");
  en_lte = readFile(SPIFFS, "/en_lte.txt");
  no_simcard = readFile(SPIFFS, "/no_simcard.txt");
  en_mqtt = readFile(SPIFFS, "/en_mqtt.txt");
  broker = readFile(SPIFFS, "/broker.txt");
  username = readFile(SPIFFS, "/username.txt");
  password = readFile(SPIFFS, "/password.txt");
  interval_mqtt = readFile(SPIFFS, "/interval_mqtt.txt");
  server_ota = readFile(SPIFFS, "/server_ota.txt");
  resource_ota = readFile(SPIFFS, "/resource_ota.txt");
  xfirm_ver = readFile(SPIFFS, "/xfirm_ver.txt");
  date_time_start = readFile(SPIFFS, "/date_time_start.txt");
  date_time_stop = readFile(SPIFFS, "/date_time_stop.txt");
  eng_state = readFile(SPIFFS, "/eng_state.txt");
  date_time_str = readFile(SPIFFS, "/date_time_str.txt");

  Serial.print("SPIFFS Free: "); Serial.println(humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes())));
  Serial.print("SPIFFS Used: "); Serial.println(humanReadableSize(SPIFFS.usedBytes()));
  Serial.print("SPIFFS Total: "); Serial.println(humanReadableSize(SPIFFS.totalBytes()));
}

void data_default()
{
  if (String(readFile(SPIFFS, "/cn.txt")) == "") {
    cn = "OFA Loader";
    writeFile(SPIFFS, "/cn.txt", "OFA Loader");
  }
  else {
    cn = readFile(SPIFFS, "/cn.txt");
  }

  writeFile(SPIFFS, "/http_username.txt", "admin");
  writeFile(SPIFFS, "/http_password.txt", "admin");
  writeFile(SPIFFS, "/cn.txt", cn.c_str());
  writeFile(SPIFFS, "/sn.txt", "1234");
  writeFile(SPIFFS, "/time_zone.txt", "8");
  writeFile(SPIFFS, "/ssid_wifi_ap.txt", cn.c_str());
  writeFile(SPIFFS, "/pass_wifi_ap.txt", "12345678");
  writeFile(SPIFFS, "/no_simcard.txt", "08xxxxxxxxx");
  writeFile(SPIFFS, "/en_lte.txt", "false");
  writeFile(SPIFFS, "/en_mqtt.txt", "false");
  writeFile(SPIFFS, "/broker.txt", "ppa-ipt.net");
  writeFile(SPIFFS, "/username.txt", "device-dev");
  writeFile(SPIFFS, "/password.txt", "H6Jn8trN");
  writeFile(SPIFFS, "/interval_mqtt.txt", "5");
  writeFile(SPIFFS, "/server_ota.txt", "fmw.ppa-bib.net");
  writeFile(SPIFFS, "/resource_ota.txt", "/OFA/IPT/loader/v1.0.bin");
  writeFile(SPIFFS, "/firm_ver.txt", "v1.0");
  writeFile(SPIFFS, "/eng_state.txt", "engine_off");
  Serial.println("default ok");
}

String readFile(fs::FS &fs, const char * path) {
  File file = fs.open(path, "r");
  if (!file || file.isDirectory())
  {
    return String();
  }
  String fileContent;
  while (file.available())
  {
    fileContent += String((char)file.read());
  }
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message)
{
  File file = fs.open(path, "w");
  if (!file)
  {
    return;
  }
  file.print(message);
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.print(F("Message appending : "));
    Serial.println(message);
    Serial.println("Message appended");
  }
  else {
    Serial.println("Append failed");
  }
  file.close();
}

String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

void notFoundd(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

String processor(const String& var)
{
  if (var == "version")
  {
    return version;
  }
  else if (var == "type_board")
  {
    return type_board;
  }
  else if (var == "sn_dev")
  {
    return readFile(SPIFFS, "/sn_dev.txt");
  }
  else if (var == "cn")
  {
    return readFile(SPIFFS, "/cn.txt");
  }
  else if (var == "sn")
  {
    return readFile(SPIFFS, "/sn.txt");
  }
  else if (var == "no_simcard")
  {
    return readFile(SPIFFS, "/no_simcard.txt");
  }
  else if (var == "en_lte")
  {
    return en_lte;
  }

  else if (var == "selected_wib")
  {
    String x = readFile(SPIFFS, "/time_zone.txt");

    if (x == "7")
    {
      return "selected";
    }
    else if (x == "8")
    {
      return " ";
    }
    else if (x == "9")
    {
      return " ";
    }
  }

  else if (var == "selected_wita")
  {
    String x = readFile(SPIFFS, "/time_zone.txt");

    if (x == "7")
    {
      return " ";
    }
    else if (x == "8")
    {
      return "selected";
    }
    else if (x == "9")
    {
      return " ";
    }
  }

  else if (var == "selected_wit")
  {
    String x = readFile(SPIFFS, "/time_zone.txt");

    if (x == "7")
    {
      return " ";
    }
    else if (x == "8")
    {
      return " ";
    }
    else if (x == "9")
    {
      return "selected";
    }
  }

  else if (var == "selected_enablelte")
  {
    String x = readFile(SPIFFS, "/en_lte.txt");

    if (x == "true")
    {
      return "selected";
    }
    else if (x == "false")
    {
      return " ";
    }
  }

  else if (var == "selected_disablelte")
  {
    String x = readFile(SPIFFS, "/en_lte.txt");

    if (x == "true")
    {
      return " ";
    }
    else if (x == "false")
    {
      return "selected";
    }
  }

  else if (var == "en_mqtt")
  {
    return en_mqtt;
  }

  else if (var == "selected_enablemqtt")
  {
    String x = readFile(SPIFFS, "/en_mqtt.txt");

    if (x == "true")
    {
      return "selected";
    }
    else if (x == "false")
    {
      return " ";
    }
  }

  else if (var == "selected_disablemqtt")
  {
    String x = readFile(SPIFFS, "/en_mqtt.txt");

    if (x == "true")
    {
      return " ";
    }
    else if (x == "false")
    {
      return "selected";
    }
  }

  else if (var == "mqtt_state")
  {
    return mqtt_state;
  }

  else if (var == "broker")
  {
    return broker;
  }

  else if (var == "username")
  {
    return username;
  }

  else if (var == "password")
  {
    return password;
  }

  else if (var == "interval_mqtt")
  {
    return readFile(SPIFFS, "/interval_mqtt.txt");
  }

  else if (var == "server_ota")
  {
    return readFile(SPIFFS, "/server_ota.txt");
  }

  else if (var == "resource_ota")
  {
    return readFile(SPIFFS, "/resource_ota.txt");
  }

  else if (var == "freespace") {
    int xfreesp = SD.totalBytes() / (1024 * 1024);
    freesp = String(xfreesp) + " MB";
    return freesp;
  }

  else if (var == "usedstorage") {
    int xusedstr = SD.usedBytes() / (1024 * 1024);
    usedstr = String(xusedstr) + " MB";
    return usedstr;
  }

  else if (var == "totalStorage") {
    int xtotalstr = SD.cardSize() / (1024 * 1024);
    totalstr = String(xtotalstr) + " MB";
    return totalstr;
  }

  else
  {
    return String();
  }
}

void server_begin()
{
  server.serveStatic("/", SPIFFS, "/");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    if (!request->authenticate(http_username.c_str(), http_password.c_str()))
      return request->requestAuthentication();
    request->send(SPIFFS, "/dashboard_ws.html", String(), false, processor);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(401);
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(401);
  });

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    if (!request->authenticate(http_username.c_str(), http_password.c_str()))
      return request->requestAuthentication();
    Serial.println("================");
    Serial.println("Device Restart");
    Serial.println("================");
    delay(3000);
    ESP.restart();
    request->send(SPIFFS, "/dashboard_ws.html", String(), false, processor);
  });

  server.on("/default", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    if (!request->authenticate(http_username.c_str(), http_password.c_str()))
      return request->requestAuthentication();
    Serial.println();
    Serial.println("__________________RESET DEFAULT_______________________");
    Serial.println();
    delay(2000);
    data_default();
    delay(200);
    ESP.restart();
    request->send(SPIFFS, "/dashboard_ws.html", String(), false, processor);
  });

  server.on("/publish", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    if (!request->authenticate(http_username.c_str(), http_password.c_str()))
      return request->requestAuthentication();
    Serial.println();
    Serial.println("__________________mqtt_test_______________________");
    Serial.println();
    request->send(SPIFFS, "/dashboard_ws.html", String(), false, processor);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request)
  {
    if (!request->authenticate(http_username.c_str(), http_password.c_str()))
      return request->requestAuthentication();
    request->redirect("/");
  });

  server.onNotFound(notFoundd);
  server.begin();
}
