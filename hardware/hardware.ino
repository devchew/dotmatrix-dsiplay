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

struct HttpConnectionStatus {
  bool client = false;
  IPAddress apIP;
  IPAddress clientIP;
};

WifiConfig wifiConfig;
WiFiUDP ntpUDP;

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

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




void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Setup EEPROM");
  EEPROM.begin(512);
  delay(10);

  Serial.println("Seutp HTTP");
  HttpConnectionStatus status = setupHttp();

  Serial.println("Seup display");
  setupDisplay();  


  Serial.println("Time client begin");
  timeClient.begin();

  IPAddress IP = status.client ? status.clientIP : status.apIP;
  sprintf(curMessage, "%s - %d:%d:%d:%d", (status.client ? "Client" : "Host"), IP[0], IP[1], IP[2], IP[3]);
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