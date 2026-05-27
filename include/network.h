#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

class NetworkControl {
public:
    NetworkControl();
    void begin();
    void loop();
    bool publish(const char* topic, const char* payload);
    bool isConnected();
private:
    WiFiClient espClient;
    PubSubClient mqttClient;
    unsigned long lastReconnectAttempt;
    void setupWiFi();
    void reconnectMQTT();
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
};
extern NetworkControl Network;

#endif // NETWORK_H
