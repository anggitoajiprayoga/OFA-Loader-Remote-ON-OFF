void ota() {
  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    delay(3000);
    return;
  }
  Serial.println(" success");

  if (modem.isNetworkConnected()) {
    Serial.println("Network connected");
  }

  Serial.print(F("Connecting to "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail");
    delay(3000);
    return;
  }
  Serial.println(" success");

  if (modem.isGprsConnected()) {
    Serial.println("GPRS connected");
  }

  HttpClient http(client, server_ota, port_ota);
  Serial.println("Server: " + server_ota);
  Serial.println("resource: " + resource_ota);
  Serial.println("port: " + port_ota);

  Serial.print(F("Performing HTTP GET request... "));
  int err = http.get(resource_ota);
  if (err != 0) {
    Serial.println(F("failed to connect"));
    delay(3000);
    return;
  }

  int status = http.responseStatusCode();
  Serial.print(F("Response status code: "));
  Serial.println(status);
  if (!status) {
    delay(3000);
    return;
  }

  Serial.println(F("Response Headers:"));
  while (http.headerAvailable()) {
    String headerName  = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    Serial.println("    " + headerName + " : " + headerValue);
  }

  int contentLength = http.contentLength();
  if (contentLength <= 0) {
    Serial.println(F("Invalid content length"));
    http.stop();
    modem.gprsDisconnect();
    delay(3000);
    return;
  }

  if (http.isResponseChunked()) {
    Serial.println(F("The response is chunked"));
    http.stop();
    modem.gprsDisconnect();
    delay(3000);
    return;
  }

  // Allocate memory for the firmware update
  if (!Update.begin(contentLength)) {
    Serial.println(F("Could not begin update"));
    http.stop();
    modem.gprsDisconnect();
    delay(3000);
    return;
  }

  // Read the HTTP response and store it in Update
  uint8_t buff[1024];
  size_t bytesWritten = 0;
  while (http.connected() and (bytesWritten < contentLength)) {
    size_t bytesRead = http.readBytes(buff, sizeof(buff));
    if (bytesRead > 0) {
      esp_task_wdt_reset();
      en_update = true;
      Update.write(buff, bytesRead);
      bytesWritten += bytesRead;
      Serial.println("progress update: " + String(bytesWritten) + "/" + String(contentLength) + "   " + String(bytesWritten * 100 / contentLength) + "%");
    }
  }

  // Finish the update process
  if (bytesWritten == contentLength and Update.end()) {
    Serial.println(F("Update complete"));
    if (Update.isFinished()) {
      pub("ofa/device/callback/" + cn, "update OTA done.");
      Serial.println(F("Firmware updated. Restarting..."));
      delay(3000);
      ESP.restart();
    }
  } else {
    en_update = true;
    pub("ofa/device/callback/" + cn, "update OTA failed.");
    Serial.println(F("Update failed"));
    delay(3000);
    Update.abort();
  }

  http.stop();
  modem.gprsDisconnect();
}
