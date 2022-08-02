esp8266_max7219_rda_msg_board
============================================================
MAX7219 RDA Message Board

This is a feature rich ESP8266 based message board and it's a considerable improvement over the various basic implementaions out there.

It has been mainly put togheter to display messages from remote systems (such as Home Assistant) using http or https as well.


Wifi Setup Mode on first start or reset (FLASH button press)
---------------------------------
You'll need to configure your wifi network by connecting to:
```
WiFi SSID: RDA-MSG-BOARD
Secret: wifi-setup
```
If the browser doesn't open automatically (it should on recent OSes) browse to http://192.168.4.1 and click on "Configure Wifi" and click "Save".

The board will reboot and should now boot in "Wifi Message Mode".

Connect to your wifi network and look for the IP the board obtained from DHCP (it should display it once at the end of the first message upon boot).

I suggest statically assigning an IP on your DHCP so the board always uses the same IP and can be referenced easily. (especially if used in automations)


Default username and password:
---------------------------------
```
username: admin
password: esp8266

```
***Please Note: Press the "FLASH" button on the ESP8266 to reset wifi and http/https credentials and reset the board (the "RST" button only restarts the board no changes)***


Key Features:
---------------------------------
* HTTP and HTTPs webserver and message baord web interface
* Ability to send messages via automation systems or scripts using url parameters or via json small api
* Parameter to specify numer of message scroll repeats
* Parameter to specify optional Buzzer sound with each start of message
* Parameter to specify delay in millisend to speed up or slow down scrolling of text
* Support for UTF8 Extended ASCII Characters (see https://www.utf8-chartable.de/)
* It uses the WifiManager library to provide a webpage to configure wifi SSID and Password when one hasn't been previously configured
* Change HTTP/HTTPs credentials and store them in config file (LittleFS)
* ESP8266 Reset (flash) button used to reset HTTP/HTTPs credentials as well as previously confiugred Wifi SSID


UTF8 Extended ASCII Characters
---------------------------------
Character you can display as arguments
```
!"$'()*,-./0123456789:<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£€¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ
```
Characters that can't always be send as part of a message:
```
#%&+;
```



Board Wiring
---------------------------------
![wiring](images/wiring.gif)



Web Interface
---------------------------------
![homepage](images/homepage.jpg)

![password_change](images/password_change.jpg)

![update_firmware](images/update_firmware.jpg)

Here you can upload the compiled binary to update the firmware



Project Case Example 1
---------------------------------
![example_a1](images/example_a1.jpg)

![example_a2](images/example_a2.jpg)



Project Case Example 2
---------------------------------
![example_b1](images/example_b1.jpg)

![example_b2](images/example_b2.jpg)

![example_b3](images/example_b3.jpg)


URL Parameters:
---------------------------------
```
MSG -> Message to display on dot matrix
REP -> Number of times the message scrolls vertically across the dot matrix
BUZ -> Number of times the buzzer makes a sound (chirps) in repeated succession
DEL -> Delay in millisecond for each scrolling step
ASC -> ASCII coversion to enable correct translation of UTF8 Extended ASCII Characters
```


Send Messages using curl:
---------------------------------
```
curl --user admin:esp8266 -X POST http://192.168.1.11/api -H 'Content-Type: application/json' -d '{"MSG":"This is a test message!","REP":"4","BUZ":"10","DEL":"30","ASC":"1"}'
```

```
curl --user admin:esp8266 -X GET -G -s -o /dev/null 'http://192.168.1.11/arg' --data-urlencode "MSG=This is a test message!" --data-urlencode "REP=4" --data-urlencode "BUZ=10" --data-urlencode "DEL=30" --data-urlencode "ASC=1"
```

```
curl --user admin:esp8266 -X GET -G -s -o /dev/null 'http://192.168.1.11/arg?MSG=This+is+a+test+message%21&REP=1&BUZ=3&DEL=30&ASC=1'
```
see https://www.url-encode-decode.com/


Send messages from Home Assistant:
---------------------------------
***Please Note: you'll have to use the base64 encoded as the username:password***

you should configure the following in your secrets.yaml file if you are using default credentials:

```
dot_matrix_secret_header: "Basic YWRtaW46ZXNwODI2Ng=="

````
for any other you can calculate your own for example enter admin:esp8266 in http://n-cg.net/base64.htm and click encode to obtain YWRtaW46ZXNwODI2Ng==


You can for example define your own home assistant lovelace interface to send test to the message board by following these steps:

![home_assistant_gui_pannel](images/home_assistant_gui_pannel.jpg)

1. Create the relevant custom entities from home assistant gui and enter this in your lovelace interface tab for example:

```
type: entities
entities:
  - entity: input_text.dot_matrix_text
  - entity: input_select.dot_matrix_device_list
  - entity: input_text.dot_matrix_ip
  - entity: input_number.dot_matrix_msg_repeat
  - entity: input_number.dot_matrix_buzzer
  - entity: input_number.dot_matrix_scroll_delay
  - entity: script.message_dot_matrix
  - entity: script.clear_dot_matrix
title: Message Boards Texting (MAX7219)
```


2. Enter this in configuration.yaml:
```
rest_command:
  message_dot_matrix_arg_http:
    url: "http://{{ states('input_text.dot_matrix_ip') }}/api"
    method: POST
    headers:
      authorization: !secret dot_matrix_secret_header
      content-type: "application/json"
    payload: "{MSG:'{{ states('input_text.dot_matrix_text') }}',REP:{{ states('input_number.dot_matrix_msg_repeat') }},BUZ:{{ states('input_number.dot_matrix_buzzer') }},DEL:{{ states('input_number.dot_matrix_scroll_delay') }},ASC:1}"
  message_dot_matrix_arg_https:
    url: "https://{{ states('input_text.dot_matrix_ip') }}/api"
    method: POST
    headers:
      authorization: !secret dot_matrix_secret_header
      content-type: "application/json"
    payload: "{MSG:'{{ states('input_text.dot_matrix_text') }}',REP:{{ states('input_number.dot_matrix_msg_repeat') }},BUZ:{{ states('input_number.dot_matrix_buzzer') }},DEL:{{ states('input_number.dot_matrix_scroll_delay') }},ASC:1}"
    verify_ssl: false
  clear_dot_matrix_arg_http:
    url: "http://{{ states('input_text.dot_matrix_ip') }}/arg?MSG=&REP={{ states('input_number.dot_matrix_msg_repeat') }}&BUZ={{ states('input_number.dot_matrix_buzzer') }}&DEL={{ states('input_number.dot_matrix_scroll_delay') }}&ASC=1"
    method: GET
    headers:
      authorization: !secret dot_matrix_secret_header
  clear_dot_matrix_arg_https:
    url: "https://{{ states('input_text.dot_matrix_ip') }}/arg?MSG=&REP={{ states('input_number.dot_matrix_msg_repeat') }}&BUZ={{ states('input_number.dot_matrix_buzzer') }}&DEL={{ states('input_number.dot_matrix_scroll_delay') }}&ASC=1"
    method: GET
    headers:
      authorization: !secret dot_matrix_secret_header
    verify_ssl: false
```


3. if you have multiple message boards you can create an automations to switch IP address from the input_select list entity:
```
alias: Change Value of Dot Matrix Input Text
description: ''
trigger:
  - platform: state
    entity_id: input_select.dot_matrix_device_list
condition: []
action:
  - service: input_text.set_value
    target:
      entity_id: input_text.dot_matrix_ip
    data_template:
      value: >
        {% if is_state('input_select.dot_matrix_device_list', 'Living Room') %}
          192.168.1.11
        {% elif is_state('input_select.dot_matrix_device_list', 'Home Office')
        %}
          192.168.1.12
        {% endif %}
mode: single 
```


4. Also create the following home assistant scripts:

script.message_dot_matrix:
```
sequence:
  - service: rest_command.message_dot_matrix_arg_http
    data: {}
mode: single
alias: Message Dot Matrix
```

script.clear_dot_matrix:
```
sequence:
  - service: rest_command.clear_dot_matrix_arg_http
    data: {}
mode: single
alias: Clear Dot Matrix
```




You can also use home assistant RSS Feeds to send news text to the message board:

1. enter some rss feeds like the below for example in your home assistant configuration.yaml
```
feedreader:
  urls:
    - http://feeds.bbci.co.uk/news/world/rss.xml
    - http://feeds.bbci.co.uk/news/rss.xml
    - http://feeds.bbci.co.uk/news/health/rss.xml
    - http://feeds.bbci.co.uk/news/technology/rss.xml
    - http://feeds.bbci.co.uk/news/science_and_environment/rss.xml
  scan_interval:
    minutes: 5
  max_entries: 5
```
and 

```
rest_command:
  ### some examples althought we'll use the first one for RSS feed automation below
  feed_to_dot_matrix_arg_http:
    url: "http://{{ipaddress}}/arg?MSG={{msg}}&REP={{rep}}&BUZ={{buz}}&DEL={{del}}&ASC=1"
    method: GET
    headers:
      authorization: !secret dot_matrix_secret_header
  feed_to_dot_matrix_arg_https:
    url: "https://{{ipaddress}}/arg?&MSG={{msg}}&REP={{rep}}&BUZ={{buz}}&DEL={{del}}&ASC=1"
    method: GET
    headers:
      authorization: !secret dot_matrix_secret_header
    verify_ssl: false
  feed_to_dot_matrix_api_http:
    url: "http://{{ipaddress}}/api"
    method: POST
    headers:
      authorization: !secret dot_matrix_secret_header
      content-type: "application/json"
    payload: "{MSG:'{{msg}}',REP:{{rep}},BUZ:{{buz}},DEL:{{del}},ASC:1}"
  feed_to_dot_matrix_api_https:
    url: "https://{{ipaddress}}/api"
    method: POST
    headers:
      authorization: !secret dot_matrix_secret_header
      content-type: "application/json"
    payload: "{MSG:'{{msg}}',REP:{{rep}},BUZ:{{buz}},DEL:{{del}},ASC:1}"
    verify_ssl: false
```

2. Then configuration an automation as follows if you have for example 2 different message boards and you want to send to both:
```
alias: Feed Reader To Matrix
description: ''
trigger:
  - platform: event
    event_type: feedreader
condition: []
action:
  - service: rest_command.feed_to_dot_matrix_api_http
    data:
      ipaddress: 192.168.1.11
      msg: >-
        News Feed: {{ trigger.event.data.title }}.... {{
        trigger.event.data.summary }}
      rep: 10
      buz: 5
      del: 30
  - service: rest_command.feed_to_dot_matrix_api_http
    data:
      ipaddress: 192.168.1.12
      msg: >-
        News Feed: {{ trigger.event.data.title }}.... {{
        trigger.event.data.summary }}
      rep: 10
      buz: 5
      del: 30
mode: single
```



Send messages from NodeRed
---------------------------------
Import file node_red_flow.json into nodered.

