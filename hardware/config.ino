void mountConfig() {
  Serial.println("Mounting FS...");

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
}

bool loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  // store values in blobal config file;
  config.display.mode = doc["display"]["mode"] | 0;
  config.display.until = doc["display"]["until"] | 1669464000;
  config.wifi.passphrase = doc["wifi"]["passphrase"] | "";
  config.wifi.ssid = doc["wifi"]["ssid"] | "";
  // strlcpy(config.wifi.passphrase,           // <- destination
  //         doc["wifi"]["passphrase"] | "",   // <- source
  //         sizeof(config.wifi.passphrase));  // <- destination's capacity

  // strlcpy(config.wifi.ssid, doc["wifi"]["ssid"] | "", sizeof(config.wifi.ssid));

  Serial.println("Config loaded");
  return true;
}

bool saveConfig() {
  StaticJsonDocument<200> doc;

  // copy config to jsondoc
  doc["display"]["mode"] = config.display.mode;
  doc["display"]["until"] = config.display.until;
  doc["wifi"]["passphrase"] = config.wifi.passphrase;
  doc["wifi"]["ssid"] = config.wifi.ssid;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  Serial.println("Store successfully");
  return true;
}