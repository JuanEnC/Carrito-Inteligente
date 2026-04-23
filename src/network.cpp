#include "Network.h"
#include "Motors.h"
#include "Feedback.h"

// Importamos la variable global y la función que definimos en main.cpp
// Esto nos permite leer en qué modo estamos y cambiarlo desde la app
extern CarMode currentMode;
extern void changeMode(CarMode newMode);

extern double targetLat;
extern double targetLng;

// Constructor: Vinculamos el cliente MQTT con el cliente WiFi
NetworkControl::NetworkControl() : mqttClient(espClient)
{
    lastReconnectAttempt = 0;
}

void NetworkControl::begin()
{
    setupWiFi();
    // Configuramos el broker MQTT y asignamos la función que recibirá los mensajes
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
}

void NetworkControl::setupWiFi()
{
    Serial.print("\nConectando a WiFi: ");
    Serial.println(WIFI_SSID);

    // Iniciamos la conexión en segundo plano
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void NetworkControl::loop()
{
    // 1. Verificar si perdimos conexión WiFi y reconectar si es necesario
    if (WiFi.status() != WL_CONNECTED)
    {
        // Podríamos intentar reconectar aquí, pero la librería WiFi del ESP32
        // suele manejar la reconexión automáticamente si usamos WiFi.begin() al inicio.
        return;
    }

    // 2. Verificar si estamos conectados al broker MQTT
    if (!mqttClient.connected())
    {
        long now = millis();
        // Intentar reconectar cada 5 segundos sin usar delay()
        if (now - lastReconnectAttempt > 5000)
        {
            lastReconnectAttempt = now;
            reconnectMQTT();
        }
    }
    else
    {
        // 3. Mantener la comunicación viva y procesar mensajes entrantes
        mqttClient.loop();
    }
}

void NetworkControl::reconnectMQTT()
{
    Serial.print("Intentando conexión MQTT...");

    // Intentar conectar con el ID de cliente definido en Config.h
    if (mqttClient.connect(MQTT_CLIENT_ID))
    {
        Serial.println(" ¡Conectado!");

        // Al conectar, nos suscribimos a los tópicos de control de la app
        mqttClient.subscribe("smartcar/control/modo");
        mqttClient.subscribe("smartcar/control/mover");
        mqttClient.subscribe("smartcar/control/feedback");

        // ¡NUEVA SUSCRIPCIÓN!
        mqttClient.subscribe("smartcar/control/gps");

        // Opcional: Avisar a la app que el carro está en línea
        publish("smartcar/status", "ONLINE");
        Feedback.playMelody(); // Un pequeño aviso sonoro de conexión exitosa
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
    {
        return mqttClient.publish(topic, payload);
    }
    return false;
}

bool NetworkControl::isConnected()
{
    return WiFi.status() == WL_CONNECTED && mqttClient.connected();
}

// ==========================================
// CALLBACK MQTT: Aquí llegan los comandos de la App
// ==========================================
void NetworkControl::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    // Convertir el payload (los datos brutos) a un String fácil de leer
    String message = "";
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.print("Mensaje recibido [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);

    // ------------------------------------------------
    // TÓPICO: CAMBIO DE MODO
    // ------------------------------------------------
    if (String(topic) == "smartcar/control/modo")
    {
        int mode = message.toInt();
        if (mode >= 0 && mode <= 4)
        {
            changeMode((CarMode)mode);
        }
    }

    // ------------------------------------------------
    // TÓPICO: MOVIMIENTO (Solo válido en MODO 3)
    // ------------------------------------------------
    else if (String(topic) == "smartcar/control/mover")
    {
        if (currentMode == MODO_MQTT)
        {
            if (message == "ADELANTE")
                Motors.drive(DIR_FORWARD);
            else if (message == "ATRAS")
                Motors.drive(DIR_BACKWARD);
            else if (message == "IZQ")
                Motors.drive(DIR_LEFT);
            else if (message == "DER")
                Motors.drive(DIR_RIGHT);
            else if (message == "DIAG_AD_IZQ")
                Motors.drive(DIR_FORWARD_LEFT);
            else if (message == "DIAG_AD_DER")
                Motors.drive(DIR_FORWARD_RIGHT);
            else if (message == "DIAG_AT_IZQ")
                Motors.drive(DIR_BACKWARD_LEFT);
            else if (message == "DIAG_AT_DER")
                Motors.drive(DIR_BACKWARD_RIGHT);
            else if (message == "STOP")
                Motors.drive(DIR_STOP);
        }
    }

    // ------------------------------------------------
    // TÓPICO: LUCES Y CLAXON (Válido en cualquier modo)
    // ------------------------------------------------
    else if (String(topic) == "smartcar/control/feedback")
    {
        // Claxon
        if (message == "CLAXON_ON")
            Feedback.setHorn(true);
        else if (message == "CLAXON_OFF")
            Feedback.setHorn(false);
        // Luces
        else if (message == "PREVENTIVAS")
            Feedback.setTurnSignal(SIGNAL_HAZARD);
        else if (message == "DIR_IZQ")
            Feedback.setTurnSignal(SIGNAL_LEFT);
        else if (message == "DIR_DER")
            Feedback.setTurnSignal(SIGNAL_RIGHT);
        else if (message == "LUCES_OFF")
            Feedback.setTurnSignal(SIGNAL_OFF);
    }
    // ------------------------------------------------
    // TÓPICO: GPS (Destino del mapa)
    // ------------------------------------------------
    else if (String(topic) == "smartcar/control/gps")
    {
        // El mensaje viene así: "19.2452,-103.7240"
        int commaIndex = message.indexOf(',');

        // Si encontramos la coma, separamos la cadena en dos partes
        if (commaIndex > 0)
        {
            String latStr = message.substring(0, commaIndex);
            String lngStr = message.substring(commaIndex + 1);

            // Convertimos el texto a números double y los guardamos en las variables globales
            targetLat = latStr.toDouble();
            targetLng = lngStr.toDouble();

            Serial.print("📍 Nuevo destino recibido: ");
            Serial.print(targetLat, 6);
            Serial.print(", ");
            Serial.println(targetLng, 6);
        }
    }   
}