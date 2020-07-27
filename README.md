# ESP32 IDF WiFi library
* All functions have comments and are inspired in the Arduino Core.
* WiFi STA does not automatically reconnect, it is necessary to do manually. (Example below)

## Simple example to connect in STATION (DHCP)
```
WF wifi;
wifi.sta_connect("home-wifi", "1234567890");
```

## Simple example to connect in STATION (Static IP)
```
WF wifi;
wifi.sta_static_ip("192.168.0.110", "192.168.0.1", "255.255.255.0");
wifi.sta_connect("home-wifi", "1234567890");
```

## Simple example to create AP
Password with less than 8 chars, will leave AP in OPEN MODE.
```
WF wifi;
wifi.ap_start("ESP32", "1234567890");
```

## Simple example to create STA + AP
Password with less than 8 chars, will leave AP in OPEN MODE.
Attention: Init STA first, reffers to issue #1. 
```
WF wifi;
wifi.sta_connect("home-wifi", "1234567890");
wifi.ap_start("ESP32", "1234567890");
```

## Simple example to reconnect in WiFi STA
```
WF wifi;
wifi.sta_connect("home-wifi", "1234567890");

int8_t status = wifi.sta_status();
if (status != SYSTEM_EVENT_STA_CONNECTED && status != SYSTEM_EVENT_STA_GOT_IP)
{
    wifi.sta_reconnect();
}
```
