#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// CONFIGURACIÓN DE RED (MODO 3)
// ==========================================
#define WIFI_SSID "TU_RED_WIFI"
#define WIFI_PASSWORD "TU_PASSWORD"

#define MQTT_SERVER "broker.hivemq.com" // Broker público de prueba, cámbialo si usas uno propio
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "SmartCar_ESP32_01"

// ==========================================
// PINES DE MOTORES (DRV-8833)
// ==========================================
// Motor Izquierdo
#define MOTOR_L_IN1 26 
#define MOTOR_L_IN2 27
// Motor Derecho
#define MOTOR_R_IN3 14
#define MOTOR_R_IN4 12

// ==========================================
// PINES DE SENSORES
// ==========================================
// Sensor Ultrasónico HC-SR04 (Modo 2)
#define TRIG_PIN 5
#define ECHO_PIN 18

// Módulo GPS GY-NEO6MV2 (Modo 4)
// Usaremos el HardwareSerial 2 del ESP32
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUDRATE 9600

// Matriz Infrarroja TCRT5000 - 8 vías (Modo 1)
// Nota: 34, 35, 36, 39 son de solo entrada (ideales para sensores)
const uint8_t IR_PINS[8] = {34, 35, 36, 39, 32, 33, 25, 4};

// ==========================================
// PINES DE FEEDBACK (LUCES Y SONIDO)
// ==========================================
#define BUZZER_PIN 13 // Claxon

// LEDs (Ámbar y Rojo)
#define LED_ROJO_L 19 // Freno Izquierdo
#define LED_ROJO_R 21 // Freno Derecho
#define LED_AMBAR_L 22 // Direccional Izquierda
#define LED_AMBAR_R 23 // Direccional Derecha

// ==========================================
// ESTADOS DEL SISTEMA
// ==========================================
enum CarMode {
    MODO_ESPERA = 0,
    MODO_SEGUIDOR_LINEA = 1,
    MODO_EVASION = 2,
    MODO_MQTT = 3,
    MODO_GPS = 4
};

// LED RGB (Indicador de Modo)
#define LED_RGB_R 2
#define LED_RGB_G 0
#define LED_RGB_B 15

#endif // CONFIG_H