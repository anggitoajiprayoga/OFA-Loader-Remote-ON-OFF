void setupEeprom() {
  String cnx_str = readStringFromEEPROM(addressEepromCN);
  int pos = cnx_str.indexOf('*');
  cn_str = cnx_str.substring(0, pos);
  if (cn_str.indexOf("⸮⸮⸮⸮⸮") > 0 or cn_str.length() == 0 or cn_str.length() > 15) {
    String cnx = "FMS HD";
    writeStringToEEPROM(addressEepromCN, cnx);
    cnx_str = readStringFromEEPROM(addressEepromCN);
    cn_str = cnx_str.substring(0, pos);
  }
  Serial.println("CN: " + cn_str);
}

void writeStringToEEPROM(int addrOffset, const String &strToWrite) {
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++) {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset) {
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];

  for (int i = 0; i < newStrLen; i++) {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '*'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)

  return String(data);
}
