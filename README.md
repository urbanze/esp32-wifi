# ESP32 IDF WiFi library
All functions have comments and are inspired in the Arduino Core.

## Simple example to connect in STATION
```
WF wifi;
wifi.sta_connect("home-wifi", "1234567890");
```

## Simple example to create AP
Password with less than 8 chars, will leave AP in OPEN MODE.
```
WF wifi;
wifi.ap_start("ESP32", "1234567890");
```
