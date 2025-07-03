void setup_webserver() {
  ssid = default_ssid;
  wifipassword = default_wifipassword;
  httpuser = default_httpuser;
  httppassword = default_httppassword;
  webserverporthttp = default_webserverporthttp;
  configureWebServer();
  Serial.println("Starting Webserver ...");
}

void configureWebServer() {
  // if url isn't found
  server.onNotFound(notFound);

  // run handleUpload function when any file is uploaded
  server.onFileUpload(handleUpload);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      //      read_setting();
      request->send(SPIFFS, "/dashboard_ws.html", String(), false, processor);
    }
    else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server.on("/listfiles", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server.on("/file", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);

      if (request->hasParam("name") and request->hasParam("action")) {
        const char *fileName = request->getParam("name")->value().c_str();
        const char *fileAction = request->getParam("action")->value().c_str();

        logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);

        if (!SD.exists(fileName)) {
          Serial.println(logmessage + " ERROR: file does not exist");
          request->send(400, "text/plain", "ERROR: file does not exist");
        }
        else {
          Serial.println(logmessage + " file exists");
          if (strcmp(fileAction, "download") == 0) {
            logmessage += " downloaded";
            request->send(SD, fileName, "application/octet-stream");
          }
          else if (strcmp(fileAction, "delete") == 0) {
            logmessage += " deleted";
            SD.remove(fileName);
            request->send(200, "text/plain", "Deleted File: " + String(fileName));
          }
          else {
            logmessage += " ERROR: invalid action param supplied";
            request->send(400, "text/plain", "ERROR: invalid action param supplied");
          }
          Serial.println(logmessage);
        }
      } else {
        request->send(400, "text/plain", "ERROR: name and action params required");
      }
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });
}

void notFound(AsyncWebServerRequest *request) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);
  request->send(404, "text/plain", "Not found");
}

// used by server.on functions to discern whether a user has the correct httpapitoken OR is authenticated by username and password
bool checkUserWebAuth(AsyncWebServerRequest * request) {
  bool isAuthenticated = false;
  if (request->authenticate(http_username.c_str(), http_password.c_str())) {
    Serial.println("is authenticated via username and password");
    isAuthenticated = true;
  }
  return isAuthenticated;
}

// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  // make sure authenticated before allowing upload
  if (checkUserWebAuth(request)) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index) {
      logmessage = "Upload Start: " + String(filename);
      // open the file on first call and store the file handle in the request object
      request->_tempFile = SD.open("/" + filename, "w");
      Serial.println(logmessage);
    }

    if (len) {
      // stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
      logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
      Serial.println(logmessage);
    }

    if (final) {
      logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
      // close the file handle as the upload is now done
      request->_tempFile.close();
      Serial.println(logmessage);
      request->redirect("/");
    }
  } else {
    Serial.println("Auth: Failed");
    return request->requestAuthentication();
  }
}

String listFiles(bool ishtml) {
  String returnText = "";
  Serial.println("Listing files stored on SD");
  File root = SD.open("/");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
  }
  while (foundfile) {
    if (ishtml) {
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
    }
    else {
      returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}
