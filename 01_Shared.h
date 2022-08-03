// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define MAX_DEVICES 4
#define CLK_PIN D5 // or SCK
#define DATA_PIN D7 // or MOSI
// CS_PIN D8 for NodeMCU 1.0 (ESP12E-Module)
#define CS_PIN D8

// Edit LED matrix hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Define pin where the buzzer is connected
#define BUZZER D1

//WifiManager use mdns
#define WM_MDNS 1
#define WM_DEBUG_LEVEL 0

//DEBUG ON OR OFF
#define DEBUG 0
#define PRINT_CALLBACK  0
#define PAUSE_TIME  0

//Flash button enabled/disaled
#define ENABLE_FLASH_BUTTON 0

#if ENABLE_FLASH_BUTTON
  // Built button used to wipe device config
  #define FLASH_BUTTON 0
  // Instance of the button
  EasyButton flash_button(FLASH_BUTTON);
#endif


// You must enable either "HTTP_SERVER" or "HTTPS_SERVER" to "1", or both.
// Set "HTTPS_SERVER 1" below to enable http server.
#define HTTP_PORT 80
#include <ESP8266WebServer.h>
ESP8266WebServer serverHttp(HTTP_PORT);

// create clientId / hostname
String clientIdPrefix = "ESP-MSG-";
String chipId = String(ESP.getChipId(), HEX);
//String clientIdTrail = "";

// char lenght of (individually)
#define STDSIZE  128

// HTTP and HTTPS Default Authentication to store in config file
char web_username[STDSIZE] = "admin";
char web_password[STDSIZE] = "esp8266";
// allows you to set the realm of authentication Default:"Login Required"
const char* www_realm = "Custom Auth Realm";
// the Content of the HTML response in case of Unautherized Access Default:empty
String authFailResponse = "Authentication Failed";

const char *webConfigFile  = "/web.config"; // config file

struct webConfigObj {
  char usernameWebHolder[STDSIZE];
  char passwordWebHolder[STDSIZE];
};
webConfigObj webConfig; // config object

WiFiClient espClient;
PubSubClient mqttClient(espClient);

String clientId;

const char* ap_mode_ssid;
const char* ap_mode_password;

//const char* ap_mode_ssid = clientId.c_str();
//const char* ap_mode_password = "wifi-setup";

char mqttOnOff[STDSIZE] = "off";
char mqttAnonymous[STDSIZE] = "off";
char mqttAlert[STDSIZE] = "off";
char mqttUsername[STDSIZE] = "";
char mqttPassword[STDSIZE] = "";
char mqttServerAddress[STDSIZE] = "192.168.1.1";
char mqttServerPort[STDSIZE] = "1883";
char mqttTopicPrefix[STDSIZE] = "rdadotmatrix";
char mqttTopicDevice[STDSIZE] = "";
char mqttTopicRoot[STDSIZE] = "";

const unsigned long mqttConnectTimeIntervall = 15000; //15*60*1000; // 15 minutes
long mqttLastReconnectAttempt = 0;
int mqttDisconnected = 0;
const char *mqttConfigFile  = "/mqtt.config"; // config file

struct mqttConfigObj {
  char onOffMqttHolder[STDSIZE];
  char anonymousMqttHolder[STDSIZE];
  char alertMqttHolder[STDSIZE];
  char usernameMqttHolder[STDSIZE];
  char passwordMqttHolder[STDSIZE];
  char serverAddressMqttHolder[STDSIZE];
  char serverPortMqttHolder[STDSIZE];
  char topicPrefixMqttHolder[STDSIZE];
};

mqttConfigObj mqttConfig; // config object

typedef struct {
  uint8_t spacing;  // character spacing
  char *msg;  // message to display
} msgDef_t;

msgDef_t  M[] = { { 1, "" } };
#define MAX_STRINGS  (sizeof(M)/sizeof(M[0]))

//Message Parameteres size
#define MSG_SIZE 3000
#define MSG_JSON_SIZE 3000
#define REP_SIZE 4
#define BUZ_SIZE 4
#define DEL_SIZE 4
#define ASC_SIZE 4

String version = VERSION;
String mainPageVars;
String mqttPageVars;
String changeCredVars;
String updateVars;
char newWebUsername[STDSIZE];
char newWebPassword[STDSIZE];
bool newWebUsernameAvailable = false;
bool newWebPasswordAvailable = false;
bool saveWebConfigAtStart = false;
char newMqttOnOff[STDSIZE];
char newMqttAnonymous[STDSIZE];
char newMqttAlert[STDSIZE];
char newMqttUsername[STDSIZE];
char newMqttPassword[STDSIZE];
char newMqttServerAddress[STDSIZE];
char newMqttServerPort[STDSIZE];
char newMqttTopicPrefix[STDSIZE];
bool newMqttOnOffAvailable = false;
bool newMqttAnonymousAvailable = false;
bool newMqttAlertAvailable = false;
bool newMqttUsernameAvailable = false;
bool newMqttPasswordAvailable = false;
bool newMqttServerAddressAvailable = false;
bool newMqttServerPortAvailable = false;
bool newMqttTopicPrefixAvailable = false;
char mqttStatusMsg[64] = "";
char mqttAlertMessage[128] = "";
bool saveMqttConfigAtStart = false;
bool alertMqttConnect =  false; //not yet implemented on gui, only an hard setting for now
char curMessage[MSG_SIZE];
char newMessage[MSG_SIZE];
char newRepeat[REP_SIZE];
char repeatDefault[REP_SIZE] = "10";
char newBuz[BUZ_SIZE];
char buzzerDefault[BUZ_SIZE] = "10";
char newDelay[DEL_SIZE];
char scrollDelayDefault[DEL_SIZE] = "35"; 
char newAsciiconv[ASC_SIZE];
bool firstMessage = true;
bool firstMessageOff = false;
bool newMessageAvailable = false;
bool newRepeatAvailable = false;
bool newBuzAvailable = false;
bool newDelayAvailable = false;
bool newAsciiconvAvailable = false;
bool endSetupModeMsg = false;
bool afterWiFiConfig = false;
char assignedIP[255];
int scrollDelay = atoi(scrollDelayDefault);
int repeatCount = 0;
IPAddress apModeIP;
String apModeSSID;

#if DEBUG
#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }
#define PRINTS(s)   { Serial.print(F(s)); }
#else
#define PRINT(s, v)
#define PRINTS(s)
#endif

//uint8_t degC[] = { 5, 3, 3, 56, 68, 68, 68 }; // Deg C
//uint8_t degF[] = { 6, 3, 3, 124, 20, 20, 4 }; // Deg F
//uint8_t waveSine[] = { 8, 1, 14, 112, 128, 128, 112, 14, 1 }; // Sine wave
//uint8_t waveSqar[] = { 8, 1, 1, 255, 128, 128, 128, 255, 1 }; // Square wave
//uint8_t waveTrng[] = { 10, 2, 4, 8, 16, 32, 64, 32, 16, 8, 4 }; // Triangle wave