void setup_sd_card() {
  Serial.println("\n[SD] Initializing card.");
  if (!SD.begin(SD_CS)) {
    //    error_sd_card = true;
    Serial.println("[SD] Card failed.");
  }
  else {
    //    error_sd_card = false;
    Serial.println("[SD] Card initializing success.");
    Serial.print("[SD] SD Card Type: ");
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_MMC) {
      Serial.println("MMC");
    }
    else if (cardType == CARD_SD) {
      Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC) {
      Serial.println("SDHC");
    }
    else {
      Serial.println("UNKNOWN");
    }
    uint32_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("[SD] Total penyimpanan pada kartu SD: %u MB\n", cardSize);
    uint32_t totalSpace = SD.totalBytes() / (1024 * 1024);
    Serial.printf("[SD] Total penggunaan penyimpanan pada kartu SD: %u MB\n", totalSpace);
    uint32_t usedStorage = cardSize - totalSpace;
    Serial.printf("[SD] Ukuran penyimpanan yang digunakan: %u MB\n", usedStorage);
  }
}

void loggingLiveLoc() {
  Serial.println("Logging Live Location...");
  String datePart = date_time_str.substring(0, 10);
  String fileName = "/L_" + datePart + "_" + cn + ".csv";
  if (lastFileDate.length() > 0 and lastFileDate != datePart and !SD.exists(fileName.c_str())) {
    Serial.println("Creating new file...");
    lastFileDate = datePart;
    fileName = "/L_" + lastFileDate + "_" + cn + ".csv";
    writeFile(SD, fileName.c_str(), "cn, sn, type, datetime, timezone, lat, lng, alt, spd, hdg, rssi, firm");
    //    saveLastFileDate();
  } else {
    Serial.println("File already exists. No overwrite.");
  }
  File file = SD.open(fileName.c_str());
  if (!file)
    writeFile(SD, fileName.c_str(), "cn, sn, type, datetime, timezone, lat, lng, alt, spd, hdg, rssi, firm");
  file.close();
  String message = cn + "," + sn + "," + type_str + "," + date_time_str + "," + timeZone +
                   "," + String(lat_float, 6) + "," + String(lng_float, 6) + "," + String(alt_float) +
                   "," + String(spd_float) + "," + String(hdg_float) + "," + get_signal_quality() + "," + firm_ver;
  Serial.println("Appending data to file...");
  appendFile(SD, fileName.c_str(), message.c_str());
  Serial.println("Logging completed.\n");
}

String read_file(fs::FS &fs, const char * path) {
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    return String();
  }
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  file.close();
  return fileContent;
}

boolean write_file(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, "w");
  if (!file) {
    file.close();
    return false;
  }
  if (file.println(message)) {
    Serial.print(F("Message writing : "));
    Serial.println(message);
    Serial.println("Message writed.");
    file.close();
    return true;
  }
  else {
    Serial.println("Write failed.");
    file.close();
    return false;
  }
}

boolean append_file(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    file.close();
    return false;
  }
  if (file.println(message)) {
    Serial.print(F("Message appending : "));
    Serial.println(message);
    Serial.println("Message appended");
    file.close();
    return true;
  }
  else {
    Serial.println("Append failed");
    file.close();
    return false;
  }
}
