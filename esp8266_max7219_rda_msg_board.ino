//MAX7219 RDA Message Board - Version 2.3.0
//Tested on "ESP8266 Boards (3.0.2) / NodeMCU 1.0 (ESP12E-Module)"
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>
#include <WiFiManager.h>
#include <EasyButton.h>
#include <stdio.h>
#include <ArduinoJson.h> 
#include "FS.h"
#include <LittleFS.h>

//Define HTTP_SERVER and HTTPS_SERVER to start enable the respective web servers individually
//At least one of the two servers needs to be set to 1 to communicate with the message board
#define HTTP_SERVER 1
#define HTTP_PORT 80
//Set "HTTPS_SERVER 1" below to enable https server. You'll need to generate certificate serverCert and serverKey to upload successfully.
//When https messages are received the text scrolling on the MAX7129 matrix will paul momentarily, this not not occur for http.
#define HTTPS_SERVER 0
#define HTTPS_PORT 443

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
)EOF";

#if HTTP_SERVER
#include <ESP8266WebServer.h>
ESP8266WebServer serverHttp(HTTP_PORT);
#endif

#if HTTPS_SERVER
#include <ESP8266WebServerSecure.h>
BearSSL::ESP8266WebServerSecure serverHttps(HTTPS_PORT);
#endif

#define PRINT_CALLBACK  0
#define DEBUG 0
#define LED_HEARTBEAT 0
#define PAUSE_TIME  0

#if DEBUG
#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }
#define PRINTS(s)   { Serial.print(F(s)); }
#else
#define PRINT(s, v)
#define PRINTS(s)
#endif

#if LED_HEARTBEAT
#define HB_LED  D2
#define HB_LED_TIME 500 // in milliseconds
#endif

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define MAX_DEVICES 4

#define CLK_PIN   D5 // or SCK
#define DATA_PIN  D7 // or MOSI
#define CS_PIN    D8 // or SS


#define HARDWARE_TYPE MD_MAX72XX::FC16_HW  //edit this as per your LED matrix hardware type

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

#define PAUSE_TIME  0

// Define pin where the buzzer is connected
#define BUZZER    D1

#define  DELAYTIME  100  // in milliseconds

//Maximum number of characters username and password can have (individually)
#define  USERPASSSIZE  128

// Arduino pin where the button is connected to.
#define FLASH_BUTTON 0

const char *configfile  = "/config.txt"; // config file

struct Config {
  char usernameHolder[USERPASSSIZE];
  char passwordHolder[USERPASSSIZE];
};

Config config; // config object

uint8_t degC[] = { 5, 3, 3, 56, 68, 68, 68 }; // Deg C
uint8_t degF[] = { 6, 3, 3, 124, 20, 20, 4 }; // Deg F
uint8_t	waveSine[] = { 8, 1, 14, 112, 128, 128, 112, 14, 1 }; // Sine wave
uint8_t waveSqar[] = { 8, 1, 1, 255, 128, 128, 128, 255, 1 }; // Square wave
uint8_t waveTrng[] = { 10, 2, 4, 8, 16, 32, 64, 32, 16, 8, 4 }; // Triangle wave

// Global variables
typedef struct {
  uint8_t spacing;  // character spacing
  char *msg;  // message to display
} msgDef_t;

msgDef_t  M[] = { { 1, "" } };
#define MAX_STRINGS  (sizeof(M)/sizeof(M[0]))

// Instance of the button.
EasyButton flash_button(FLASH_BUTTON);

//WiFi Config Portal WiFi Details
const char* ap_mode_ssid = "RDA-MSG-BOARD";
const char* ap_mode_password = "wifi-setup";

//HTTP and HTTPS Default Authentication to store in config file
char www_username[USERPASSSIZE] = "admin";
char www_password[USERPASSSIZE] = "esp8266";
 
// allows you to set the realm of authentication Default:"Login Required"
const char* www_realm = "Custom Auth Realm";
// the Content of the HTML response in case of Unautherized Access Default:empty
String authFailResponse = "Authentication Failed";

// Global message buffers shared by Wifi and Scrolling functions
const uint8_t MESG_SIZE = 255;
const uint8_t CHAR_SPACING = 1;

char newUsername[USERPASSSIZE];
char newPassword[USERPASSSIZE];
bool newUsernameAvailable = false;
bool newPasswordAvailable = false;
int scrollDelay = 40;
int repeats = 0;
char curMessage[MESG_SIZE];
char newMessage[MESG_SIZE];
char newRepeat[MESG_SIZE];
char newBuz[MESG_SIZE];
char newDelay[MESG_SIZE];
char newAsciiconv[MESG_SIZE];
bool firstMessage = true;
bool firstMessageOff = false;
bool newMessageAvailable = false;
bool newRepeatAvailable = false;
bool newBuzAvailable = false;
bool newDelayAvailable = false;
bool newAsciiconvAvailable = false;
bool endSetupModeMsg = false;
bool afterWiFiConfig = false;
char assignedIP[MESG_SIZE];
IPAddress apModeIP;
String apModeSSID;

const char MAIN_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head><meta name="viewport" content="width=device-width, initial-scale=1">
<title>MAX7219 RDA Message Board</title>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
#message_container {
  color: black;
  width: 90%;
  height: 100%;
  margin-left: auto;
  margin-right: auto;
  margin-top: 20px;
  border: solid 2px;
  padding: 10px;
  background-color: #9f9fa6;
}
#firmware_container {
  color: black;
  width: 200px;
  height: 100%;
  margin-left: auto;
  margin-right: auto;
  margin-top: auto;
  border: solid 2px;
  padding-top: 10px;
  padding-bottom: 10px;
  background-color: #914f41;
}        
</style>
<script>
delay = "";
function SendText() {
  var request = new XMLHttpRequest();
  msg = "&MSG=" + document.getElementById("txt_form").Message.value;
  repeat = "&REP=" + document.getElementById("txt_form").Repeat.value;
  buzzer = "&BUZ=" + document.getElementById("txt_form").Buzzer.value;
  delay = "&DEL=" + document.getElementById("txt_form").Delay.value;
  asciiconv = "&ASC=0";
  request.open("GET", "arg?" + msg + repeat + buzzer + delay + asciiconv, false);
  request.send(null);
}
</script>
</head>
<body>
<H1><b>MAX7219 RDA Message Board</b></H1> 
<div id="message_container">
<form id="txt_form" name="frmText">
<label>Message:<input type="text" name="Message" maxlength="255"></label><br><br>
<label>Repeat:<input type="text" name="Repeat" maxlength="3" size="3" value="1"></label><br><br>
<label>Buzzer:<input type="text" name="Buzzer" maxlength="3" size="3" value="1"></label><br><br>
<label>Delay:<input type="text" name="Delay" maxlength="3" size="3" value="40"></label><br>
</form>
<br>
<input type="submit" value="Send Message" onclick="SendText()">
</div>
<br>
<form method="get" action="/changeuserpass">
<button type="submit">Change Credentials</button>
</form>
<br>
<div id="firmware_container">
<form method="get" action="/update">
<button type="submit">Update Firmware</button>
</form>
</div>
</body>
</html>
)=====";


const char UPDATE_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head><meta name="viewport" content="width=device-width, initial-scale=1">
<title>MAX7219 RDA Firmware Update</title>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
#firmware_container {
  width: 90%;
  height: 100%;
  margin-left: auto;
  margin-right: auto;
  margin-top: 20px;
  border: solid 2px;
  padding: 10px;
  background-color: #914f41;
}          
</style>
</head>
<body>
<H1><b>MAX7219 RDA Firmware Update</b></H1>
</head>
<div id="firmware_container">
<form method='POST' action='/submitupdate' enctype='multipart/form-data'>
<input type='file' name='update'>
<br><br>
<input type='submit' value='Update'>
</form>
</div>
</body>
</html>
)=====";


const char CHANGECREDENTIALS_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head><meta name="viewport" content="width=device-width, initial-scale=1">
<title>MAX7219 RDA User and Password Change</title>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
#userpassword_container {
  color: black;
  width: 90%;
  height: 100%;
  margin-left: auto;
  margin-right: auto;
  margin-top: 20px;
  border: solid 2px;
  padding: 10px;
  background-color: #9f9fa6;
}
</style>
</head>
<body>
<H1><b>MAX7219 RDA - Change Credentials</b></H1> 
<div id="userpassword_container"> 
<form method="post" action="changecredentials">
<label for="Username">Username: </label>
<input type="text" placeholder="Enter Username" name="Username" required><br><br>
<label for="Password">Password: </label>
<input type="password" placeholder="Enter Password" name="Password" required><br><br>
<input type="submit" value="Save Changes">
</form> 
</div>
</body>
</html>
)=====";

uint8_t htoi(char c) {
  c = toupper(c);
  if ((c >= '0') && (c <= '9')) return(c - '0');
  if ((c >= 'A') && (c <= 'F')) return(c - 'A' + 0xa);
  return(0);
}

uint8_t utf8Ascii(uint8_t ascii)
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
{
  static uint8_t cPrev;
  uint8_t c = '\0';

  //PRINT("\nutf8Ascii 0x", ascii);

  if (ascii < 0x7f)   // Standard ASCII-set 0..0x7F, no conversion
  {
    cPrev = '\0';
    c = ascii;
  }
  else
  {
    switch (cPrev)  // Conversion depending on preceding UTF8-character
    {
    case 0xC2: c = ascii;  break;
    case 0xC3: c = ascii | 0xC0;  break;
    case 0x82: if (ascii==0xAC) c = 0x80; // Euro symbol special case
    case 0xE2: 
      switch (ascii)
      {
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

void utf8Ascii(char* s)
// In place conversion UTF-8 string to Extended ASCII
// The extended ASCII string is always shorter.
{
  uint8_t c;
  char *cp = s;

  //PRINT("\nConverting: ", s);

  while (*s != '\0')
  {
    c = utf8Ascii(*s++);
    if (c != '\0')
      *cp++ = c;
  }
  *cp = '\0';   // terminate the new string
}

void scrollDataSink(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t col)
// Callback function for data that is being scrolled off the display
{
#if PRINT_CALLBACK
  Serial.print("\n cb ");
  Serial.print(dev);
  Serial.print(' ');
  Serial.print(t);
  Serial.print(' ');
  Serial.println(col);
#endif
}

void displayText() {
  
  strcpy(curMessage, newMessage); // copy it in
  M[0].msg = curMessage;
  
  if (atoi(newAsciiconv) == 1) {
    utf8Ascii(M[0].msg);
  }
  
  //M[0].spacing = 1; //Character spacing defaults to 1 but you can change it here
  //P.setScrollSpacing(0);
  PRINTS("\nScrolling Text");
  PRINT("\nCurrent Repeat: ", repeats);
  PRINT("\nRequested Repeats: ", newRepeat);
  P.displayText(M[0].msg, PA_CENTER, scrollDelay, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  P.setTextBuffer(M[0].msg);
  P.setCharSpacing(M[0].spacing);
  P.displayReset();
}

void scrollTextParola() {	
static char   *p;

  if (newDelayAvailable) {
    scrollDelay = atoi(newDelay);
  }
  p = curMessage;      // reset the pointer to start of message
  if (newMessageAvailable) { // there is a new message waiting
    if ((P.displayAnimate())) {
      if (firstMessage == true) {
        displayText();
        firstMessage = false;
      }
      else{
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
            newBuzAvailable = false;
          }
        }
        else{
          if (atoi(newRepeat) == 0) {
            repeats = 1;
            strcpy(newRepeat, "1");
          }
          else {
            repeats = atoi(newRepeat);
          }
        }
      }
      if ((atoi(newRepeat) == 0) && (firstMessage == false)) {
        repeats = 0;
        displayText();
      }
      else {
        if (repeats < atoi(newRepeat)) {
          repeats++;
          displayText();
        }
        else {
          sprintf(curMessage, "");
          repeats = atoi(newRepeat);
          newMessageAvailable = false;
        }
      }
    }
  }
}

//this will run just before the wifisetup server is started if there is no wifi ssid setup
void configModeCallback (WiFiManager *myWiFiManager) {
  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);
  Serial.println("Entered config mode");
  apModeIP = WiFi.softAPIP();
  apModeSSID = myWiFiManager->getConfigPortalSSID();
  Serial.println(apModeIP);
  Serial.println(apModeSSID);
  strcpy(newAsciiconv, "1");
  sprintf(newMessage, "WiFi Setup Mode - IP: %s - SSID: %s - Password: %s", "192.168.004.001", ap_mode_ssid, ap_mode_password);
  newMessageAvailable = true;
  strcpy(newRepeat, "1");
  int x = 0;
  while(!firstMessageOff) {
    scrollTextParola();
    yield();
  }
  afterWiFiConfig = true;
}

// Flash button callback function to be called when the button is pressed.
void onPressed() {
  Serial.println("\nFlash button has been pressed, Erasing Credential and Resetting.");
  
  LittleFS.remove(configfile);
  delay(500);
  WiFi.disconnect();
  delay(2000);
  ESP.restart();
}

// Loads the configuration from a file
void loadConfiguration(const char *configfile, Config &config) {

  File file = LittleFS.open(configfile, "r");
  if (!file) {
    Serial.println("Failed to open data file");
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  strlcpy(config.usernameHolder,                  // <- destination
          doc["usernameHolder"],                  // <- source
          sizeof(config.usernameHolder));         // <- destination's capacity
  strlcpy(config.passwordHolder,                  // <- destination
          doc["passwordHolder"],                  // <- source
          sizeof(config.passwordHolder));         // <- destination's capacity

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}

// Saves the configuration to a file
void saveConfiguration(const char *configfile, const Config &config) {
  // Delete existing file, otherwise the configuration is appended to the file
  //LittleFS.remove(configfile);

  // Open file for writing
  File file = LittleFS.open(configfile, "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return;
  }
  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<512> doc;
  
  // Set the values in the document
  doc["usernameHolder"] = config.usernameHolder;
  doc["passwordHolder"] = config.passwordHolder;
  
  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  
  // Close the file
  file.close();
}

// Prints the content of a file to the Serial
void printFile(const char *configfile) {
  // Open file for reading

  File file = LittleFS.open(configfile, "r");
  if (!file) {
    Serial.println("Failed to open data file");
    return;
  }
  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}

//change login credentials and store into config file
void changeLoginCredentials() {
  //set username and password from webpage to config object
  strlcpy(config.usernameHolder, newUsername, sizeof(config.usernameHolder));
  strlcpy(config.passwordHolder, newPassword, sizeof(config.passwordHolder));
  //save username and password from config object to config file
  saveConfiguration(configfile, config);
  //set the http/https credentials to the new password
  strlcpy(www_username, config.usernameHolder, sizeof(www_username));
  strlcpy(www_password, config.passwordHolder, sizeof(www_password));
  // Dump config file
  PRINTS("Username and Password changed\nPrinting config file:\n");
  printFile(configfile);
}

//################################ START OF SPECIFIC HTTP SERVER FUNCTIONS ################################//
#if HTTP_SERVER
void showWebpageHttp() {
  String s = MAIN_page; //Read HTML contents
  serverHttp.send(200, "text/html", s); //Send web page
}

void showChangeCredentialsHttp() {
  String s = CHANGECREDENTIALS_page; //Read HTML contents
  serverHttp.send(200, "text/html", s); //Send web page
}

void usernamePasswordHttp() {
  String message = "\nReceived request:\n";
  message += "URI: ";
  message += serverHttp.uri();
  message += "\nMethod: ";
  message += (serverHttp.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += serverHttp.args();
  message += "\n";
  for (uint8_t i = 0; i < serverHttp.args(); i++) {
    message += " " + serverHttp.argName(i) + ": " + serverHttp.arg(i) + "\n";
    if (serverHttp.argName(i) == "Username") {
      serverHttp.arg(i).toCharArray(newUsername, USERPASSSIZE);
      newUsernameAvailable = true;
    }
    if (serverHttp.argName(i) == "Password") {
      serverHttp.arg(i).toCharArray(newPassword, USERPASSSIZE);
      newPasswordAvailable = true;
    }
  }
  Serial.println(message);
}

void onMessageCallHttp(void) {
  String message = "\nReceived request:\n";
  message += "URI: ";
  message += serverHttp.uri();
  message += "\nMethod: ";
  message += (serverHttp.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += serverHttp.args();
  message += "\n";
  for (uint8_t i = 0; i < serverHttp.args(); i++) {
    message += " " + serverHttp.argName(i) + ": " + serverHttp.arg(i) + "\n";
    if (serverHttp.argName(i) == "MSG") {
      serverHttp.arg(i).toCharArray(newMessage, MESG_SIZE);
      newMessageAvailable = true;
    }
    if (serverHttp.argName(i) == "REP") {
      serverHttp.arg(i).toCharArray(newRepeat, MESG_SIZE);
      repeats = 0;
      newRepeatAvailable = true;
    }
    if (serverHttp.argName(i) == "BUZ") {
      serverHttp.arg(i).toCharArray(newBuz, MESG_SIZE);
      newBuzAvailable = true;
    }
    if (serverHttp.argName(i) == "DEL") {
      serverHttp.arg(i).toCharArray(newDelay, MESG_SIZE);
      newDelayAvailable = true;
    }
    if (serverHttp.argName(i) == "ASC") {
      serverHttp.arg(i).toCharArray(newAsciiconv, MESG_SIZE);
      newAsciiconvAvailable = true;
    }
  }
  Serial.println(message);
  showWebpageHttp();
}

void serverHttp_post_api() {
  String data = serverHttp.arg("plain");
  StaticJsonDocument<300> doc;
  Serial.println("");
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
  String ASC = doc["ASC"];
  
  MSG.toCharArray(newMessage, MESG_SIZE);
  newMessageAvailable = true;
  
  REP.toCharArray(newRepeat, MESG_SIZE);
  repeats = 0;
  newRepeatAvailable = true;
  
  BUZ.toCharArray(newBuz, MESG_SIZE);
  newBuzAvailable = true;
  
  DEL.toCharArray(newDelay, MESG_SIZE);
  newDelayAvailable = true;
  
  ASC.toCharArray(newAsciiconv, MESG_SIZE);
  newAsciiconvAvailable = true;
  
  serverHttp.send(204,"");
}

void onNotFoundUriHttp() {
  serverHttp.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
#endif
//################################ END OF SPECIFIC HTTPS SERVER FUNCTIONS ################################//

//################################ START OF SPECIFIC HTTPS SERVER FUNCTIONS ################################//
#if HTTPS_SERVER
void showWebpageHttps() {
  String s = MAIN_page; //Read HTML contents
  serverHttps.send(200, "text/html", s); //Send web page
}

void showChangeCredentialsHttps() {
  String s = CHANGECREDENTIALS_page; //Read HTML contents
  serverHttps.send(200, "text/html", s); //Send web page
}

void usernamePasswordHttps() {
  String message = "\nReceived request:\n";
  message += "URI: ";
  message += serverHttps.uri();
  message += "\nMethod: ";
  message += (serverHttps.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += serverHttps.args();
  message += "\n";
  for (uint8_t i = 0; i < serverHttps.args(); i++) {
    message += " " + serverHttps.argName(i) + ": " + serverHttps.arg(i) + "\n";
    if (serverHttps.argName(i) == "Username") {
      serverHttps.arg(i).toCharArray(newUsername, USERPASSSIZE);
      newUsernameAvailable = true;
    }
    if (serverHttps.argName(i) == "Password") {
      serverHttps.arg(i).toCharArray(newPassword, USERPASSSIZE);
      newPasswordAvailable = true;
    }
  }
  Serial.println(message);
}

void onMessageCallHttps(void) {
  String message = "\nReceived request:\n";
  message += "URI: ";
  message += serverHttps.uri();
  message += "\nMethod: ";
  message += (serverHttps.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += serverHttps.args();
  message += "\n";
  for (uint8_t i = 0; i < serverHttps.args(); i++) {
    message += " " + serverHttps.argName(i) + ": " + serverHttps.arg(i) + "\n";
    if (serverHttps.argName(i) == "MSG") {
      serverHttps.arg(i).toCharArray(newMessage, MESG_SIZE);
      newMessageAvailable = true;
    }
    if (serverHttps.argName(i) == "REP") {
      serverHttps.arg(i).toCharArray(newRepeat, MESG_SIZE);
      repeats = 0;
      newRepeatAvailable = true;
    }
    if (serverHttps.argName(i) == "BUZ") {
      serverHttps.arg(i).toCharArray(newBuz, MESG_SIZE);
      newBuzAvailable = true;
    }
    if (serverHttps.argName(i) == "DEL") {
      serverHttps.arg(i).toCharArray(newDelay, MESG_SIZE);
      newDelayAvailable = true;
    }
    if (serverHttps.argName(i) == "ASC") {
      serverHttps.arg(i).toCharArray(newAsciiconv, MESG_SIZE);
      newAsciiconvAvailable = true;
    }
  }
  Serial.println(message);
  showWebpageHttps();
} 

void serverHttps_post_api() {
  String data = serverHttps.arg("plain");
  StaticJsonDocument<300> doc;
  Serial.println("");
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
  String ASC = doc["ASC"];
  
  MSG.toCharArray(newMessage, MESG_SIZE);
  newMessageAvailable = true;
  
  REP.toCharArray(newRepeat, MESG_SIZE);
  repeats = 0;
  newRepeatAvailable = true;
  
  BUZ.toCharArray(newBuz, MESG_SIZE);
  newBuzAvailable = true;
  
  DEL.toCharArray(newDelay, MESG_SIZE);
  newDelayAvailable = true;
  
  ASC.toCharArray(newAsciiconv, MESG_SIZE);
  newAsciiconvAvailable = true;
  
  serverHttps.send(204,"");
}

void onNotFoundUriHttps(){
  serverHttps.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
#endif
//################################ END OF SPECIFIC HTTPS SERVER FUNCTIONS ################################//

void setup() {
#if DEBUG
  Serial.begin(115200);
  PRINTS("\n[MD_MAX72XX WiFi Message Display]\nType a message for the scrolling display from your internet browser");
#endif
  Serial.println("");
  
  flash_button.begin();
  // Add the callback function to be called when the button is pressed.
  flash_button.onPressed(onPressed);
 
  // Initialize LittleFS library
  while (!LittleFS.begin()) {
    Serial.println(F("Failed to initialize LittleFS library"));
    delay(1000);
  }
  
  //load config stored in config file
  Serial.println(F("Loading configuration..."));
  loadConfiguration(configfile, config);
  //if no username is defined in config file store default
  if ((config.usernameHolder != NULL) && (config.usernameHolder[0] == '\0')) {
    PRINT("no username set, setting default username: ", www_username);
    strlcpy(config.usernameHolder, www_username, sizeof(config.usernameHolder));
  }
  //if no password is defined in config file store default
  if ((config.passwordHolder != NULL) && (config.passwordHolder[0] == '\0')) {
    PRINT("no password set, setting default password: ", www_password);
    strlcpy(config.passwordHolder, www_password, sizeof(config.passwordHolder));
  }
  //set http/https server to config file defined values or defined default
  strlcpy(www_username, config.usernameHolder, sizeof(www_username));
  strlcpy(www_password, config.passwordHolder, sizeof(www_password));
  
  // Create configuration file
  Serial.println(F("Saving configuration..."));
  saveConfiguration(configfile, config);

  // Dump config file
  Serial.println(F("Print config file..."));
  printFile(configfile);
  
#if LED_HEARTBEAT
  pinMode(HB_LED, OUTPUT);
  digitalWrite(HB_LED, LOW);
#endif

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  
  // Display initialisation
  P.begin();

  //P.addChar('¬', degC);
  //P.addChar('&', degF);
  //P.addChar('~', waveSine);
  //P.addChar('+', waveSqar);
  //P.addChar('^', waveTrng);

  curMessage[0] = newMessage[0] = '\0';

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  
  WiFiManager wm; //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around

  //Comment out and edit line below if you want to change AP Config Portal IP from default 192.168.4.1 to something else
  //wm.setAPStaticIPConfig(IPAddress(192,168,100,1), IPAddress(192,168,100,1), IPAddress(255,255,255,0));

  //run code before wifi setup is checked/started
  wm.setAPCallback(configModeCallback);

  //try to connect for 60 seconds before starting config portal
  wm.setConnectTimeout(60);
  
  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect(ap_mode_ssid,ap_mode_password); // Start AP either in setup mode or connects to configured wifi network
  if(!res) {
    Serial.println("Failed to connect");
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
    firstMessage = true;
    if(afterWiFiConfig){
      ESP.restart();
    }
  }

  //show ip address on serial
  sprintf(assignedIP, "%03d.%03d.%03d.%03d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  PRINT("\nAssigned IP: ", assignedIP);
  PRINTS("\n\n");

//################################ START OF HTTP SERVER ################################//
#if HTTP_SERVER
  serverHttp.on("/", []() {
    if (!serverHttp.authenticate(www_username, www_password)) {
      return serverHttp.requestAuthentication();
    }
    showWebpageHttp();
  });
  serverHttp.on("/arg", []() {
    if (!serverHttp.authenticate(www_username, www_password)) {
      return serverHttp.requestAuthentication();
    }
    onMessageCallHttp();
  });
  serverHttp.on("/api", HTTP_POST, [](){
    if (!serverHttp.authenticate(www_username, www_password)) {
      return serverHttp.requestAuthentication();
    }
    serverHttp_post_api();
  });
  serverHttp.on("/changeuserpass", [](){
    if (!serverHttp.authenticate(www_username, www_password)) {
      return serverHttp.requestAuthentication();
    }
    showChangeCredentialsHttp();
  });
  serverHttp.on("/changecredentials", [](){
    if (!serverHttp.authenticate(www_username, www_password)) {
      return serverHttp.requestAuthentication();
    }
    usernamePasswordHttp();
	changeLoginCredentials();
    serverHttp.sendHeader("Connection", "close");
    serverHttp.send(200, "text/plain", "Username and Password updated!");
  });
  serverHttp.on("/update", HTTP_GET, []() {
    if (!serverHttp.authenticate(www_username, www_password)) {
      return serverHttp.requestAuthentication();
    }
    serverHttp.sendHeader("Connection", "close");
    serverHttp.send(200, "text/html", UPDATE_page);
  });
  serverHttp.on("/submitupdate", HTTP_POST, []() {
    if (!serverHttp.authenticate(www_username, www_password)) {
      return serverHttp.requestAuthentication();
    }
    serverHttp.sendHeader("Connection", "close");
    serverHttp.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  }, []() {
    if (!serverHttp.authenticate(www_username, www_password)) {
      return serverHttp.requestAuthentication();
    }
    HTTPUpload& upload = serverHttp.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } 
    else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } 
    else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        serverHttp.send(200, "text/plain", "Update Completed Successfully \nRebooting now...\n");
        delay(1000);
        ESP.restart();
      } 
      else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });
  serverHttp.onNotFound([]() {
    if (!serverHttp.authenticate(www_username, www_password)) {
      return serverHttp.requestAuthentication();
    }
    onNotFoundUriHttp();
  });
  
  // Start the http server
  serverHttp.begin();
  PRINTS("HTTP Server started on port 80\n");
  Serial.printf("You can update firmware from the browser opening! -> Open http://%s/update in your browser\n\n", assignedIP);
#endif
//################################ START OF HTTP SERVER ################################//


//################################ START OF HTTPS (SSL) SERVER ################################//
#if HTTPS_SERVER
  serverHttps.on("/", []() {
    if (!serverHttps.authenticate(www_username, www_password)) {
      return serverHttps.requestAuthentication();
    }
    showWebpageHttps();
  });
  serverHttps.on("/arg", []() {
    if (!serverHttps.authenticate(www_username, www_password)) {
      return serverHttps.requestAuthentication();
    }
    onMessageCallHttps();
  });
  serverHttps.on("/api", HTTP_POST, []() {
    if (!serverHttps.authenticate(www_username, www_password)) {
      return serverHttps.requestAuthentication();
    }
    serverHttps_post_api();
  });
  serverHttps.on("/changeuserpass", []() {
    if (!serverHttps.authenticate(www_username, www_password)) {
      return serverHttps.requestAuthentication();
    }
    showChangeCredentialsHttps();
  });
  serverHttps.on("/changecredentials", [](){
    if (!serverHttps.authenticate(www_username, www_password)) {
      return serverHttps.requestAuthentication();
    }
    usernamePasswordHttps();
	changeLoginCredentials();
    serverHttps.sendHeader("Connection", "close");
    serverHttps.send(200, "text/plain", "Username and Password updated!");
  });
  serverHttps.on("/update", HTTP_GET, []() {
    if (!serverHttps.authenticate(www_username, www_password)) {
      return serverHttps.requestAuthentication();
    }
    serverHttps.sendHeader("Connection", "close");
    serverHttps.send(200, "text/html", UPDATE_page);
  });
  serverHttps.on("/submitupdate", HTTP_POST, []() {
    if (!serverHttps.authenticate(www_username, www_password)) {
      return serverHttps.requestAuthentication();
    }
    serverHttps.sendHeader("Connection", "close");
    serverHttps.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  }, []() {
    if (!serverHttps.authenticate(www_username, www_password)) {
      return serverHttps.requestAuthentication();
    }
    HTTPUpload& upload = serverHttps.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    }
    else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        serverHttps.send(200, "text/plain", "Update Completed Successfully \nRebooting now...\n");
        ESP.restart();
        delay(1000);
      }
      else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });
  serverHttps.onNotFound([]() {
    if (!serverHttps.authenticate(www_username, www_password)) {
      return serverHttps.requestAuthentication();
    }
    onNotFoundUriHttps();
  });
  
  // Start the server
  serverHttps.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
  serverHttps.begin();
  PRINTS("HTTPS Server started on port 443\n");
  Serial.printf("You can update firmware from the browser opening! -> Open https://%s/update in your browser\n\n", assignedIP);
#endif
//################################ END OF HTTPS (SSL) SERVER ################################//

  //Scroll first message in message mode
  strcpy(newAsciiconv, "1");
  sprintf(newMessage, "Wifi Message Mode - IP: %s - http/s Username: %s - Password: %s", assignedIP, www_username, www_password);

  PRINTS("\nBUZZ");
  digitalWrite(BUZZER, HIGH);
  delay(500);
  digitalWrite(BUZZER, LOW);
  delay(100);
  digitalWrite(BUZZER, HIGH);
  delay(500);
  digitalWrite(BUZZER, LOW);
  delay(100);
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
  delay(100);
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
  delay(100);
  strcpy(newRepeat, "1");
  newMessageAvailable = true;
  scrollTextParola();
}


void loop()
{
#if LED_HEARTBEAT
  static uint32_t timeLast = 0;
  
  if (millis() - timeLast >= HB_LED_TIME) {
    digitalWrite(HB_LED, digitalRead(HB_LED) == LOW ? HIGH : LOW);
    timeLast = millis();
  }
#endif

#if HTTP_SERVER
  serverHttp.handleClient();
#endif

#if HTTPS_SERVER
  serverHttps.handleClient();
#endif

  flash_button.read();

  scrollTextParola();  
}