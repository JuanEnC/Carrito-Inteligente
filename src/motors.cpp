#include "Motors.h"
#include "Feedback.h" // Importamos Feedback para poder encender las luces rojas al frenar

// Implementación del constructor vacío
MotorsControl::MotorsControl() {}

void MotorsControl::begin() {
    // 1. Configurar los canales PWM del ESP32 (Frecuencia y Resolución)
    ledcSetup(pwmChannelL1, pwmFreq, pwmResolution);
    ledcSetup(pwmChannelL2, pwmFreq, pwmResolution);
    ledcSetup(pwmChannelR1, pwmFreq, pwmResolution);
    ledcSetup(pwmChannelR2, pwmFreq, pwmResolution);

    // 2. Vincular los canales PWM a los pines físicos definidos en Config.h
    ledcAttachPin(MOTOR_L_IN1, pwmChannelL1);
    ledcAttachPin(MOTOR_L_IN2, pwmChannelL2);
    ledcAttachPin(MOTOR_R_IN3, pwmChannelR1);
    ledcAttachPin(MOTOR_R_IN4, pwmChannelR2);

    // 3. Asegurar que los motores inicien apagados
    stop();
}

void MotorsControl::setSpeeds(int leftSpeed, int rightSpeed) {
    // Restringir los valores de velocidad entre -255 y 255 por seguridad
    leftSpeed = constrain(leftSpeed, -255, 255);
    rightSpeed = constrain(rightSpeed, -255, 255);

    // Si hay movimiento, apagamos las luces de freno
    if (leftSpeed != 0 || rightSpeed != 0) {
        Feedback.setBrakeLights(false);
    }

    // ==========================================
    // CONTROL MOTOR IZQUIERDO
    // ==========================================
    if (leftSpeed > 0) {
        // Avance
        ledcWrite(pwmChannelL1, leftSpeed);
        ledcWrite(pwmChannelL2, 0);
    } else if (leftSpeed < 0) {
        // Reversa
        ledcWrite(pwmChannelL1, 0);
        ledcWrite(pwmChannelL2, abs(leftSpeed));
    } else {
        // Detenido
        ledcWrite(pwmChannelL1, 0);
        ledcWrite(pwmChannelL2, 0);
    }

    // ==========================================
    // CONTROL MOTOR DERECHO
    // ==========================================
    if (rightSpeed > 0) {
        // Avance
        ledcWrite(pwmChannelR1, rightSpeed);
        ledcWrite(pwmChannelR2, 0);
    } else if (rightSpeed < 0) {
        // Reversa
        ledcWrite(pwmChannelR1, 0);
        ledcWrite(pwmChannelR2, abs(rightSpeed));
    } else {
        // Detenido
        ledcWrite(pwmChannelR1, 0);
        ledcWrite(pwmChannelR2, 0);
    }
}

void MotorsControl::drive(MoveDirection dir, int speed) {
    // Calculamos una velocidad reducida para las diagonales (giros suaves)
    int slowSpeed = speed / 2;

    switch (dir) {
        case DIR_FORWARD:
            setSpeeds(speed, speed);
            break;
        case DIR_BACKWARD:
            setSpeeds(-speed, -speed);
            break;
        case DIR_LEFT: // Giro sobre su propio eje a la izquierda
            setSpeeds(-speed, speed);
            break;
        case DIR_RIGHT: // Giro sobre su propio eje a la derecha
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
    // Detener ambos motores usando la función base
    setSpeeds(0, 0);
    
    // Cumplir con el requerimiento de encender las luces rojas al frenar/detenerse
    Feedback.setBrakeLights(true);
}