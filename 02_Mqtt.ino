//processes plain messages
void plainMsgFunct(String plainMsgString) {
  strcpy(newMessage, plainMsgString.c_str());
  PRINT("\nMQTT Plain Message Received!\nMQTT Message:\n", newMessage);
  repeatCount = 0;
  newMessageAvailable = true;
  strcpy(newRepeat, repeatDefault);
  newRepeatAvailable = true;
  strcpy(newBuz, buzzerDefault);
  newBuzAvailable = true;
  strcpy(newDelay, scrollDelayDefault);
  newDelayAvailable = true;
  strcpy(newAsciiconv, "1");
  newAsciiconvAvailable = true;
}

//processes json messages
void jsonMsgFunct(String jsonMsgString) {
  char data[MSG_JSON_SIZE];
  strcpy(data, jsonMsgString.c_str());
  PRINT("\nMQTT JSON Message Arrived!\nMQTT Message: ", data);
  StaticJsonDocument<MSG_JSON_SIZE> doc;
  
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
  
  //MSG.replace("'", "\u0027");
  
  MSG.toCharArray(newMessage, MSG_SIZE);
  newMessageAvailable = true;
  
  REP.toCharArray(newRepeat, REP_SIZE);
  repeatCount = 0;
  newRepeatAvailable = true;
  
  BUZ.toCharArray(newBuz, BUZ_SIZE);
  newBuzAvailable = true;
  
  DEL.toCharArray(newDelay, DEL_SIZE);
  newDelayAvailable = true;
  
  ASC.toCharArray(newAsciiconv, ASC_SIZE);
  newAsciiconvAvailable = true;
}

//function called when an MQTT message is received
void mqttCallBack(const char *topic, byte *payload, unsigned int length) {
  PRINTS("\nMQTT CALLBACK SEEN!")
  String PayloadString = "";
  payload[length] = '\0';  
  for (int i = 0; i < length; i++) {
    PayloadString += (char)payload[i];
  }
  //matches when messages come in with topic NOT ending in /json and when topic configured is NOT wildcard #
  if ((String(topic).startsWith((String(mqttTopicPrefix))) && ((String(topic).endsWith("/json")) == false)) ||
  ((String(topic).startsWith(String(mqttTopicRoot))) && ((String(topic).endsWith("/json")) == false)) ||
  (strcmp(topic, (char*) (String(mqttTopicDevice) + "").c_str()) == 0)) {
    plainMsgFunct(PayloadString);
  }
  //matches when messages come in with topic ending in /json and when topic configured is NOT wildcard #
  else if ((String(topic).startsWith((String(mqttTopicPrefix))) && ((String(topic).endsWith("/json")) == true)) ||
  ((String(topic).startsWith(String(mqttTopicRoot))) && ((String(topic).endsWith("/json")) == true)) ||
  ((String(topic).startsWith(String(mqttTopicDevice))) && ((String(topic).endsWith("/json")) == true))) {
    jsonMsgFunct(PayloadString);
  }
  //matches when message come in with topic ending in /json and when topic is wildcard #
  else if ((String(topic).endsWith("/json")) == true) {
    jsonMsgFunct(PayloadString);
  }
  //matches when message come in with topic NOT ending in /json and when topic is wildcard #
  else {
    plainMsgFunct(PayloadString);
  }
}

void mqttConnectProc(char mqttAlertMessageIn[128], bool buzz) {
  PRINTS("\n\nRestoring MQTT connection...");
  // Attempt to connect
  PRINTS("\n");
  Serial.print(clientId);
  PRINT(" connected to MQTT Server: ", mqttServerAddress);
  PRINT(":", mqttServerPort);
  //// Once connected, publish an announcement...
  mqttClient.publish((char*) (String(mqttTopicDevice) + "/status").c_str(), "Connected");//clientId.c_str());
  PRINT("\nPublishing to topic ", (String(mqttTopicDevice)  + "/status"));
  PRINTS(": connected");
  //PRINTS("\n");
  // subscribe
  //####mqttClient.subscribe((char*) (TOPIC_PREFIX + "/#").c_str());
  int slashIndex = String(mqttTopicPrefix).indexOf('/');
  int hashIndex = String(mqttTopicPrefix).indexOf('#');
  strcpy(mqttTopicRoot, (char*) (String(mqttTopicPrefix).substring(0, slashIndex)).c_str());
  String strMqttTopicRoot = String(mqttTopicRoot);
  String strMqttTopicPrefix = String(mqttTopicPrefix);
  String strMqttTopicDevice = String(mqttTopicDevice);
  mqttClient.subscribe((char*) strMqttTopicRoot.c_str());
  PRINT("\nSubscribe to topic: ", strMqttTopicRoot);
  if (hashIndex == 1) {
    mqttClient.subscribe((char*) (strMqttTopicRoot + "/json").c_str());
    PRINT("\nSubscribe to topic: ", (strMqttTopicRoot + "/json"));
  }
  if (strMqttTopicPrefix != strMqttTopicRoot) {
    mqttClient.subscribe((char*) (strMqttTopicRoot + "/json").c_str());
    PRINT("\nSubscribe to topic: ", (strMqttTopicRoot + "/json"));
    mqttClient.subscribe((char*) (strMqttTopicPrefix + "").c_str());
    PRINT("\nSubscribe to topic: ", (strMqttTopicPrefix + ""));
  }
  if (hashIndex == -1) {
    mqttClient.subscribe((char*) (strMqttTopicPrefix + "/json").c_str());
    PRINT("\nSubscribe to topic: ", (strMqttTopicPrefix + "/json"));
  }
  mqttClient.subscribe((char*) (strMqttTopicDevice + "").c_str());
  PRINT("\nSubscribe to topic: ", (strMqttTopicDevice + ""));
  mqttClient.subscribe((char*) (strMqttTopicDevice + "/json").c_str());
  PRINT("\nSubscribe to topic: ", (strMqttTopicDevice+ "/json"));
  PRINTS("\n");
  if ((strcmp(mqttAlert, "on") == 0)  and (buzz)) {
    sprintf(newMessage,mqttAlertMessageIn);
    mqttConnectBuzzer();
    displaySilentMsg();
  }
}

void mqttDisconnectedProc(char mqttAlertMessageIn[128], bool buzzIn) {
  PRINTS("\n");
  Serial.print(clientId);
  PRINT(" has lost connection to MQTT Server: ", mqttServerAddress);
  PRINT(":", mqttServerPort);
  if ((strcmp(mqttAlert, "on") == 0) and (buzzIn)){
    sprintf(newMessage,mqttAlertMessageIn);
    mqttDisconnectBuzzer();
    displaySilentMsg();
  }
}

boolean reconnectMqtt() {
  if (strcmp(mqttAnonymous, "off") == 0) {
    if (mqttClient.connect(clientId.c_str(), mqttUsername, mqttPassword)) {
      sprintf(mqttAlertMessage, "%sMQTT connected to server: %s:%s in user mode", mqttStatusMsg, mqttServerAddress, mqttServerPort);
      sprintf(mqttStatusMsg,  "");
      mqttConnectProc(mqttAlertMessage, true);
      mqttDisconnected = 0;
    }
    else {
      mqttDisconnected += 1;
      if (mqttDisconnected == 1) {
        sprintf(mqttAlertMessage, "%sMQTT disconnected from Server: %s:%s", mqttStatusMsg, mqttServerAddress, mqttServerPort);
        sprintf(mqttStatusMsg,  "");
        mqttDisconnectedProc(mqttAlertMessage, true);
      }
    }
  }
  else if (strcmp(mqttAnonymous, "on") == 0) {
    if (mqttClient.connect(clientId.c_str())) {
      sprintf(mqttAlertMessage, "%sMQTT connected to server: %s:%s in anonymous mode", mqttStatusMsg, mqttServerAddress, mqttServerPort);
      sprintf(mqttStatusMsg,  "");
      mqttConnectProc(mqttAlertMessage, true);
      mqttDisconnected = 0;
    }
    else {
      mqttDisconnected += 1;
      if (mqttDisconnected == 1) {
        sprintf(mqttAlertMessage, "%sMQTT disconnected from Server: %s:%s", mqttStatusMsg, mqttServerAddress, mqttServerPort);
        sprintf(mqttStatusMsg,  "");
        mqttDisconnectedProc(mqttAlertMessage, true);
      }
    }
  }
  return mqttClient.connected();
}

// Loads the configuration from a file
void loadMqttConfiguration(const char *mqttConfigFile, mqttConfigObj &mqttConfig) {

  File file = LittleFS.open(mqttConfigFile, "r");
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

  // Copy values from the JsonDocument to the mqttConfig
  strlcpy(mqttConfig.onOffMqttHolder,
          doc["onOffMqttHolder"],
          sizeof(mqttConfig.onOffMqttHolder));
  strlcpy(mqttConfig.anonymousMqttHolder,
          doc["anonymousMqttHolder"],
          sizeof(mqttConfig.anonymousMqttHolder));
  strlcpy(mqttConfig.alertMqttHolder,
          doc["alertMqttHolder"],
          sizeof(mqttConfig.alertMqttHolder));
  strlcpy(mqttConfig.usernameMqttHolder,
          doc["usernameMqttHolder"],
          sizeof(mqttConfig.usernameMqttHolder));
  strlcpy(mqttConfig.passwordMqttHolder,
          doc["passwordMqttHolder"],
          sizeof(mqttConfig.passwordMqttHolder));
  strlcpy(mqttConfig.serverAddressMqttHolder,
          doc["serverAddressMqttHolder"],
          sizeof(mqttConfig.serverAddressMqttHolder));
  strlcpy(mqttConfig.serverPortMqttHolder,
          doc["serverPortMqttHolder"],
          sizeof(mqttConfig.serverPortMqttHolder));
  strlcpy(mqttConfig.topicPrefixMqttHolder,
          doc["topicPrefixMqttHolder"],
          sizeof(mqttConfig.topicPrefixMqttHolder));


  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}

// Saves the configuration to a file
void saveMqttConfiguration(const char *mqttConfigFile, const mqttConfigObj &mqttConfig) {
  // Delete existing file, otherwise the configuration is appended to the file
  //LittleFS.remove(mqttConfigFile);

  // Open file for writing
  File file = LittleFS.open(mqttConfigFile, "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return;
  }
  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<512> doc;
  
  // Set the values in the document
  doc["onOffMqttHolder"] = mqttConfig.onOffMqttHolder;
  doc["anonymousMqttHolder"] = mqttConfig.anonymousMqttHolder;
  doc["alertMqttHolder"] = mqttConfig.alertMqttHolder;
  doc["usernameMqttHolder"] = mqttConfig.usernameMqttHolder;
  doc["passwordMqttHolder"] = mqttConfig.passwordMqttHolder;
  doc["serverAddressMqttHolder"] = mqttConfig.serverAddressMqttHolder;
  doc["serverPortMqttHolder"] = mqttConfig.serverPortMqttHolder;
  doc["topicPrefixMqttHolder"] = mqttConfig.topicPrefixMqttHolder;
  
  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  
  // Close the file
  file.close();
}

// Prints the content of a file to the Serial
void printMqttFile(const char *mqttConfigFile) {
  // Open file for reading

  File file = LittleFS.open(mqttConfigFile, "r");
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
void changeMqttConfig() {
  //set new mqtt config values from webpage to config object
  if (newMqttOnOffAvailable) {
    strlcpy(mqttConfig.onOffMqttHolder, newMqttOnOff, sizeof(mqttConfig.onOffMqttHolder));
  }
  if (newMqttAnonymousAvailable) {
    strlcpy(mqttConfig.anonymousMqttHolder, newMqttAnonymous, sizeof(mqttConfig.anonymousMqttHolder));
  }
  if (newMqttAlertAvailable) {
    strlcpy(mqttConfig.alertMqttHolder, newMqttAlert, sizeof(mqttConfig.alertMqttHolder));
  }
  if (newMqttUsernameAvailable) {
    strlcpy(mqttConfig.usernameMqttHolder, newMqttUsername, sizeof(mqttConfig.usernameMqttHolder));
  }
  if (newMqttPasswordAvailable) {
    strlcpy(mqttConfig.passwordMqttHolder, newMqttPassword, sizeof(mqttConfig.passwordMqttHolder));
  }
  if (newMqttServerAddressAvailable) {
    strlcpy(mqttConfig.serverAddressMqttHolder, newMqttServerAddress, sizeof(mqttConfig.serverAddressMqttHolder));
  }
  if (newMqttServerPortAvailable) {
    strlcpy(mqttConfig.serverPortMqttHolder, newMqttServerPort, sizeof(mqttConfig.serverPortMqttHolder));
  }
  if (newMqttTopicPrefixAvailable) {
    strlcpy(mqttConfig.topicPrefixMqttHolder, newMqttTopicPrefix, sizeof(mqttConfig.topicPrefixMqttHolder));
  }
  //save new mqtt config values set from config object to config file
  saveMqttConfiguration(mqttConfigFile, mqttConfig);
  //apply the new mqtt config values as running values
  strlcpy(mqttOnOff, mqttConfig.onOffMqttHolder, sizeof(mqttOnOff));
  strlcpy(mqttAnonymous, mqttConfig.anonymousMqttHolder, sizeof(mqttAnonymous));
  strlcpy(mqttAlert, mqttConfig.alertMqttHolder, sizeof(mqttAlert));
  strlcpy(mqttUsername, mqttConfig.usernameMqttHolder, sizeof(mqttUsername));
  strlcpy(mqttPassword, mqttConfig.passwordMqttHolder, sizeof(mqttPassword));
  strlcpy(mqttServerAddress, mqttConfig.serverAddressMqttHolder, sizeof(mqttServerAddress));
  strlcpy(mqttServerPort, mqttConfig.serverPortMqttHolder, sizeof(mqttServerPort));
  strlcpy(mqttTopicPrefix, mqttConfig.topicPrefixMqttHolder, sizeof(mqttTopicPrefix));

  // Dump config file
  PRINTS("MQTT config changed.\nPrinting config file:\n");
  printMqttFile(mqttConfigFile);

  if (strcmp(mqttOnOff, "on") == 0) {
    PRINTS("\n");
    Serial.print(clientId);
    PRINTS("MQTT Enabled!");
    sprintf(mqttStatusMsg,  "MQTT ENABLED. ");
  }
  else if (strcmp(mqttOnOff, "off") == 0) {
    mqttClient.disconnect();
    PRINTS("\n");
    Serial.print(clientId);
    PRINTS("MQTT Disable!");
    sprintf(mqttStatusMsg,  "MQTT DISABLED. ");
    sprintf(mqttAlertMessage, "%sMQTT disconnected from Server: %s:%s", mqttStatusMsg, mqttServerAddress, mqttServerPort);
    mqttDisconnectedProc(mqttAlertMessage, true);
    sprintf(mqttStatusMsg,  "");
  }
}

void initMqttStoreConfig() {
  //load config stored in config file
  Serial.println(F("Loading Mqtt configuration...\n"));
  loadMqttConfiguration(mqttConfigFile, mqttConfig);
  //if no onoff is defined in config file store default
  if ((mqttConfig.onOffMqttHolder != NULL) && (mqttConfig.onOffMqttHolder[0] == '\0')) {
    PRINT("no onoff set, setting default onoff: ", mqttOnOff);
    strlcpy(mqttConfig.onOffMqttHolder, mqttOnOff, sizeof(mqttConfig.onOffMqttHolder));
    saveMqttConfigAtStart = true;
  }
  //if no anonymous is defined in config file store default
  if ((mqttConfig.anonymousMqttHolder != NULL) && (mqttConfig.anonymousMqttHolder[0] == '\0')) {
    PRINT("no anonymous set, setting default anonymous: ", mqttAnonymous);
    strlcpy(mqttConfig.anonymousMqttHolder, mqttAnonymous, sizeof(mqttConfig.anonymousMqttHolder));
    saveMqttConfigAtStart = true;
  }
  //if no alert is defined in config file store default
  if ((mqttConfig.alertMqttHolder != NULL) && (mqttConfig.alertMqttHolder[0] == '\0')) {
    PRINT("no alert set, setting default alert: ", mqttAlert);
    strlcpy(mqttConfig.alertMqttHolder, mqttAlert, sizeof(mqttConfig.alertMqttHolder));
    saveMqttConfigAtStart = true;
  }
  //if no username is defined in config file store default
  if ((mqttConfig.usernameMqttHolder != NULL) && (mqttConfig.usernameMqttHolder[0] == '\0')) {
    PRINTS("\n")
    PRINT("no username set, setting default username: ", mqttUsername);
    strlcpy(mqttConfig.usernameMqttHolder, mqttUsername, sizeof(mqttConfig.usernameMqttHolder));
    saveMqttConfigAtStart = true;
  }
  //if no password is defined in config file store default
  if ((mqttConfig.passwordMqttHolder != NULL) && (mqttConfig.passwordMqttHolder[0] == '\0')) {
    PRINTS("\n")
    PRINT("no password set, setting default password: ", mqttPassword);
    strlcpy(mqttConfig.passwordMqttHolder, mqttPassword, sizeof(mqttConfig.passwordMqttHolder));
    saveMqttConfigAtStart = true;
  }
  //if no mqtt server address is defined in config file store default
  if ((mqttConfig.serverAddressMqttHolder != NULL) && (mqttConfig.serverAddressMqttHolder[0] == '\0')) {
    PRINTS("\n")
    PRINT("no mqtt server address set, setting default mqtt server address: ", mqttServerAddress);
    strlcpy(mqttConfig.serverAddressMqttHolder, mqttServerAddress, sizeof(mqttConfig.serverAddressMqttHolder));
    saveMqttConfigAtStart = true;
  }
  //if no mqtt server port is defined in config file store default
  if ((mqttConfig.serverPortMqttHolder != NULL) && (mqttConfig.serverPortMqttHolder[0] == '\0')) {
    PRINTS("\n")
    PRINT("no mqtt server port set, setting default mqtt server port: ", mqttServerPort);
    strlcpy(mqttConfig.serverPortMqttHolder, mqttServerPort, sizeof(mqttConfig.serverPortMqttHolder));
    saveMqttConfigAtStart = true;
  }
  //if no mqtt topic prefix is defined in config file store default
  if ((mqttConfig.topicPrefixMqttHolder != NULL) && (mqttConfig.topicPrefixMqttHolder[0] == '\0')) {
    PRINTS("\n")
    PRINT("no mqtt topic prefix set, setting default mqtt topic prefix: ", mqttTopicPrefix);
    strlcpy(mqttConfig.topicPrefixMqttHolder, mqttTopicPrefix, sizeof(mqttConfig.topicPrefixMqttHolder));
    saveMqttConfigAtStart = true;
  }
  PRINTS("\n")
  strlcpy(mqttOnOff, mqttConfig.onOffMqttHolder, sizeof(mqttOnOff));
  strlcpy(mqttAnonymous, mqttConfig.anonymousMqttHolder, sizeof(mqttAnonymous));
  strlcpy(mqttAlert, mqttConfig.alertMqttHolder, sizeof(mqttAlert));
  strlcpy(mqttUsername, mqttConfig.usernameMqttHolder, sizeof(mqttUsername));
  strlcpy(mqttPassword, mqttConfig.passwordMqttHolder, sizeof(mqttPassword));
  strlcpy(mqttServerAddress, mqttConfig.serverAddressMqttHolder, sizeof(mqttServerAddress));
  strlcpy(mqttServerPort, mqttConfig.serverPortMqttHolder, sizeof(mqttServerPort));
  strlcpy(mqttTopicPrefix, mqttConfig.topicPrefixMqttHolder, sizeof(mqttTopicPrefix));
  
  // Create mqtt configuration file
  if (saveMqttConfigAtStart) {
    Serial.println(F("Saving mqtt configuration..."));
    saveMqttConfiguration(mqttConfigFile, mqttConfig);
  }

  // Dump mqtt config file
  Serial.println(F("Print mqtt config file...\n"));
  printMqttFile(mqttConfigFile);
}

void initMqtt(){
  if (strcmp(mqttOnOff, "on") == 0) {
    mqttDisconnected = 0;
    mqttClient.setClient(espClient);
    mqttClient.setServer(mqttServerAddress, atoi(mqttServerPort));  //1883 
    mqttClient.setCallback(mqttCallBack);
    mqttClient.setBufferSize(2048);
  }
}

void checkMqtt() {
  if (newMqttOnOffAvailable) {
    newMqttOnOffAvailable = false;
    if (mqttClient.connected()) {
      mqttClient.disconnect();
    }
    else {
      mqttDisconnected = 1;
    }
    if (strcmp(mqttOnOff, "on") == 0) {
      initMqtt();
    }
  }
  if (mqttDisconnected == 1) {
    if (((newMqttOnOffAvailable == false) and (firstMessage == true)) or ((newMqttOnOffAvailable == true) and (firstMessage == false))){
      sprintf(mqttAlertMessage, "%sMQTT disconnected from Server: %s:%s", mqttStatusMsg, mqttServerAddress, mqttServerPort);
      sprintf(mqttStatusMsg,  "");
      mqttDisconnectedProc(mqttAlertMessage, true);
    }
  }
  if (strcmp(mqttOnOff, "on") == 0) {
    if (!mqttClient.connected()) {
      long now = millis();
      if (now - mqttLastReconnectAttempt > mqttConnectTimeIntervall) {
        mqttLastReconnectAttempt = now;
        if (reconnectMqtt()) {
          mqttLastReconnectAttempt = 0;
        }
      }
    }
    mqttClient.loop();
  }
}