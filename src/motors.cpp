#include "Motors.h"
#include "Feedback.h" // Importamos Feedback para poder encender las luces rojas al frenar

// ==========================================
// DEFINICIÓN DE CANALES PWM (PlatformIO / Core v2.x)
// ==========================================
const int pwmChannelL1 = 0;
const int pwmChannelL2 = 1;
const int pwmChannelR1 = 2;
const int pwmChannelR2 = 3;

// ==========================================
// VARIABLES DE ENCODERS (Globales y Volátiles)
// ==========================================
volatile unsigned long pulsesL = 0;
volatile unsigned long pulsesR = 0;

// Rutinas de interrupción (ISRs) súper rápidas almacenadas en la RAM
void IRAM_ATTR countEncoderL() {
    pulsesL++;
}

void IRAM_ATTR countEncoderR() {
    pulsesR++;
}

// Implementación del constructor vacío
MotorsControl::MotorsControl() {}

void MotorsControl::begin() {
    // ==========================================
    // 1. CONFIGURACIÓN PWM (Sintaxis PlatformIO v2.x)
    // ==========================================
    // Configurar los canales
    ledcSetup(pwmChannelL1, pwmFreq, pwmResolution);
    ledcSetup(pwmChannelL2, pwmFreq, pwmResolution);
    ledcSetup(pwmChannelR1, pwmFreq, pwmResolution);
    ledcSetup(pwmChannelR2, pwmFreq, pwmResolution);

    // Vincular canales a pines
    ledcAttachPin(MOTOR_L_IN1, pwmChannelL1);
    ledcAttachPin(MOTOR_L_IN2, pwmChannelL2);
    ledcAttachPin(MOTOR_R_IN3, pwmChannelR1);
    ledcAttachPin(MOTOR_R_IN4, pwmChannelR2);

    // ==========================================
    // 2. CONFIGURACIÓN DE ENCODERS DE VELOCIDAD
    // ==========================================
    pinMode(ENCODER_L_PIN, INPUT_PULLUP);
    pinMode(ENCODER_R_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENCODER_L_PIN), countEncoderL, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_R_PIN), countEncoderR, RISING);

    // 3. Asegurar que los motores inicien apagados
    stop();
}

void MotorsControl::setSpeeds(int leftSpeed, int rightSpeed) {
    // Restringir los valores de velocidad entre -255 y 255 por seguridad
    leftSpeed = constrain(leftSpeed, -255, 255);
    rightSpeed = constrain(rightSpeed, -255, 255);

    // ==========================================
    // LÓGICA DE SINCRONIZACIÓN (CONTROL PROPORCIONAL)
    // ==========================================
    if (leftSpeed > 0 && rightSpeed > 0 && leftSpeed == rightSpeed) {
        
        int diferencia = pulsesL - pulsesR;
        float kp = 1.2; 
        int compensacion = diferencia * kp;

        leftSpeed -= compensacion;
        rightSpeed += compensacion;
        
        leftSpeed = constrain(leftSpeed, 0, 255);
        rightSpeed = constrain(rightSpeed, 0, 255);
        
    } else if (leftSpeed == 0 && rightSpeed == 0) {
        pulsesL = 0;
        pulsesR = 0;
    }

    if (leftSpeed != 0 || rightSpeed != 0) {
        Feedback.setBrakeLights(false);
    }

    // ==========================================
    // CONTROL MOTOR IZQUIERDO (Usando Canales v2.x)
    // ==========================================
    if (leftSpeed > 0) {
        ledcWrite(pwmChannelL1, leftSpeed);
        ledcWrite(pwmChannelL2, 0);
    } else if (leftSpeed < 0) {
        ledcWrite(pwmChannelL1, 0);
        ledcWrite(pwmChannelL2, abs(leftSpeed));
    } else {
        ledcWrite(pwmChannelL1, 0);
        ledcWrite(pwmChannelL2, 0);
    }

    // ==========================================
    // CONTROL MOTOR DERECHO (Usando Canales v2.x)
    // ==========================================
    if (rightSpeed > 0) {
        ledcWrite(pwmChannelR1, rightSpeed);
        ledcWrite(pwmChannelR2, 0);
    } else if (rightSpeed < 0) {
        ledcWrite(pwmChannelR1, 0);
        ledcWrite(pwmChannelR2, abs(rightSpeed));
    } else {
        ledcWrite(pwmChannelR1, 0);
        ledcWrite(pwmChannelR2, 0);
    }
}

void MotorsControl::drive(MoveDirection dir, int speed) {
    int slowSpeed = speed / 2;

    switch (dir) {
        case DIR_FORWARD:
            setSpeeds(speed, speed);
            break;
        case DIR_BACKWARD:
            setSpeeds(-speed, -speed);
            break;
        case DIR_LEFT: 
            setSpeeds(-speed, speed);
            break;
        case DIR_RIGHT: 
            setSpeeds(speed, -speed);
            break;
        case DIR_FORWARD_LEFT:
            setSpeeds(slowSpeed, speed);
            break;
        case DIR_FORWARD_RIGHT:
            setSpeeds(speed, slowSpeed);
            break;
        case DIR_BACKWARD_LEFT:
            setSpeeds(-slowSpeed, -speed);
            break;
        case DIR_BACKWARD_RIGHT:
            setSpeeds(-speed, -slowSpeed);
            break;
        case DIR_STOP:
        default:
            stop();
            break;
    }
}

void MotorsControl::stop() {
    setSpeeds(0, 0);
    Feedback.setBrakeLights(true);
}