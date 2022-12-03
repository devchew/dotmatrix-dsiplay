#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

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

WifiConfig wifiConfig;

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

WiFiUDP ntpUDP;

unsigned long until = 1669464000;

NTPClient timeClient(ntpUDP, "0.pl.pool.ntp.org", 3600, 60000);
ESP8266WebServer server(80);

uint8_t frameDelay = 50;
textEffect_t scrollEffect = PA_SCROLL_LEFT;

#define BUF_SIZE 512
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool time_interval = false;

String data;
unsigned int run_seconds = 0;

String format(unsigned long rawTime) {
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  return hoursStr + ":" + minuteStr;
}

String timeCheck() {
  timeClient.update();
  String data = format(until - timeClient.getEpochTime());


  data.toCharArray(newMessage, BUF_SIZE);
  P.displayText(newMessage, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

  return data;
}

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

void setupDisplay() {
  P.begin();
  P.displayClear();
  P.displaySuspend(false);
  P.displayScroll(curMessage, PA_LEFT, PA_SCROLL_LEFT, frameDelay);
  P.setIntensity(10);

  curMessage[0] = newMessage[0] = '\0';
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
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS);
  server.on("/", HTTP_GET, handleHome);
  server.on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  server.on("/api/wifi/set", HTTP_POST, handleWifiSet);
  server.on("/api/wifi/get", HTTP_GET, handleWiFiGetCurrent);
  server.onNotFound(handleHome);
  server.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Setup EEPROM");
  EEPROM.begin(512);
  delay(10);
  Serial.println("Seup display");
  setupDisplay();
  Serial.println("Load config from eeprom");
  wifiConfig = loadConfigFromEEPROM();
  Serial.print("ssid:");
  Serial.print(wifiConfig.ssid);
  Serial.print("; pass:");
  Serial.println(wifiConfig.passphrase);


  Serial.println("Setup wireless things");
  setupServer();

  Serial.println("tryToConnect");
  bool connected = tryToConnect();
  Serial.println(connected ? "connected" : "cant connect");
  IPAddress IP = getIpToManage(connected);

  Serial.print("Ip to manage: ");
  Serial.println(IP);

  Serial.println("Time client begin");
  timeClient.begin();

  sprintf(curMessage, "%s - %03d:%03d:%03d:%03d", (connected ? "Client" : "Host"), IP[0], IP[1], IP[2], IP[3]);
  Serial.print("display set to: ");
  Serial.println(curMessage);
}

void loop() {
  server.handleClient();
  if ((millis() / 1000 - run_seconds) > 30) {
    run_seconds = millis() / 1000;
    time_interval = true;
  }

  if (time_interval) {
    data = timeCheck();
    time_interval = false;
  }

  data.toCharArray(newMessage, BUF_SIZE);

  if (P.displayAnimate()) {
    strcpy(curMessage, newMessage);
    P.displayReset();
  }
}