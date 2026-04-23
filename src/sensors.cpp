#include "Sensors.h"

// Implementación del constructor vacío
SensorsControl::SensorsControl() {
    duration = 0;
    distanceCm = 0.0;
}

void SensorsControl::begin() {
    // 1. Configuración del Sensor Ultrasónico (HC-SR04)
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW); // Aseguramos que inicie en bajo

    // 2. Configuración de la Matriz Infrarroja (TCRT5000 - 8 vías)
    // Recorremos el arreglo de pines que definimos en Config.h
    for (int i = 0; i < 8; i++) {
        pinMode(IR_PINS[i], INPUT);
    }

    // 3. Configuración del Módulo GPS (GY-NEO6MV2)
    // Utilizamos el HardwareSerial 2 del ESP32 para no interferir con el Serial (USB) de depuración.
    // SERIAL_8N1 es el formato estándar de comunicación (8 bits de datos, sin paridad, 1 bit de parada).
    Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println("Sensores inicializados (Ultrasónico, IR, GPS).");
}

uint8_t SensorsControl::readIRArray() {
    uint8_t sensorState = 0;

    // Leemos cada uno de los 8 sensores y construimos un byte usando operaciones a nivel de bits (bitwise).
    // Suponemos que el sensor devuelve HIGH (1) cuando detecta la línea negra y LOW (0) en blanco.
    // (Si tu pista es blanca y la línea es negra, puede que necesites invertir esta lógica dependiendo del módulo)
    for (int i = 0; i < 8; i++) {
        if (digitalRead(IR_PINS[i]) == HIGH) {
            // Desplaza un '1' a la posición 'i' y lo suma al estado general
            sensorState |= (1 << i);
        }
    }
    
    return sensorState;
}

float SensorsControl::getDistance() {
    // Generar un pulso limpio de 10 microsegundos en el pin TRIG
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Leer el tiempo que tarda el eco en regresar
    // IMPORTANTE: Le agregamos un timeout de 30,000 microsegundos (aprox 5 metros) 
    // para evitar que el ESP-32 se quede "congelado" esperando un rebote que nunca llega.
    duration = pulseIn(ECHO_PIN, HIGH, 30000); 

    // Si el duration es 0, significa que superó el timeout (no hay obstáculos cercanos)
    if (duration == 0) {
        return 999.0; 
    }

    // Calcular la distancia en centímetros (velocidad del sonido = 343 m/s)
    distanceCm = duration * 0.034 / 2.0;
    
    return distanceCm;
}

void SensorsControl::updateGPS() {
    // Esta función se llama constantemente en el loop() de main.cpp.
    // Mientras haya datos llegando por el RX del Serial2, se los pasamos a la librería TinyGPS++
    while (Serial2.available() > 0) {
        gps.encode(Serial2.read());
    }
}

bool SensorsControl::getGPSLocation(double &latitude, double &longitude) {
    // Verificamos si el GPS tiene una señal válida de los satélites
    if (gps.location.isValid()) {
        // Pasamos los valores por referencia a las variables que nos mande el main.cpp
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        return true;
    }
    
    // Si aún no hay señal (muy común al estar en interiores), devolvemos falso
    return false;
}

double SensorsControl::getDistanceTo(double targetLat, double targetLng) {
    if (gps.location.isValid()) {
        // Retorna la distancia en metros
        return TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), targetLat, targetLng);
    }
    return 99999.0; // Distancia falsa si no hay señal
}

double SensorsControl::getCourseTo(double targetLat, double targetLng) {
    if (gps.location.isValid()) {
        // Retorna el ángulo en grados hacia donde debe apuntar el vehículo
        return TinyGPSPlus::courseTo(gps.location.lat(), gps.location.lng(), targetLat, targetLng);
    }
    return 0.0;
}