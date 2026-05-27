#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <Arduino.h>
#include "Config.h"

// Enumerador para gestionar los estados de las luces ámbar
enum SignalState
{
    SIGNAL_OFF = 0,
    SIGNAL_LEFT,  // Direccional izquierda
    SIGNAL_RIGHT, // Direccional derecha
    SIGNAL_HAZARD // Luces preventivas (ambas parpadean)
};

class FeedbackControl
{
public:
    // Constructor
    FeedbackControl();

    // Inicializa los pines de los LEDs y el Buzzer como salidas
    void begin();

    // Función crítica: debe llamarse constantemente en el loop() principal
    // Se encarga de hacer parpadear los LEDs y gestionar el buzzer sin usar delay()
    void update();

    // ==========================================
    // CONTROL DE LUCES
    // ==========================================
    // Enciende o apaga las luces traseras rojas.
    // Se llamará automáticamente desde la clase Motors cuando el carro se detenga.
    void setBrakeLights(bool state);

    // Establece qué luces ámbar deben parpadear (o apagarlas)
    void setTurnSignal(SignalState state);

    // ==========================================
    // CONTROL DE SONIDO
    // ==========================================
    // Activa el claxon de manera continua mientras sea 'true'
    void setHorn(bool state);

    // Reproduce una melodía corta predefinida (ideal para cuando el robot inicia o se conecta)
    void playMelody();

    // En la sección public de Feedback.h
    void updateModeIndicator(CarMode mode);

private:
    // Método interno para escribir los valores PWM
    void setRGB(int r, int g, int b);

private:
    SignalState currentSignal;
    bool hornActive;

    // Variables internas para el parpadeo "no bloqueante" de los LEDs
    unsigned long previousMillisBlink;
    const long blinkInterval = 500; // Medio segundo entre encendido y apagado
    bool blinkState;

    // Variables para el control de la melodía sin bloquear el código
    unsigned long previousMillisMelody;
    int melodyStep;
    bool playingMelody;
};

// Declaración de la instancia global
extern FeedbackControl Feedback;

#endif // FEEDBACK_H