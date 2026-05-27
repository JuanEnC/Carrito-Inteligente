#include "network.h"
#include "motors.h"
#include "feedback.h"

extern CarMode currentMode;
extern void changeMode(CarMode newMode);
extern double targetLat;
extern double targetLng;
extern int currentSpeed;

NetworkControl::NetworkControl() : mqttClient(espClient)
{
    lastReconnectAttempt = 0;
}

void NetworkControl::begin()
{
    setupWiFi();
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
}

void NetworkControl::setupWiFi()
{
    Serial.print("\nConectando a WiFi: ");
    Serial.println(WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void NetworkControl::loop()
{
    if (WiFi.status() != WL_CONNECTED)
        return;
    if (!mqttClient.connected())
    {
        long now = millis();
        if (now - lastReconnectAttempt > 5000)
        {
            lastReconnectAttempt = now;
            reconnectMQTT();
        }
    }
    else
    {
        mqttClient.loop();
    }
}

void NetworkControl::reconnectMQTT()
{
    Serial.print("Intentando conexión MQTT...");
    if (mqttClient.connect(MQTT_CLIENT_ID))
    {
        Serial.println(" ¡Conectado!");
        mqttClient.subscribe("smartcar/control/modo");
        mqttClient.subscribe("smartcar/control/mover");
        mqttClient.subscribe("smartcar/control/feedback");
        mqttClient.subscribe("smartcar/control/gps");
        mqttClient.subscribe("smartcar/control/velocidad");
        publish("smartcar/status", "ONLINE");
        Feedback.playMelody();
    }
    else
    {
        Serial.print(" Falló, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" - Reintentando en 5 segundos");
    }
}

bool NetworkControl::publish(const char *topic, const char *payload)
{
    if (mqttClient.connected())
        return mqttClient.publish(topic, payload);
    return false;
}

bool NetworkControl::isConnected()
{
    return WiFi.status() == WL_CONNECTED && mqttClient.connected();
}

void NetworkControl::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String message = "";
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    Serial.print("Mensaje recibido [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);

    if (String(topic) == "smartcar/control/modo")
    {
        int mode = message.toInt();
        if (mode >= 0 && mode <= 4)
            changeMode((CarMode)mode);
    }
    else if (String(topic) == "smartcar/control/mover")
    {
        if (message == "ADELANTE")
            Motors.drive(DIR_FORWARD, currentSpeed);
        else if (message == "ATRAS")
            Motors.drive(DIR_BACKWARD, currentSpeed);
        else if (message == "IZQ")
            Motors.drive(DIR_LEFT, currentSpeed);
        else if (message == "DER")
            Motors.drive(DIR_RIGHT, currentSpeed);
        else if (message == "DIAG_AD_IZQ")
            Motors.drive(DIR_FORWARD_LEFT, currentSpeed);
        else if (message == "DIAG_AD_DER")
            Motors.drive(DIR_FORWARD_RIGHT, currentSpeed);
        else if (message == "DIAG_AT_IZQ")
            Motors.drive(DIR_BACKWARD_LEFT, currentSpeed);
        else if (message == "DIAG_AT_DER")
            Motors.drive(DIR_BACKWARD_RIGHT, currentSpeed);
        else if (message == "STOP")
            Motors.stop();
    }
    else if (String(topic) == "smartcar/control/feedback")
    {
        if (message == "CLAXON_ON")
            Feedback.setHorn(true);
        else if (message == "CLAXON_OFF")
            Feedback.setHorn(false);
        else if (message == "PREVENTIVAS")
            Feedback.setTurnSignal(SIGNAL_HAZARD);
        else if (message == "DIR_IZQ")
            Feedback.setTurnSignal(SIGNAL_LEFT);
        else if (message == "DIR_DER")
            Feedback.setTurnSignal(SIGNAL_RIGHT);
        else if (message == "LUCES_OFF")
            Feedback.setTurnSignal(SIGNAL_OFF);
    }
    else if (String(topic) == "smartcar/control/gps")
    {
        int commaIndex = message.indexOf(',');
        if (commaIndex > 0)
        {
            String latStr = message.substring(0, commaIndex);
            String lngStr = message.substring(commaIndex + 1);
            targetLat = latStr.toDouble();
            targetLng = lngStr.toDouble();
            Serial.print("📍 Nuevo destino recibido: ");
            Serial.print(targetLat, 6);
            Serial.print(", ");
            Serial.println(targetLng, 6);
        }
    }
    else if (String(topic) == "smartcar/control/velocidad")
    {
        int newSpeed = message.toInt();
        currentSpeed = constrain(newSpeed, 0, 255);
        Serial.print("Velocidad actualizada a: ");
        Serial.println(currentSpeed);
    }
}
