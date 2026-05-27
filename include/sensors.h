#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "Config.h"
#include <TinyGPS++.h> // Librería para el Módulo GPS GY-NEO6MV2

class SensorsControl
{
public:
    // Constructor
    SensorsControl();

    // Inicializa los pines de los sensores y el puerto serial de hardware para el GPS
    void begin();

    // ==========================================
    // MODO 1: Seguidor de Línea (TCRT5000)
    // ==========================================
    // Lee la matriz de 8 sensores infrarrojos.
    // Retorna un byte (uint8_t) donde cada bit (0 o 1) representa el estado de un sensor individual.
    // Esto facilita calcular el error para mantener el carro centrado en la línea.
    uint8_t readIRArray();

    // ==========================================
    // MODO 2: Evasión de Obstáculos (HC-SR04)
    // ==========================================
    // Dispara el pulso ultrasónico y calcula el rebote.
    // Retorna la distancia medida en centímetros.
    float getDistance();

    // ==========================================
    // MODO 4: Navegación GPS (GY-NEO6MV2)
    // ==========================================
    // Esta función debe llamarse constantemente dentro del loop() principal
    // para procesar los datos NMEA que llegan por el puerto Serial.
    void updateGPS();
    // Calcula la distancia en metros hacia el objetivo
    double getDistanceTo(double targetLat, double targetLng);

    // Calcula el ángulo (0 a 360 grados) hacia el objetivo
    double getCourseTo(double targetLat, double targetLng);

    // Devuelve por referencia la latitud y longitud actuales.
    // Retorna 'true' si el GPS tiene una ubicación válida fija, de lo contrario 'false'.
    bool getGPSLocation(double &latitude, double &longitude);

private:
    // Instancia del objeto TinyGPSPlus para procesar los datos del módulo
    TinyGPSPlus gps;

    // Variables internas para el cálculo de distancia
    long duration;
    float distanceCm;
};

// Declaración de la instancia global para acceder a los sensores desde cualquier archivo
extern SensorsControl Sensors;

#endif // SENSORS_H