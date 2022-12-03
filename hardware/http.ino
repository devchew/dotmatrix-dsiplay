

bool tryToConnect() {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  WiFi.begin(wifiConfig.ssid, wifiConfig.passphrase);
  while (c < 20) {
    if (WiFi.status() == WL_CONNECTED) { return true; }
    delay(500);
    Serial.print(WiFi.status());
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out");
  return false;
}

IPAddress getIpToManage(bool isClient) {
  if (isClient) {
    return WiFi.localIP();
  }
  return WiFi.softAPIP();
}

WifiConfig loadConfigFromEEPROM() {
  WifiConfig newConfig;
  newConfig.ssid = "";
  for (int i = 0; i < 32; ++i) {
    if (char(EEPROM.read(i)) == 0) break;
    newConfig.ssid += char(EEPROM.read(i));
  }
  newConfig.passphrase = "";
  for (int i = 32; i < 96; ++i) {
    if (char(EEPROM.read(i)) == 0) break;
    newConfig.passphrase += char(EEPROM.read(i));
  }
  return newConfig;
}

void storeConfigInEEPROM(WifiConfig config) {
  Serial.println("clearing eeprom");
  for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
  for (int i = 0; i < config.ssid.length(); ++i) {
    EEPROM.write(i, config.ssid[i]);
  }
  for (int i = 0; i < config.passphrase.length(); ++i) {
    EEPROM.write(32 + i, config.passphrase[i]);
  }
  EEPROM.commit();
}
void handleWiFiScan() {
  Serial.println("handleWiFiScan");
  int n = WiFi.scanNetworks();
  String networksList = "{\"ssids\":[";
  for (int i = 0; i < n; ++i) {
    networksList = networksList + "\"" + WiFi.SSID(i) + "\"" + (i < n - 1 ? "," : "");
  }
  networksList = networksList + "]}";
  server.send(200, "application/json", networksList);
}

void handleWiFiGetCurrent() {
  Serial.println("handleWiFiGetCurrent");
  server.send(200, "application/json", "{\"ssid\": \"" + wifiConfig.ssid + "\", \"passphrase\": \"" + wifiConfig.passphrase + "\"}");
}

void handleWifiSet() {
  Serial.println("handleWifiSet");

  if (!server.arg("ssid")) {
    server.send(400);
    return;
  }
  wifiConfig.ssid = server.arg("ssid");
  wifiConfig.passphrase = server.arg("passphrase");

  Serial.println("Store new credentials");
  Serial.print("ssid:");
  Serial.print(wifiConfig.ssid);
  Serial.print("pass:");
  Serial.println(wifiConfig.passphrase);
  Serial.println("---");

  storeConfigInEEPROM(wifiConfig);
  server.send(200);
}

void setupServer() {
  server.on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  server.on("/api/wifi/set", HTTP_POST, handleWifiSet);
  server.on("/api/wifi/get", HTTP_GET, handleWiFiGetCurrent);
  server.on("/", HTTP_GET, webRoot);
  server.onNotFound(webRoot);
  server.begin();
}

HttpConnectionStatus setupHttp() {
  HttpConnectionStatus output;
  WiFi.mode(WIFI_AP_STA);

  Serial.println("start soft AP");
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.println("Load config from eeprom");
  wifiConfig = loadConfigFromEEPROM();
  Serial.print("ssid:");
  Serial.print(wifiConfig.ssid);
  Serial.print("; pass:");
  Serial.println(wifiConfig.passphrase);
  
  Serial.println("tryToConnect");
  output.client = tryToConnect();
  Serial.println(output.client ? "connected" : "cant connect");
  output.clientIP = getIpToManage(true);
  output.apIP = getIpToManage(true);

  Serial.print("Ip to manage: ");
  Serial.println(output.client ? output.clientIP : output.apIP);

  Serial.println("Setup ap");
  setupServer();

  return output;
}

