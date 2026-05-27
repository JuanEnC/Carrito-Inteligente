#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Config.h"

class NetworkControl {
public:
    // Constructor
    NetworkControl();

    // Inicializa la conexión WiFi y configura el servidor del broker MQTT
    void begin();

    // Función crítica: debe llamarse constantemente en el loop() principal
    // Mantiene la conexión viva (Keep-Alive) y procesa los mensajes entrantes
    void loop();

    // Permite al ESP32 enviar datos (ej. coordenadas GPS o estado de la batería) a la app
    bool publish(const char* topic, const char* payload);

    // Verifica si el vehículo está conectado al broker
    bool isConnected();

private:
    WiFiClient espClient;
    PubSubClient mqttClient;

    // Temporizador para intentar reconexiones sin bloquear el resto del código (evasión/seguidor)
    unsigned long lastReconnectAttempt;

    // Métodos internos de conexión
    void setupWiFi();
    void reconnectMQTT();

    // Callback estático para manejar los mensajes que llegan desde la app (MQTT)
    // Se declara como estático porque la librería PubSubClient requiere un puntero a función estándar de C
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
};

// Declaración de la instancia global
extern NetworkControl Network;

#endif // NETWORK_H