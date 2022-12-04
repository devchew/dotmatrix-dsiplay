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
  if ((millis() / 1000 - run_seconds) > 30) {
    run_seconds = millis() / 1000;
    time_interval = true;
  }

  if (time_interval) {
    displayData = timeCheck();
    Serial.print("Display update: ");
    Serial.println(displayData);
    time_interval = false;
  }

  displayData.toCharArray(newMessage, BUF_SIZE);

  if (P.displayAnimate()) {
    strcpy(curMessage, newMessage);
    P.displayReset();
  }
}