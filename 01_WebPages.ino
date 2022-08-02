
const char MAIN_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head><meta name="viewport" content="width=device-width, initial-scale=1">
<title>MAX7219 RDA - Message Board</title>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  text-align: center;
  background-color: #2f3136;
}
#message_container {
  border-radius: 8px;
  color: black;
  width: 90%;
  height: 100%;
  margin-left: auto;
  margin-right: auto;
  margin-top: 20px;
  border: solid 2px;
  padding: 10px;
  background-color: #9f9fa6;
  padding: 10px;
  position: centre;
  text-align: center;
}
#firmware_container {
  color: black;
  width: 200px;
  height: 100%;
  margin-left: auto;
  margin-right: auto;
  margin-top: auto;
  margin-bottom: auto;
  border: solid 2px;
  padding-top: 10px;
  padding-bottom: 3px;
  background-color: #914f41;
}
#clientid_container {
  font-family: Arial;
  font-size: 14px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: white;
}
#version_container {
  font-family: Arial;
  font-size: 12px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: #ffff00;
}
label {
  display: block;
  padding-left: 0px;
  text-indent: 0px;
  font-family: Arial;
  font-size: 14px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: black;
}
input {
  text-align: center;
  width: 200px;
  padding: 5px;
  border: 2px solid #ccc;
  border-radius: 8px;
  box-sizing: border-box;
  margin-top: 2px;
  margin-bottom: 10px;
}
input.small-field {
  width: 100px;
}
input.msg-field {
  width: 70%;
}
input[type=submit] {
  background-color: #04AA6D;
  color: white;
  width: 150px;
  box-shadow: 0 5px #666;
}
input[type=submit]:hover {
  background-color: #32ba39;
  width: 150px;
  box-shadow: 0 5px #666;
}
input[type=submit]:active {
  background-color: #32ba39;
  box-shadow: 0 5px #666;
  transform: translateY(4px);
}
.pill-nav a {
  display: inline-block;
  color: #eee;
  border: 2px solid #ccc;
  text-align: center;
  padding: 14px;
  text-decoration: none;
  font-size: 15px;
  border-radius: 5px;
  background-color: #555;
  width: 150px;
  margin-bottom: 10px;
  margin-right: 5px;
  margin-left: 5px;
}
.pill-nav a:hover {
  background-color: #ddd;
  color: black;
}
.pill-nav a.active {
  background-color: #9f9fa6;
  color: black;
}
.pill-nav a.firmware {
  color: #ff3347;
}
.pill-nav a.firmware:hover {
  color: #ff3347;
}
</style>
<script>
function SendText() {
  var request = new XMLHttpRequest();
  msg = "&MSG=" + document.getElementById("txt_form").MSG.value;
  repeat = "&REP=" + document.getElementById("txt_form").REP.value;
  buzzer = "&BUZ=" + document.getElementById("txt_form").BUZ.value;
  delay = "&DEL=" + document.getElementById("txt_form").DEL.value;
  asciiconv = "&ASC=0";
  request.open("GET", "arg?" + msg + repeat + buzzer + delay + asciiconv, false);
  request.send(null);
}
function getData() {
  var parser, xmlDoc;
  var request = new XMLHttpRequest();
  parser = new DOMParser();
  request.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      xmlDoc = parser.parseFromString(this.responseText,"text/xml");
      document.getElementById("clientid_val").innerHTML = 
      xmlDoc.getElementsByTagName("clientid")[0].childNodes[0].nodeValue;
      document.getElementById("version_val").innerHTML = 
      xmlDoc.getElementsByTagName("version")[0].childNodes[0].nodeValue;
      document.getElementById("txt_form").REP.value = 
      xmlDoc.getElementsByTagName("repeat")[0].childNodes[0].nodeValue;
      document.getElementById("txt_form").BUZ.value = 
      xmlDoc.getElementsByTagName("buzzer")[0].childNodes[0].nodeValue;
      document.getElementById("txt_form").DEL.value = 
      xmlDoc.getElementsByTagName("delay")[0].childNodes[0].nodeValue;
	  }
  };
  request.open("GET", "mainpagevars", true);
  request.send();
}
getData();
</script>
</head>
<body>
<H1><b>MAX7219 RDA - Message Board</b></H1> 
<div id="message_container">
<form id="txt_form" name="frmText">
<br/>
<label>Message</label><input type="text" class="msg-field" id="MSG" maxlength="999">
<label>Repeat</label><input type="text" class="small-field" id="REP" maxlength="3" size="3">
<label>Buzzer</label><input type="text" class="small-field" id="BUZ" maxlength="3" size="3">
<label>Delay</label><input type="text" class="small-field" id="DEL" maxlength="3" size="3">
</br>
</form>
<br>
<input type="submit" value="Send Message" onclick="SendText()">
</div>
<br>
<div class="pill-nav">
  <a class="active" href="/"> Home</a>
  <a href="/changemqttconfig"> MQTT Configuration</a>
  <a href="/changeuserpass">Web Credentials</a>
  <a class="firmware" href="/update">Update Firmware</a>
</div>
<div id="clientid_container">
<p>Hostname: <span id="clientid_val"></span></p>
</div>
<div id="version_container">
<p>Firmware Version: <span id="version_val"></span></p>
</div>
</body>
</html>
)=====";


const char UPDATE_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head><meta name="viewport" content="width=device-width, initial-scale=1">
<title>MAX7219 RDA - Firmware Update</title>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  text-align: center;
  background-color: #2f3136;
}
#firmware_container {
  border-radius: 8px;
  color: black;
  width: 90%;
  height: 100%;
  margin-left: auto;
  margin-right: auto;
  margin-top: 20px;
  border: solid 2px;
  padding: 10px;
  background-color: #914f41;
  padding: 10px;
  position: centre;
  text-align: center;
}
#clientid_container {
  font-family: Arial;
  font-size: 14px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: white;
}
#version_container {
  font-family: Arial;
  font-size: 12px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: #ffff00;
}
label {
  display: block;
  padding-left: 0px;
  text-indent: 0px;
  font-family: Arial;
  font-size: 14px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: black;
}
input {
  text-align: center;
  width: 200px;
  padding: 5px;
  border: 2px solid #ccc;
  border-radius: 8px;
  box-sizing: border-box;
  margin-top: 2px;
  margin-bottom: 10px;
}
input.small-field {
  width: 100px;
}
input.msg-field {
  width: 70%;
}
input[type=file] {
  background-color: grey;
  color: black;
  width: 600px;
}
input[type=submit] {
  background-color: #04AA6D;
  color: white;
  width: 150px;
}
input[type=submit]:hover {
  background-color: #32ba39;
  width: 150px;
}
input[type=submit]:active {
  background-color: #32ba39;
  transform: translateY(4px);
}
.pill-nav a {
  display: inline-block;
  color: #eee;
  border: 2px solid #ccc;
  text-align: center;
  padding: 14px;
  text-decoration: none;
  font-size: 15px;
  border-radius: 5px;
  background-color: #555;
  width: 150px;
  margin-bottom: 10px;
  margin-right: 5px;
  margin-left: 5px;
}
.pill-nav a:hover {
  background-color: #ddd;
  color: black;
}
.pill-nav a.active {
  background-color: #9f9fa6;
  color: red;
}
.pill-nav a.firmware {
  color: #ff3347;
}
.pill-nav a.firmware:hover {
  color: #ff3347;
}
</style>
<script>
function getData() {
  var parser, xmlDoc;
  var request = new XMLHttpRequest();
  parser = new DOMParser();
  request.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      xmlDoc = parser.parseFromString(this.responseText,"text/xml");
      document.getElementById("clientid_val").innerHTML = 
      xmlDoc.getElementsByTagName("clientid")[0].childNodes[0].nodeValue;
      document.getElementById("version_val").innerHTML = 
      xmlDoc.getElementsByTagName("version")[0].childNodes[0].nodeValue;
	}
  };
  request.open("GET", "updatevars", true);
  request.send();
}
getData();
</script>
</head>
<body>
<H1><b>MAX7219 RDA - Firmware Update</b></H1>
<div id="firmware_container">
<form method='POST' action='/submitupdate' enctype='multipart/form-data'>
<input type='file' name='update'>
<br><br>
<input type='submit' value='Update'>
</form>
</div>
<br>
<div class="pill-nav">
  <a href="/"> Home</a>
  <a href="/changemqttconfig"> MQTT Configuration</a>
  <a href="/changeuserpass">Web Credentials</a>
  <a class="active" class="firmware" href="/update">Update Firmware</a>
</div>
<div id="clientid_container">
<p>Hostname: <span id="clientid_val"></span></p>
</div>
<div id="version_container">
<p>Firmware Version: <span id="version_val"></span></p>
</div>
</body>
</html>
)=====";


const char CHANGECREDENTIALS_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head><meta name="viewport" content="width=device-width, initial-scale=1">
<title>MAX7219 RDA - User and Password Change</title>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  text-align: center;
  background-color: #2f3136;
}
#userpassword_container {
  border-radius: 8px;
  color: black;
  width: 90%;
  height: 100%;
  margin-left: auto;
  margin-right: auto;
  margin-top: 20px;
  border: solid 2px;
  padding: 10px;
  background-color: #9f9fa6;
  padding: 10px;
  position: centre;
  text-align: center;
}
#firmware_container {
  color: black;
  width: 200px;
  height: 100%;
  margin-left: auto;
  margin-right: auto;
  margin-top: auto;
  margin-bottom: auto;
  border: solid 2px;
  padding-top: 10px;
  padding-bottom: 3px;
  background-color: #914f41;
}
#clientid_container {
  font-family: Arial;
  font-size: 14px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: white;
}
#version_container {
  font-family: Arial;
  font-size: 12px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: #ffff00;
}
label {
  display: block;
  padding-left: 0px;
  text-indent: 0px;
  font-family: Arial;
  font-size: 14px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: black;
}
input {
  text-align: center;
  width: 200px;
  padding: 5px;
  border: 2px solid #ccc;
  border-radius: 8px;
  box-sizing: border-box;
  margin-top: 2px;
  margin-bottom: 10px;
}
input.small-field {
  width: 100px;
}
input.msg-field {
  width: 70%;
}
input[type=submit] {
  background-color: #04AA6D;
  color: white;
  width: 150px;
  box-shadow: 0 5px #666;
}
input[type=submit]:hover {
  background-color: #32ba39;
  width: 150px;
  box-shadow: 0 5px #666;
}
input[type=submit]:active {
  background-color: #32ba39;
  box-shadow: 0 5px #666;
  transform: translateY(4px);
}
.pill-nav a {
  display: inline-block;
  color: #eee;
  border: 2px solid #ccc;
  text-align: center;
  padding: 14px;
  text-decoration: none;
  font-size: 15px;
  border-radius: 5px;
  background-color: #555;
  width: 150px;
  margin-bottom: 10px;
  margin-right: 5px;
  margin-left: 5px;
}
.pill-nav a:hover {
  background-color: #ddd;
  color: black;
}
.pill-nav a.active {
  background-color: #9f9fa6;
  color: black;
}
.pill-nav a.firmware {
  color: #ff3347;
}
.pill-nav a.firmware:hover {
  color: #ff3347;
}
</style>
<script>
function getData() {
  var parser, xmlDoc;
  var request = new XMLHttpRequest();
  parser = new DOMParser();
  request.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      xmlDoc = parser.parseFromString(this.responseText,"text/xml");
      document.getElementById("cred_change_form").username.value = 
      xmlDoc.getElementsByTagName("username")[0].childNodes[0].nodeValue;
      document.getElementById("clientid_val").innerHTML = 
      xmlDoc.getElementsByTagName("clientid")[0].childNodes[0].nodeValue;
      document.getElementById("version_val").innerHTML = 
      xmlDoc.getElementsByTagName("version")[0].childNodes[0].nodeValue;
	}
  };
  request.open("GET", "changecredvars", true);
  request.send();
}
getData();
</script>
</head>
<body>
<H1><b>MAX7219 RDA - Change Credentials</b></H1> 
<div id="userpassword_container"> 
<form id="cred_change_form" method="post" action="changecredentials">
<label for="Username">Username: </label>
<input type="text" id="username" placeholder="Enter Username" name="Username" required><br><br>
<label for="Password">Password: </label>
<input type="password" id="password" placeholder="Enter Password" name="Password" required><br><br>
<input type="submit" value="Save Changes">
</form> 
</div>
</br>
<div class="pill-nav">
  <a href="/"> Home</a>
  <a href="/changemqttconfig"> MQTT Configuration</a>
  <a class="active" href="/changeuserpass">Web Credentials</a>
  <a class="firmware" href="/update">Update Firmware</a>
</div>
<div id="clientid_container">
<p>Hostname: <span id="clientid_val"></span></p>
</div>
<div id="version_container">
<p>Firmware Version: <span id="version_val"></span></p>
</div>
</body>
</html>
)=====";



const char CHANGEMQTTCONFIG_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head><meta name="viewport" content="width=device-width, initial-scale=1">
<title>MAX7219 RDA - MQTT Config Change</title>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
#mqttconfig_container {
  border-radius: 8px;
  color: black;
  width: 90%;
  height: 100%;
  margin-left: auto;
  margin-right: auto;
  margin-bottom: 20px;
  border: solid 2px;
  padding: 10px;
  background-color: #9f9fa6;
  padding: 10px;
  position: centre;
  text-align: center;
}
#clientid_container {
  font-family: Arial;
  font-size: 14px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: white;
}
#version_container {
  font-family: Arial;
  font-size: 12px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: #ffff00;
}
label {
  display: block;
  padding-left: 0px;
  text-indent: 0px;
  font-family: Arial;
  font-size: 14px;
  font-style: normal;
  font-weight: normal;
  text-decoration: none;
  text-transform: none;
  color: black;
}
.checkbox_inliner {
  display: inline;
  color: black;
  width: 13px;
  height: 13px;
  margin-left: 1px;
  margin-right: 1px;
  margin-top: 20px;
  padding: 40px;
  background-color: #9f9fa6;
  margin-top: 20px;
  margin-bottom: 0px;
}
.label_inliner {
  display: inline;
  color: black;
  width: 13px;
  height: 13px;
  margin-left: 1px;
  margin-right: 1px;
  padding: 20px;
}
input {
  text-align: center;
  width: 200px;
  padding: 5px;
  border: 2px solid #ccc;
  border-radius: 8px;
  box-sizing: border-box;
  margin-top: 2px;
  margin-bottom: 10px;
}
input[type=submit] {
  background-color: #04AA6D;
  color: white;
  width: 150px;
  box-shadow: 0 5px #666;
}
input[type=submit]:hover {
  background-color: #32ba39;
  width: 150px;
  box-shadow: 0 5px #666;
}
input[type=submit]:active {
  background-color: #32ba39;
  box-shadow: 0 5px #666;
  transform: translateY(4px);
}
.pill-nav a {
  display: inline-block;
  color: #eee;
  border: 2px solid #ccc;
  text-align: center;
  padding: 14px;
  text-decoration: none;
  font-size: 15px;
  border-radius: 5px;
  background-color: #555;
  width: 150px;
  margin-bottom: 10px;
  margin-right: 5px;
  margin-left: 5px;
}
.pill-nav a:hover {
  background-color: #ddd;
  color: black;
}
.pill-nav a.active {
  background-color: #9f9fa6;
  color: black;
}
.pill-nav a.firmware {
  color: #ff3347;
}
.pill-nav a.firmware:hover {
  color: #ff3347;
}
</style>
<script>
function removeTrailingInvalidTopic(str) {
  str = str.replace(/\/*$/, '');
  str = str.replace(/\#.*$/, '#');
  str = str.replace(/([^\/])(\#$)/, '$1/#');
  str = str.replace(/([^\/])(\+$)/, '$1/+');
  return str;
}
function SubmitFunction() {
  if(document.getElementById("MQTTONOFF").checked) {
    document.getElementById("MQTTONOFFHIDDEN").disabled = true;
  };
  if(document.getElementById("MQTTANONYMOUS").checked) {
    document.getElementById("MQTTANONYMOUSHIDDEN").disabled = true;
  };
  if(document.getElementById("MQTTALERT").checked) {
    document.getElementById("MQTTALERTHIDDEN").disabled = true;
  };
  document.getElementById("txt_form").MQTTTOPICPREFIX.value = 
  removeTrailingInvalidTopic(document.getElementById("txt_form").MQTTTOPICPREFIX.value);
}
function getData() {
  var parser, xmlDoc;
  var request = new XMLHttpRequest();
  parser = new DOMParser();
  request.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      xmlDoc = parser.parseFromString(this.responseText,"text/xml");
      document.getElementById("clientid_val").innerHTML = 
      xmlDoc.getElementsByTagName("clientid")[0].childNodes[0].nodeValue;
      document.getElementById("version_val").innerHTML = 
      xmlDoc.getElementsByTagName("version")[0].childNodes[0].nodeValue;
      if (xmlDoc.getElementsByTagName("mqttonoff")[0].childNodes[0].nodeValue == "on"){
        document.getElementById("MQTTONOFF").checked = true;
      }
      else{
        document.getElementById("MQTTONOFF").checked = false;
      }
      if (xmlDoc.getElementsByTagName("mqttanonymous")[0].childNodes[0].nodeValue == "on"){
        document.getElementById("MQTTANONYMOUS").checked = true;
        document.getElementById("MQTTUSERNAME").disabled = true;
        document.getElementById("MQTTPASSWORD").disabled = true;
        document.getElementById("MQTTUSERNAME").value = "";
        document.getElementById("MQTTPASSWORD").value = "";
      }
      else{
        document.getElementById("MQTTANONYMOUS").checked = false;
        document.getElementById("MQTTUSERNAME").disabled = false;
        document.getElementById("MQTTPASSWORD").disabled = false;
        document.getElementById("txt_form").MQTTUSERNAME.value = 
        xmlDoc.getElementsByTagName("mqttusername")[0].childNodes[0].nodeValue;
      }
      if (xmlDoc.getElementsByTagName("mqttalert")[0].childNodes[0].nodeValue == "on"){
        document.getElementById("MQTTALERT").checked = true;
      }
      else{
        document.getElementById("MQTTALERT").checked = false;
      }
      document.getElementById("txt_form").MQTTSERVERADDRESS.value = 
      xmlDoc.getElementsByTagName("mqttserveraddress")[0].childNodes[0].nodeValue;
      document.getElementById("txt_form").MQTTSERVERPORT.value = 
      xmlDoc.getElementsByTagName("mqttserverport")[0].childNodes[0].nodeValue;
      document.getElementById("txt_form").MQTTTOPICPREFIX.value = 
      removeTrailingInvalidTopic(xmlDoc.getElementsByTagName("mqtttopicprefix")[0].childNodes[0].nodeValue);
    }
  };
  request.open("GET", "mqttpagevars", true);
  request.send();
}
getData();

function init() {
  var mqttAnonymousCheckbox = document.querySelector('input[id="MQTTANONYMOUS"]');
  
  mqttAnonymousCheckbox.addEventListener('change', () => {
    if(mqttAnonymousCheckbox.checked) {
      document.getElementById("MQTTUSERNAME").value = "";
      document.getElementById("MQTTPASSWORD").value = "";
      document.getElementById("MQTTUSERNAME").disabled = true;
      document.getElementById("MQTTPASSWORD").disabled = true;
    }
    else {
      document.getElementById("MQTTUSERNAME").disabled = false;
      document.getElementById("MQTTPASSWORD").disabled = false;
    }
  });
}
</script>
</head>
<body onload="init()">
<H1><b>MAX7219 RDA - MQTT Config Change</b></H1> 

<div id="mqttconfig_container"> 
<form id="txt_form" name="frmText" method="post" action="applymqttconfig"></br>
<label class="label_inliner" for="MQTTONOFF">Enable MQTT</label><input class="checkbox_inliner" type="checkbox" id="MQTTONOFF" name="MQTTONOFF"></br>
<input type="hidden" id="MQTTONOFFHIDDEN" name="MQTTONOFF" value="off">
<label class="label_inliner" for="MQTTANONYMOUS">Anonymous Mode</label><input class="checkbox_inliner" type="checkbox" id="MQTTANONYMOUS" name="MQTTANONYMOUS"></br>
<input type="hidden" id="MQTTANONYMOUSHIDDEN" name="MQTTANONYMOUS" value="off">
<label class="label_inliner" for="MQTTALERT">Connect Alert</label><input class="checkbox_inliner" type="checkbox" id="MQTTALERT" name="MQTTALERT"></br></br>
<input type="hidden" id="MQTTALERTHIDDEN" name="MQTTALERT" value="off">
<label>Username</label><input type="text" placeholder="Empty Not Updated" id="MQTTUSERNAME" name="MQTTUSERNAME">
<label>Password</label><input type="password" placeholder="Empty Not Updated" id="MQTTPASSWORD" name="MQTTPASSWORD">
<label>Server Address</label><input type="text" placeholder="Empty Not Updated" id="MQTTSERVERADDRESS" name="MQTTSERVERADDRESS">
<label>Server Port</label><input type="text" placeholder="Empty Not Updated" id="MQTTSERVERPORT" name="MQTTSERVERPORT">
<label>Topic Prefix</label><input type="text" placeholder="Empty Not Updated" id="MQTTTOPICPREFIX" name="MQTTTOPICPREFIX">
<br>
<br>
<input type="submit" value="Save Changes" onclick="SubmitFunction()">
</form>
</div>
<div class="pill-nav">
  <a href="/">Home</a>
  <a class="active" href="">MQTT Configuration</a>
  <a href="/changeuserpass">Web Credentials</a>
  <a class="firmware" href="/update">Update Firmware</a>
</div>
<div id="clientid_container">
<p>Hostname: <span id="clientid_val"></span></p>
</div>
<div id="version_container">
<p>Firmware Version: <span id="version_val"></span></p>
</div>
</body>
</html>
)=====";

const char APPLYUSERPASS_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
</style>
<head>
<script>
 setTimeout(function(){
	window.location.href = 'changeuserpass';
 }, 5000);
</script>
</head>
<body>
<p>HTTP Username and Password Updated!</p>
<p>Redirecting back to previous page in 5 seconds.</p>
</body>
</html>
)=====";

const char APPLYMQTTCONFIG_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
</style>
<head>
<script>
 setTimeout(function(){
	window.location.href = 'changemqttconfig';
 }, 5000);
</script>
</head>
<body>
<p>MQTT Config updated!</p>
<p>Redirecting back to previous page in 5 seconds.</p>
</body>
</html>
)=====";

const char SUBMITUPDATEFAIL_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
</style>
<head>
<script>
 setTimeout(function(){
	window.location.href = 'update';
 }, 5000);
</script>
</head>
<body>
<p>Firmware Update Failed!</p>
<p>Redirecting back to previous page in 5 seconds.</p>
</body>
</html>
)=====";

const char SUBMITUPDATEOK_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
</style>
<head>
<script>
 setTimeout(function(){
	window.location.href = 'update';
 }, 5000);
</script>
</head>
<body>
<p>Firmware Update OK!</p>
<p>Redirecting back to previous page in 5 seconds.</p>
</body>
</html>
)=====";

const char SUBMITUPDATESUCCESS_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
</style>
<head>
<script>
 setTimeout(function(){
	window.location.href = '/';
 }, 30000);
</script>
</head>
<body>
<p>Firmware Update Completed Successfully!</p>
<p>Device Rebooting Now!</p>
</br>
<p>Redirecting back to previous page in 30 seconds.</p>
</body>
</html>
)=====";

const char REBOOT_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
</style>
<head>
<script>
 setTimeout(function(){
	window.location.href = '/';
 }, 20000);
</script>
</head>
<body>
<p>Device Rebooting Now!</p>
</br>
<p>Redirecting back to previous page in 20 seconds.</p>
</body>
</html>
)=====";

const char FACTORYRESET_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<style>
html, body {
  color: #ebebed;
  font-family: Helvetica;
  display: block;
  margin: 0px auto;
  text-align: center;
  background-color: #2f3136;
}
</style>
<body>
<p>This device has been factory reset!</p>
<p>You'll need to setup wifi through the portal to use the device now.</p>
</br>
<p>Device Rebooting...</p>
</body>
</html>
)=====";


