#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 14   // or SCK
#define DATA_PIN 13  // or MOSI
#define CS_PIN 12    // or SS

// Set AP credentials
#define AP_SSID "Yellow-clock"
#define AP_PASS "123456789"

struct WifiConfig {
  const char * ssid;
  const char * passphrase = 0;
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
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    if (WiFi.SSID(i) == wifiConfig.ssid) {
      WiFi.begin(wifiConfig.ssid, wifiConfig.passphrase);
      return true;
      break;
    }
  }
  return false;
}

IPAddress getIpToManage(bool isClient) {
  if (isClient) {
    return WiFi.localIP();
  }
  return WiFi.softAPIP();
}

void setupDisplay() {
  P.begin();
  P.displayClear();
  P.displaySuspend(false);
  P.displayScroll(curMessage, PA_LEFT, PA_SCROLL_LEFT, frameDelay);
  P.setIntensity(10);

  curMessage[0] = newMessage[0] = '\0';
}

void handleWiFiScan(){
  int n = WiFi.scanNetworks();
  String networksList = "{\"ssids\":[";
  for (int i = 0; i < n; ++i) {
    networksList = networksList + "\"" + WiFi.SSID(i) + "\"" + (i < n - 1 ? "," : "");
  }
  networksList = networksList + "]}";
  server.send(200, "application/json", networksList);
}

void handleWiFiGetCurrent(){
  server.send(200, "application/json", "{\"ssid\": \"" + String(wifiConfig.ssid) + "\", \"passphrase\": \"" +  String(wifiConfig.passphrase) + "\"}");
}

void handleWifiSet(){
  if (!server.arg("ssid")) {
    server.send(400);
    return; 
  }
  wifiConfig.ssid = server.arg("ssid").c_str();
  wifiConfig.passphrase = server.arg("passphrase").c_str();
  server.send(200);  
}

void setupServer() {
  SPIFFS.begin();
  WiFi.softAP(AP_SSID, AP_PASS);
  server.on("/", HTTP_GET, handleHome);
  server.on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  server.on("/api/wifi/set", HTTP_POST, handleWifiSet);
  server.on("/api/wifi/get", HTTP_GET, handleWiFiGetCurrent);
  server.onNotFound(handleHome);
  server.begin();
}

void setup() {
  setupDisplay();
  
  WiFi.mode(WIFI_AP_STA);
  setupServer();

  bool isClient = tryToConnect();
  IPAddress IP = getIpToManage(isClient);

  if (isClient) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
  }

  timeClient.begin();

  sprintf(curMessage, "%s - %03d:%03d:%03d:%03d", (isClient ? "Client" : "Host"), IP[0], IP[1], IP[2], IP[3]);
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