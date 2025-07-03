void setup_wifi() {
  String APssid2 = cn;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(APssid2.c_str(), "");
  server.begin();
  server_begin();
}

void loop_wifi() {
  static unsigned long lastTime;
  static bool state, simbol;
  if ((millis() - lastTime) > 500) {
    String get_payload = payload();
    notifyClients(get_payload);
    //    Serial.println("//web_socket_payload_send");
    //    Serial.println(get_payload);
    if (state == simbol) {
      state = !simbol;
    }
    lastTime = millis();
  }

  simbol = state;
  if (state == true) {
    ind = " | ";
  }
  else {
    ind = " - ";
  }
  ws.cleanupClients();
}
