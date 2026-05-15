#ifndef MY_WIFI_H
#define MY_WIFI_H

#include <WiFi.h>

extern WiFiServer server;

void setupWiFi();
void wifiRequest();

#endif // MY_WIFI_H
