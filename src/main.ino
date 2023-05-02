#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 14   // or SCK
#define DATA_PIN 13  // or MOSI
#define CS_PIN 12    // or SS

// Set AP credentials
#define AP_SSID "Yellow-clock"
#define AP_PASS "123456789"

struct WifiConfig {
  String ssid;
  String passphrase = emptyString;
};


struct DisplayConfig {
  int mode;
  int until;
  int intensity = 8;
  int offset = 0;
};

struct HttpConnectionStatus {
  bool client = false;
  IPAddress apIP;
  IPAddress clientIP;
};

struct Config {
  WifiConfig wifi;
  DisplayConfig display;
};

Config config;

WiFiUDP ntpUDP;

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);


NTPClient timeClient(ntpUDP, "0.pl.pool.ntp.org", 3600, 60000);
ESP8266WebServer server(80);

uint8_t frameDelay = 50;
textEffect_t scrollEffect = PA_SCROLL_LEFT;

#define BUF_SIZE 512
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool time_interval = false;

String displayData;
unsigned int run_seconds = 0;

void mountConfig() {
  // Serial.println("Mounting FS...");

  if (!LittleFS.begin()) {
    // Serial.println("Failed to mount file system");
    return;
  }
  config.display.intensity = 8;
  config.display.mode = 0;
  config.display.until = 1669464000;
  config.display.offset = 0;
  config.wifi.passphrase = "";
  config.wifi.ssid = "";
}

bool loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    // Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    // Serial.println("Config file size is too large");
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
    // Serial.println("Failed to parse config file");
    return false;
  }

  // store values in blobal config file;
  config.display.intensity = doc["display"]["intensity"] | 8;
  config.display.mode = doc["display"]["mode"] | 0;
  config.display.until = doc["display"]["until"] | 1669464000;
  config.display.offset = doc["display"]["offset"] | 0;
  config.wifi.passphrase = doc["wifi"]["passphrase"] | "";
  config.wifi.ssid = doc["wifi"]["ssid"] | "";

  // Serial.println("Config loaded");
  // Serial.println("----");

  // Serial.print("config.display.intensity: ");
  // Serial.println(config.display.intensity);
  // Serial.print("config.display.mode: ");
  // Serial.println(config.display.mode);
  // Serial.print("config.display.until: ");
  // Serial.println(config.display.until);
  // Serial.print("config.display.offset: ");
  // Serial.println(config.display.offset);
  // Serial.print("config.wifi.passphrase: ");
  // Serial.println(config.wifi.passphrase);
  // Serial.print("config.wifi.ssid: ");
  // Serial.println(config.wifi.ssid);

  // Serial.println("----");
  return true;
}

bool saveConfig() {
  StaticJsonDocument<200> doc;

  // copy config to jsondoc
  doc["display"]["intensity"] = config.display.intensity;
  doc["display"]["mode"] = config.display.mode;
  doc["display"]["until"] = config.display.until;
  doc["display"]["offset"] = config.display.offset;
  doc["wifi"]["passphrase"] = config.wifi.passphrase;
  doc["wifi"]["ssid"] = config.wifi.ssid;

  // Serial.println("Config saved");
  // Serial.println("----");

  // Serial.print("config.display.mode: ");
  // Serial.println(config.display.mode);
  // Serial.print("config.display.intensity: ");
  // Serial.println(config.display.intensity);
  // Serial.print("config.display.until: ");
  // Serial.println(config.display.until);
  // Serial.print("config.display.offset: ");
  // Serial.println(config.display.offset);
  // Serial.print("config.wifi.passphrase: ");
  // Serial.println(config.wifi.passphrase);
  // Serial.print("config.wifi.ssid: ");
  // Serial.println(config.wifi.ssid);

  // Serial.println("----");

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    // Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  // Serial.println("Store successfully");
  return true;
}

bool tryToConnect() {
  int c = 0;
  // Serial.println("Waiting for Wifi to connect");
  WiFi.begin(config.wifi.ssid, config.wifi.passphrase);
  while (c < 20) {
    if (WiFi.status() == WL_CONNECTED) { return true; }
    delay(500);
    // Serial.print(WiFi.status());
    c++;
  }
  // Serial.println("");
  // Serial.println("Connect timed out");
  return false;
}

IPAddress getIpToManage(bool isClient) {
  if (isClient) {
    return WiFi.localIP();
  }
  return WiFi.softAPIP();
}

void handleWiFiScan() {
  // Serial.println("handleWiFiScan");
  int n = WiFi.scanNetworks();
  String networksList = "{\"ssids\":[";
  for (int i = 0; i < n; ++i) {
    networksList = networksList + "\"" + WiFi.SSID(i) + "\"" + (i < n - 1 ? "," : "");
  }
  networksList = networksList + "]}";
  server.send(200, "application/json", networksList);
}

void handleWiFiGet() {
  // Serial.println("handleWiFiGet");
  server.send(200, "application/json", "{\"ssid\": \"" + config.wifi.ssid + "\", \"passphrase\": \"" + config.wifi.passphrase + "\"}");
}

void handleWifiSet() {
  // Serial.println("handleWifiSet");

  if (!server.arg("ssid")) {
    server.send(400);
    return;
  }
  config.wifi.ssid = server.arg("ssid");
  config.wifi.passphrase = server.arg("passphrase");

  // Serial.println("Store new credentials");
  // Serial.print("ssid:");
  // Serial.print(config.wifi.ssid);
  // Serial.print("pass:");
  // Serial.println(config.wifi.passphrase);
  // Serial.println("---");

  saveConfig();
  server.send(200);
}

void handleDisplayGet() {
  // Serial.println("handleDisplayGet");
  server.send(200, "application/json", "{\"mode\": " + String(config.display.mode) + ", \"until\": " + String(config.display.until) + ", \"offset\": " + String(config.display.offset) + ", \"intensity\": " + String(config.display.intensity) + "}");
}

void handleDisplaySet() {
  // Serial.println("handleDisplaySet");
  if (server.arg("mode")) {
    config.display.mode = server.arg("mode").toInt();
  }

  if (server.arg("until")) {
    config.display.until = server.arg("until").toInt();
  }

  if (server.arg("intensity")) {
    config.display.intensity = server.arg("intensity").toInt();
  }
   
  if (server.arg("offset")) {
    config.display.offset = server.arg("offset").toInt();
  }

  saveConfig();
  server.send(200);
}

// This function is called when the WebServer was requested without giving a filename.
// This will redirect to the file index.htm when it is existing otherwise to the built-in $upload.htm page
void handleRedirect() {
  server.sendHeader("Location", "/index.html", true);
  server.send(302);
}  // handleRedirect()

void setupServer() {
  server.on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  server.on("/api/wifi/set", HTTP_POST, handleWifiSet);
  server.on("/api/wifi/get", HTTP_GET, handleWiFiGet);
  server.on("/api/display/get", HTTP_GET, handleDisplayGet);
  server.on("/api/display/set", HTTP_POST, handleDisplaySet);

  // register a redirect handler when only domain name is given.
  server.on("/", HTTP_GET, handleRedirect);

  // serve all static files
  server.serveStatic("/", LittleFS, "/");

  server.begin();
}

HttpConnectionStatus setupHttp() {
  HttpConnectionStatus output;
  WiFi.mode(WIFI_AP_STA);

  // Serial.println("start soft AP");
  WiFi.softAP(AP_SSID, AP_PASS);

  // Serial.print("ssid:");
  // Serial.print(config.wifi.ssid);
  // Serial.print("; pass:");
  // Serial.println(config.wifi.passphrase);
  if (!config.wifi.ssid.isEmpty()) {
    // Serial.println("tryToConnect");
    output.client = tryToConnect();
  }
  // Serial.println(output.client ? "connected" : "cant connect");
  output.clientIP = getIpToManage(true);
  output.apIP = getIpToManage(false);

  // Serial.print("Ip to manage: ");
  // Serial.println(output.client ? output.clientIP : output.apIP);

  // Serial.println("Setup ap");
  setupServer();

  return output;
}

void setupDisplay() {
  P.begin();
  P.displayClear();
  P.displaySuspend(false);
  P.displayScroll(curMessage, PA_LEFT, PA_SCROLL_LEFT, frameDelay);
  P.setIntensity(10);

  curMessage[0] = newMessage[0] = '\0';
}
String formatTime(unsigned long rawTime) {
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  return hoursStr + ":" + minuteStr;
}

String timeCheck() {
  timeClient.update();
  String data;
  //Countdown
  if (config.display.mode == 0) {
    data = formatTime(config.display.until - timeClient.getEpochTime());
  }
  //Clock
  if (config.display.mode == 1) {
    data = formatTime(timeClient.getEpochTime());
  }


  data.toCharArray(newMessage, BUF_SIZE);
  P.displayText(newMessage, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

  return data;
}

void displayUpdate() {
  P.setIntensity(config.display.intensity);
  timeClient.setTimeOffset(config.display.offset);
  if ((millis() / 1000 - run_seconds) > 30) {
    run_seconds = millis() / 1000;
    time_interval = true;
  }

  if (time_interval) {
    displayData = timeCheck();
    // Serial.print("Display update: ");
    // Serial.println(displayData);
    time_interval = false;
  }

  displayData.toCharArray(newMessage, BUF_SIZE);

  if (P.displayAnimate()) {
    strcpy(curMessage, newMessage);
    P.displayReset();
  }
}

void setup() {
  // Serial.begin(115200);
  // Serial.println("");
  delay(10);
  mountConfig();
  loadConfig();

  // Serial.println("Seutp HTTP");
  HttpConnectionStatus status = setupHttp();

  // Serial.println("Seup display");
  setupDisplay();


  // Serial.println("Time client begin");
  timeClient.begin();

  IPAddress IP = status.client ? status.clientIP : status.apIP;
  sprintf(curMessage, "%s - %d:%d:%d:%d", (status.client ? "Client" : "Host"), IP[0], IP[1], IP[2], IP[3]);
  // Serial.print("display set to: ");
  // Serial.println(curMessage);

}

void loop() {
  server.handleClient();
  displayUpdate();
}
