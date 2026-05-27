#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include "Config.h"

// Enumerador para mapear las 8 direcciones solicitadas para el control remoto (Modo 3)
enum MoveDirection {
    DIR_STOP = 0,
    DIR_FORWARD,          // Adelante
    DIR_BACKWARD,         // Atrás
    DIR_LEFT,             // Giro sobre su propio eje a la izquierda
    DIR_RIGHT,            // Giro sobre su propio eje a la derecha
    DIR_FORWARD_LEFT,     // Diagonal adelante-izquierda
    DIR_FORWARD_RIGHT,    // Diagonal adelante-derecha
    DIR_BACKWARD_LEFT,    // Diagonal atrás-izquierda
    DIR_BACKWARD_RIGHT    // Diagonal atrás-derecha
};

class MotorsControl {
public:
    // Constructor
    MotorsControl();

    // Inicializa los pines de los motores y configura el PWM del ESP32
    void begin();

    // Función base para establecer la velocidad de cada motor independientemente.
    // Recibe valores de -255 (reversa máxima) a 255 (avance máximo).
    void setSpeeds(int leftSpeed, int rightSpeed);

    // Mueve el vehículo en una de las 8 direcciones a una velocidad base dada
    void drive(MoveDirection dir, int speed = 255);

    // Detiene completamente los motores y enciende los LEDs rojos de freno
    void stop();

private:
    // Variables internas para canales PWM del ESP32 (evita bloqueos de hardware)
    const int pwmChannelL1 = 0;
    const int pwmChannelL2 = 1;
    const int pwmChannelR1 = 2;
    const int pwmChannelR2 = 3;
    
    // Frecuencia y resolución estándar para motores DC
    const int pwmFreq = 5000;
    const int pwmResolution = 8; 
};

// Declaración de una instancia global para poder usarla en otros archivos
extern MotorsControl Motors;

#endif // MOTORS_H