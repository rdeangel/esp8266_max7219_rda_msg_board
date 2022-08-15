void wifiModeBuzzer() {
  PRINTS("\nBUZZ");
  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);
}

void startupBuzzer() {
  PRINTS("\nBUZZ");
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(500);
    digitalWrite(BUZZER, LOW);
    delay(100);
  }
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
    delay(100);
  }
}

void mqttConnectBuzzer() {
  PRINTS("\nBUZZ");
  for (int i = 0; i < 10; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(30);
    digitalWrite(BUZZER, LOW);
    delay(30);
  }
}

void mqttDisconnectBuzzer() {
  PRINTS("\nBUZZ");
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(500);
    digitalWrite(BUZZER, LOW);
    delay(500);
  }
}

uint8_t utf8Ascii(uint8_t ascii){
  // Convert a single Character from UTF8 to Extended ASCII according to ISO 8859-1,
  // also called ISO Latin-1. Codes 128-159 contain the Microsoft Windows Latin-1
  // extended characters:
  // - codes 0..127 are identical in ASCII and UTF-8
  // - codes 160..191 in ISO-8859-1 and Windows-1252 are two-byte characters in UTF-8
  //                 + 0xC2 then second byte identical to the extended ASCII code.
  // - codes 192..255 in ISO-8859-1 and Windows-1252 are two-byte characters in UTF-8
  //                 + 0xC3 then second byte differs only in the first two bits to extended ASCII code.
  // - codes 128..159 in Windows-1252 are different, but usually only the €-symbol will be needed from this range.
  //                 + The euro symbol is 0x80 in Windows-1252, 0xa4 in ISO-8859-15, and 0xe2 0x82 0xac in UTF-8.
  //
  // Modified from original code at http://playground.arduino.cc/Main/Utf8ascii
  // Extended ASCII encoding should match the characters at http://www.ascii-code.com/
  //
  // Return "0" if a byte has to be ignored.

  static uint8_t cPrev;
  uint8_t c = '\0';

  //PRINT("\nutf8Ascii 0x", ascii);

  if (ascii < 0x7f) {   // Standard ASCII-set 0..0x7F, no conversion
    cPrev = '\0';
    c = ascii;
  }
  else {
    switch (cPrev) {  // Conversion depending on preceding UTF8-character
    case 0xC2: c = ascii;  break;
    case 0xC3: c = ascii | 0xC0;  break;
    case 0x82: if (ascii==0xAC) c = 0x80; // Euro symbol special case
    case 0xE2: 
      switch (ascii) {
      case 0x80: c = 133;  break;// ellipsis special case
      }
      break;

    //default: PRINTS("!Unhandled! ");
    }
    cPrev = ascii;   // save last char
  }

  //PRINT(" -> 0x", c);
  return(c);
}

void utf8Ascii(char* s) {
  // In place conversion UTF-8 string to Extended ASCII
  // The extended ASCII string is always shorter.
  uint8_t c;
  char *cp = s;

  //PRINT("\nConverting: ", s);

  while (*s != '\0') {
    c = utf8Ascii(*s++);
    if (c != '\0')
      *cp++ = c;
  }
  *cp = '\0';   // terminate the new string
}

//void scrollDataSink(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t col) {
//// Callback function for data that is being scrolled off the display
//#if PRINT_CALLBACK
//  Serial.print("\n cb ");
//  Serial.print(dev);
//  Serial.print(' ');
//  Serial.print(t);
//  Serial.print(' ');
//  Serial.println(col);
//#endif
//}

//Message send based on http URL argument request
void onMessageCallHttp(void) {
  String message = "\nReceived request:\n";
  message += "URI: ";
  message += serverHttp.uri();
  message += "\nMethod: ";
  message += (serverHttp.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += serverHttp.args();
  message += "\n";
  bool messageArg = false;
  for (uint8_t i = 0; i < serverHttp.args(); i++) {
    message += " " + serverHttp.argName(i) + ": " + serverHttp.arg(i) + "\n";
    if (serverHttp.argName(i) == "MSG") {
      repeatCount = 0;
      serverHttp.arg(i).toCharArray(newMessage, MSG_SIZE);
      newMessageAvailable = true;
      messageArg = true;
    }
    if (serverHttp.argName(i) == "REP") {
      serverHttp.arg(i).toCharArray(newRepeat, REP_SIZE);
      newRepeatAvailable = true;
    }
    if (serverHttp.argName(i) == "BUZ") {
      serverHttp.arg(i).toCharArray(newBuz, BUZ_SIZE);
      newBuzAvailable = true;
    }
    if (serverHttp.argName(i) == "DEL") {
      serverHttp.arg(i).toCharArray(newDelay, DEL_SIZE);
      newDelayAvailable = true;
    }
    if (serverHttp.argName(i) == "BRI") {
      serverHttp.arg(i).toCharArray(newBrightness, DEL_SIZE);
      newBrightnessAvailable = true;
    }
    if (serverHttp.argName(i) == "ASC") {
      serverHttp.arg(i).toCharArray(newAsciiConv, ASC_SIZE);
      newAsciiConvAvailable = true;
    }
  }
  if (!messageArg) {
    strcpy(newMessage, "");
    newMessageAvailable = true;
  }
  if (!newRepeatAvailable) {
    strcpy(newRepeat, repeatDefault);
    newRepeatAvailable = true;
  }
  if (!newBuzAvailable) {
    strcpy(newBuz, buzzerDefault);
    newBuzAvailable = true;
  }
  if (!newDelayAvailable) {
    strcpy(newDelay, scrollDelayDefault);
    newDelayAvailable = true;
  }
  if (!newBrightnessAvailable) {
    strcpy(newBrightness, ledBrightnessDefault);
    newBrightnessAvailable = true;
  }
  if (!newAsciiConvAvailable) {
    strcpy(newAsciiConv, asciiConvDefault);
    newAsciiConvAvailable = true;
  }
  Serial.println(message);
  showWebpageHttp();
}

void onMessageCallJson(String jsonMsgData){
  char data[MSG_JSON_SIZE];
  strcpy(data, jsonMsgData.c_str());

  StaticJsonDocument<MSG_JSON_SIZE> doc;
  Serial.println("Json Message Data Received: ");
  Serial.println(data);

  auto error = deserializeJson(doc, data);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  String MSG = doc["MSG"];
  String REP = doc["REP"];
  String BUZ = doc["BUZ"];
  String DEL = doc["DEL"];
  String BRI = doc["BRI"];
  String ASC = doc["ASC"];

  if (doc.containsKey("MSG") == true) {
    MSG.toCharArray(newMessage, MSG_SIZE);
    newMessageAvailable = true;
    repeatCount = 0;
  } else { 
    strcpy(newMessage, "");
    newMessageAvailable = true;
  } 

  if (doc.containsKey("REP") == true) {
    REP.toCharArray(newRepeat, REP_SIZE);
    repeatCount = 0;
    newRepeatAvailable = true;
  } else { 
    strcpy(newRepeat, repeatDefault);
    newRepeatAvailable = true;
  } 

  if (doc.containsKey("BUZ") == true) {
    BUZ.toCharArray(newBuz, BUZ_SIZE);
    newBuzAvailable = true;
  } else {
    strcpy(newBuz, buzzerDefault);
    newBuzAvailable = true;
  }

  if (doc.containsKey("DEL") == true) {
    DEL.toCharArray(newDelay, DEL_SIZE);
    newDelayAvailable = true;
  } else {
    strcpy(newDelay, scrollDelayDefault);
    newDelayAvailable = true;
  }

  if (doc.containsKey("BRI") == true) {
    BRI.toCharArray(newBrightness, DEL_SIZE);
    newBrightnessAvailable = true;
  } else {
    strcpy(newBrightness, ledBrightnessDefault);
    newBrightnessAvailable = true;
  }
  
  if (doc.containsKey("ASC") == true) {
    ASC.toCharArray(newAsciiConv, ASC_SIZE);
    newAsciiConvAvailable = true;
  } else {
    strcpy(newAsciiConv, asciiConvDefault);
    newAsciiConvAvailable = true;
  }

}

void displayText() {
  strcpy(curMessage, newMessage); // copy it in
  M[0].msg = curMessage;
  
  if (atoi(newAsciiConv) == 1) {
    utf8Ascii(M[0].msg);
  }
  
  //M[0].spacing = 1; //Character spacing defaults to 1 but you can change it here
  //P.setScrollSpacing(0);
  PRINTS("\nScrolling Text");
  PRINT("\nCurrent Repeat: ", repeatCount);
  PRINT("\nRequested Repeats: ", newRepeat);
  P.displayText(M[0].msg, PA_CENTER, scrollDelay, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  P.setIntensity(ledBrightness);
  P.setTextBuffer(M[0].msg);
  P.setCharSpacing(M[0].spacing);
  P.displayReset();
}

void scrollTextParola() {
  static char *p;

  if (newDelayAvailable) {
    scrollDelay = atoi(newDelay);
    newDelayAvailable = false;
  }
  if (newBrightnessAvailable) {
    ledBrightness = atoi(newBrightness);
    newBrightnessAvailable = false;
  }
  p = curMessage;      // reset the pointer to start of message
  if (newMessageAvailable) { // there is a new message waiting
    if ((P.displayAnimate())) {
      if (firstMessage == true) {
        displayText();
        firstMessage = false;
      }
      else {
        firstMessageOff = true;
      }
	    //changed this from previous version
      if ((newBuzAvailable) && (newMessageAvailable)) {
        strcpy(curMessage, newMessage);
        if (*p != '\0') {
          for (int i = 1; i <= atoi(newBuz); i++) {
            PRINTS("\nBUZZ");
            digitalWrite(BUZZER, HIGH);
            delay(10);
            digitalWrite(BUZZER, LOW);
            delay(10);
          }
          newBuzAvailable = false;
        }
        else {
          if ((atoi(newRepeat) == 0)  && (firstMessage == false)) {
            repeatCount = 1;
            strcpy(newRepeat, "1");
          }
          else {
            repeatCount = atoi(newRepeat);
            //newRepeatAvailable = false;
          }
        }
      }
      if (atoi(newRepeat) == 0) {
        repeatCount = 0;
        displayText();
      }
      else {
        if (repeatCount < atoi(newRepeat)) {
          repeatCount++;
          displayText();
        }
        else {
          sprintf(curMessage, "");
          repeatCount = atoi(newRepeat);
          newMessageAvailable = false;
        }
      }
    }
  }
  if (newRepeatAvailable) {
    newRepeatAvailable = false;
  }
  if (newAsciiConvAvailable) {
    newAsciiConvAvailable = false;
  }
}

void displaySilentMsg() {
  repeatCount = 0;
  strcpy(newRepeat, "1");
  strcpy(newBuz, "0");
  strcpy(newBrightness, ledBrightnessDefault);
  strcpy(newAsciiConv, asciiConvDefault);
  newRepeatAvailable = true;
  newMessageAvailable = true;
  newBuzAvailable = true;
  newBrightnessAvailable = true;
  newAsciiConvAvailable = true;
  PRINT("\nSilent text to scroll: \n", newMessage);
  while(!firstMessageOff) {
    scrollTextParola();
    yield();
  }
  scrollTextParola();
}

// this will run just before the wifisetup server is started if there is no wifi ssid setup
void configModeCallback (WiFiManager *myWiFiManager) {
  wifiModeBuzzer();
  sprintf(newMessage, "WiFi Setup Mode - IP: %s - SSID: %s - Password: %s - Version: %s", "192.168.4.1", ap_mode_ssid, ap_mode_password, VERSION);
  displaySilentMsg();
  Serial.println("Entered config mode");
  apModeIP = WiFi.softAPIP();
  apModeSSID = myWiFiManager->getConfigPortalSSID();
  Serial.println(apModeIP);
  Serial.println(apModeSSID);
  afterWiFiConfig = true;
}

void rebootDevice() {
  ESP.restart();
}

//clears all configs
void factoryReset() {
  LittleFS.remove(webConfigFile);
  LittleFS.remove(mqttConfigFile);
  delay(500);
  WiFi.disconnect();
  delay(2000);
  ESP.restart();
}

// Flash button callback function to be called when the button is pressed.
void onPressed() {
  Serial.println("\nFlash button has been pressed, Erasing all Config and Resetting.");
  factoryReset();
}

// Called from web factoryreset url
void webFactoryReset() {
  Serial.println("\nWeb Factory Reset has been initiated, Erasing all Config and Resetting.");
  factoryReset();
}