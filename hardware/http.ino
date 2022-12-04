bool tryToConnect() {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  WiFi.begin(config.wifi.ssid, config.wifi.passphrase);
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

void handleWiFiGet() {
  Serial.println("handleWiFiGet");
  server.send(200, "application/json", "{\"ssid\": \"" + config.wifi.ssid + "\", \"passphrase\": \"" + config.wifi.passphrase + "\"}");
}

void handleWifiSet() {
  Serial.println("handleWifiSet");

  if (!server.arg("ssid")) {
    server.send(400);
    return;
  }
  config.wifi.ssid = server.arg("ssid");
  config.wifi.passphrase = server.arg("passphrase");

  Serial.println("Store new credentials");
  Serial.print("ssid:");
  Serial.print(config.wifi.ssid);
  Serial.print("pass:");
  Serial.println(config.wifi.passphrase);
  Serial.println("---");

  saveConfig();
  server.send(200);
}

void handleDisplayGet() {
  Serial.println("handleDisplayGet");
  server.send(200, "application/json", "{\"mode\": " + String(config.display.mode) + ", \"until\": " + String(config.display.until) + ", \"intensity\": " + String(config.display.intensity) + "}");
}

void handleDisplaySet() {
  Serial.println("handleDisplaySet");
  if (server.arg("mode")) {
    config.display.mode = server.arg("mode").toInt();
  }

  if (server.arg("until")) {
    config.display.until = server.arg("until").toInt();
  }

   if (server.arg("intensity")) {
    config.display.intensity = server.arg("intensity").toInt();
  }

  saveConfig();
  server.send(200);
}

void setupServer() {
  server.on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  server.on("/api/wifi/set", HTTP_POST, handleWifiSet);
  server.on("/api/wifi/get", HTTP_GET, handleWiFiGet);
  server.on("/api/display/get", HTTP_GET, handleDisplayGet);
  server.on("/api/display/set", HTTP_POST, handleDisplaySet);
  server.on("/", HTTP_GET, webRoot);
  server.onNotFound(webRoot);
  server.begin();
}

HttpConnectionStatus setupHttp() {
  HttpConnectionStatus output;
  WiFi.mode(WIFI_AP_STA);

  Serial.println("start soft AP");
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.print("ssid:");
  Serial.print(config.wifi.ssid);
  Serial.print("; pass:");
  Serial.println(config.wifi.passphrase);
  if (!config.wifi.ssid.isEmpty()) {
    Serial.println("tryToConnect");
    output.client = tryToConnect();
  }
  Serial.println(output.client ? "connected" : "cant connect");
  output.clientIP = getIpToManage(true);
  output.apIP = getIpToManage(false);

  Serial.print("Ip to manage: ");
  Serial.println(output.client ? output.clientIP : output.apIP);

  Serial.println("Setup ap");
  setupServer();

  return output;
}

