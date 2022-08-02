void setMainPageVars() {
  mainPageVars = String("<data><clientid>")
  + clientId + String("</clientid><repeat>") 
  + String(repeatDefault) + String("</repeat><buzzer>") 
  + String(buzzerDefault) + String("</buzzer><delay>") 
  + String(scrollDelayDefault) + String("</delay><version>") 
  + String(version) + String("</version></data>");
}

void setMqttPageVars() {
  mqttPageVars = String("<data><clientid>")
  + clientId + String("</clientid><mqttonoff>") 
  + String(mqttOnOff) + String("</mqttonoff><mqttanonymous>") 
  + String(mqttAnonymous) + String("</mqttanonymous><mqttalert>") 
  + String(mqttAlert) + String("</mqttalert><mqttusername>") 
  + String(mqttUsername) + String("</mqttusername><mqttserveraddress>") 
  + String(mqttServerAddress) + String("</mqttserveraddress><mqttserverport>") 
  + String(mqttServerPort) + String("</mqttserverport><mqtttopicprefix>") 
  + String(mqttTopicPrefix) + String("</mqtttopicprefix><version>")
  + String(version) + String("</version></data>");
}

void setChangeCredVars() {
  changeCredVars = String("<data><clientid>")
  + clientId + String("</clientid><username>") 
  + String(web_username) + String("</username><version>")
  + String(version) + String("</version></data>");
}

void setUpdateVars() {
  updateVars = String("<data><clientid>")
  + clientId + String("</clientid><version>")
  + String(version) + String("</version></data>");
}

// Loads the configuration from a file
void loadConfiguration(const char *webConfigFile, webConfigObj &webConfig) {

  File file = LittleFS.open(webConfigFile, "r");
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

  // Copy values from the JsonDocument to the webConfig
  strlcpy(webConfig.usernameWebHolder,                  // <- destination
          doc["usernameWebHolder"],                  // <- source
          sizeof(webConfig.usernameWebHolder));         // <- destination's capacity
  strlcpy(webConfig.passwordWebHolder,                  // <- destination
          doc["passwordWebHolder"],                  // <- source
          sizeof(webConfig.passwordWebHolder));         // <- destination's capacity

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}

// Saves the configuration to a file
void saveConfiguration(const char *webConfigFile, const webConfigObj &webConfig) {
  // Delete existing file, otherwise the configuration is appended to the file
  //LittleFS.remove(webConfigFile);

  // Open file for writing
  File file = LittleFS.open(webConfigFile, "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return;
  }
  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<512> doc;
  
  // Set the values in the document
  doc["usernameWebHolder"] = webConfig.usernameWebHolder;
  doc["passwordWebHolder"] = webConfig.passwordWebHolder;
  
  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  
  // Close the file
  file.close();
}

// Prints the content of a file to the Serial
void printWebFile(const char *webConfigFile) {
  // Open file for reading

  File file = LittleFS.open(webConfigFile, "r");
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
void changeWebLoginCredentials() {
  //set username and password from webpage to config object
  strlcpy(webConfig.usernameWebHolder, newWebUsername, sizeof(webConfig.usernameWebHolder));
  strlcpy(webConfig.passwordWebHolder, newWebPassword, sizeof(webConfig.passwordWebHolder));
  //save username and password from config object to config file
  saveConfiguration(webConfigFile, webConfig);
  //set the http/https credentials to the new password
  strlcpy(web_username, webConfig.usernameWebHolder, sizeof(web_username));
  strlcpy(web_password, webConfig.passwordWebHolder, sizeof(web_password));
  // Dump config file
  PRINTS("Username and Password changed\nPrinting web user config file:\n");
  printWebFile(webConfigFile);
}

void initWebStoreConfig() {
  //load config stored in config file
  Serial.println(F("Loading web configuration...\n"));
  loadConfiguration(webConfigFile, webConfig);
  //if no username is defined in config file store default
  if ((webConfig.usernameWebHolder != NULL) && (webConfig.usernameWebHolder[0] == '\0')) {
    PRINT("no username set, setting default username: ", web_username);
    strlcpy(webConfig.usernameWebHolder, web_username, sizeof(webConfig.usernameWebHolder));
    saveWebConfigAtStart = true;
  }
  //if no password is defined in config file store default
  if ((webConfig.passwordWebHolder != NULL) && (webConfig.passwordWebHolder[0] == '\0')) {
    PRINTS("\n")
    PRINT("no password set, setting default password: ", web_password);
    strlcpy(webConfig.passwordWebHolder, web_password, sizeof(webConfig.passwordWebHolder));
    saveWebConfigAtStart = true;
  }
  PRINTS("\n")
  //set http/https server to config file defined values or defined default
  strlcpy(web_username, webConfig.usernameWebHolder, sizeof(web_username));
  strlcpy(web_password, webConfig.passwordWebHolder, sizeof(web_password));
  
  // Create configuration file
  if (saveWebConfigAtStart) {
    Serial.println(F("Saving web user configuration..."));
    saveConfiguration(webConfigFile, webConfig);
  }

  // Dump config file
  Serial.println(F("Print web user config file...\n"));
  printWebFile(webConfigFile);
}

//################################ START OF SPECIFIC HTTP SERVER FUNCTIONS ################################//
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
      serverHttp.arg(i).toCharArray(newWebUsername, STDSIZE);
      newWebUsernameAvailable = true;
    }
    if (serverHttp.argName(i) == "Password") {
      serverHttp.arg(i).toCharArray(newWebPassword, STDSIZE);
      newWebPasswordAvailable = true;
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
      serverHttp.arg(i).toCharArray(newMessage, MSG_SIZE);
      newMessageAvailable = true;
    }
    if (serverHttp.argName(i) == "REP") {
      serverHttp.arg(i).toCharArray(newRepeat, REP_SIZE);
      repeatCount = 0;
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
    if (serverHttp.argName(i) == "ASC") {
      serverHttp.arg(i).toCharArray(newAsciiconv, ASC_SIZE);
      newAsciiconvAvailable = true;
    }
  }
  Serial.println(message);
  showWebpageHttp();
}

void serverHttp_post_api() {
  char data[MSG_JSON_SIZE];
  strcpy(data, serverHttp.arg("plain").c_str());
  StaticJsonDocument<MSG_JSON_SIZE> doc;
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

  serverHttp.send(204,"");
}

void showChangeMqttConfigHttp() {
  String s = CHANGEMQTTCONFIG_page; //Read HTML contents
  serverHttp.send(200, "text/html", s); //Send web page
}

void onMqttConfigChangeHttp() {
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
    if (serverHttp.argName(i) == "MQTTONOFF") {
      serverHttp.arg(i).toCharArray(newMqttOnOff, STDSIZE);
      if ((newMqttOnOff != NULL) && (newMqttOnOff[0] == '\0')) {
        newMqttOnOffAvailable = false;
      } else { newMqttOnOffAvailable = true; }
    }
    if (serverHttp.argName(i) == "MQTTANONYMOUS") {
      serverHttp.arg(i).toCharArray(newMqttAnonymous, STDSIZE);
      if ((newMqttAnonymous != NULL) && (newMqttAnonymous[0] == '\0')) {
        newMqttAnonymousAvailable = false;
      } else { newMqttAnonymousAvailable = true; }
    }
    if (serverHttp.argName(i) == "MQTTALERT") {
      serverHttp.arg(i).toCharArray(newMqttAlert, STDSIZE);
      if ((newMqttAlert != NULL) && (newMqttAlert[0] == '\0')) {
        newMqttAlertAvailable = false;
      } else { newMqttAlertAvailable = true; }
    }
    if (serverHttp.argName(i) == "MQTTUSERNAME") {
      serverHttp.arg(i).toCharArray(newMqttUsername, STDSIZE);
      if ((newMqttUsername != NULL) && (newMqttUsername[0] == '\0')) {
        newMqttUsernameAvailable = false;
      } else { newMqttUsernameAvailable = true; }
    }
    if (serverHttp.argName(i) == "MQTTPASSWORD") {
      serverHttp.arg(i).toCharArray(newMqttPassword, STDSIZE);
      if ((newMqttPassword != NULL) && (newMqttPassword[0] == '\0')) {
        newMqttPasswordAvailable = false;
      } else { newMqttPasswordAvailable = true; }
    }
    if (serverHttp.argName(i) == "MQTTSERVERADDRESS") {
      serverHttp.arg(i).toCharArray(newMqttServerAddress, STDSIZE);
      if ((newMqttServerAddress != NULL) && (newMqttServerAddress[0] == '\0')) {
        newMqttServerAddressAvailable = false;
      } else { newMqttServerAddressAvailable = true; }
    }
    if (serverHttp.argName(i) == "MQTTSERVERPORT") {
      serverHttp.arg(i).toCharArray(newMqttServerPort, STDSIZE);
      if ((newMqttServerPort != NULL) && (newMqttServerPort[0] == '\0')) {
        newMqttServerPortAvailable = false;
      } else { newMqttServerPortAvailable = true; }
    }
    if (serverHttp.argName(i) == "MQTTTOPICPREFIX") {
      serverHttp.arg(i).toCharArray(newMqttTopicPrefix, STDSIZE);
      if ((newMqttTopicPrefix != NULL) && (newMqttTopicPrefix[0] == '\0')) {
        newMqttTopicPrefixAvailable = false;
      } else { newMqttTopicPrefixAvailable = true; }
    }
  }
  Serial.println(message);
}

void onNotFoundUriHttp() {
  serverHttp.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void httpWebDirDef(){
  serverHttp.on("/", []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    showWebpageHttp();
  });
  serverHttp.on("/mainpagevars", []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    setMainPageVars();
    serverHttp.send(200, "text/plane", mainPageVars);
  });
  serverHttp.on("/changecredvars", []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    setChangeCredVars();
    serverHttp.send(200, "text/plane", changeCredVars);
  });
  serverHttp.on("/updatevars", []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    setUpdateVars();
    serverHttp.send(200, "text/plane", updateVars);
  });
  serverHttp.on("/arg", []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    onMessageCallHttp();
  });
  serverHttp.on("/api", HTTP_POST, [](){
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    serverHttp_post_api();
  });  
  serverHttp.on("/changeuserpass", [](){
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    showChangeCredentialsHttp();
  });
  serverHttp.on("/changecredentials", [](){
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    usernamePasswordHttp();
	  changeWebLoginCredentials();
    serverHttp.sendHeader("Connection", "close");
    serverHttp.send(200, "text/html", APPLYUSERPASS_page);
  });
  serverHttp.on("/changemqttconfig", [](){
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    showChangeMqttConfigHttp();
  });
  serverHttp.on("/mqttpagevars", []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    setMqttPageVars();
    serverHttp.send(200, "text/plane", mqttPageVars);
  });
  serverHttp.on("/applymqttconfig", [](){
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    onMqttConfigChangeHttp();
	  changeMqttConfig();
    serverHttp.sendHeader("Connection", "close");
    serverHttp.send(200, "text/html", APPLYMQTTCONFIG_page);
  });
  serverHttp.on("/update", HTTP_GET, []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    serverHttp.sendHeader("Connection", "close");
    serverHttp.send(200, "text/html", UPDATE_page);
  });
  serverHttp.on("/reboot", HTTP_GET, []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    serverHttp.sendHeader("Connection", "close");
    serverHttp.send(200, "text/html", REBOOT_page);
    delay(2000);
    rebootDevice();
  });
  serverHttp.on("/factoryreset", HTTP_GET, []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    serverHttp.sendHeader("Connection", "close");
    serverHttp.send(200, "text/html", FACTORYRESET_page);
    factoryReset();
  });
  serverHttp.on("/submitupdate", HTTP_POST, []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    serverHttp.sendHeader("Connection", "close");
    serverHttp.send(200, "text/html", (Update.hasError()) ? SUBMITUPDATEFAIL_page : SUBMITUPDATEOK_page);
  }, []() {
    if (!serverHttp.authenticate(web_username, web_password)) {
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
        serverHttp.send(200, "text/html", SUBMITUPDATESUCCESS_page);
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
    if (!serverHttp.authenticate(web_username, web_password)) {
      return serverHttp.requestAuthentication();
    }
    onNotFoundUriHttp();
  });
  
  // Start the http server
  serverHttp.begin();
  PRINTS("HTTP Server started on port 80\n");
  Serial.printf("You can update firmware from the browser opening! -> Open http://%s/update in your browser\n\n", assignedIP);
}

void handleHttpServer() {
  serverHttp.handleClient();
}

//################################ END OF SPECIFIC HTTP SERVER FUNCTIONS ################################//