#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// CONFIGURACIÓN DE RED (MODO 3)
// ==========================================
#define WIFI_SSID "Mario"
#define WIFI_PASSWORD "smartcar"
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "SmartCar_ESP32_01"

// ==========================================
// PINES DE MOTORES (DRV-8833)
// ==========================================
// Motor Izquierdo
#define MOTOR_L_IN1 26 
#define MOTOR_L_IN2 27
// Motor Derecho (⚠️ CORREGIR si usas pines de strapping)
#define MOTOR_R_IN3 14
#define MOTOR_R_IN4 33 // Cambiado del 12 a 33 para evitar problemas de strapping

// ==========================================
// ENCODERS DE VELOCIDAD (Directos al ESP-32)
// ==========================================
#define ENCODER_L_PIN 19
#define ENCODER_R_PIN 23

// ==========================================
// PINES DE SENSORES
// ==========================================
#define TRIG_PIN 5
#define ECHO_PIN 18
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUDRATE 9600
// Matriz Infrarroja TCRT5000 - 8 vías (Modo 1)
// Ajusta los pines a GPIOS seguros!
const uint8_t IR_PINS[8] = {36, 39, 26, 27, 14, 32, 25, 4};

// ==========================================
// EXPANSOR I2C (PCF8574)
// ==========================================
#define I2C_SDA 21
#define I2C_SCL 22
#define PCF_ADDRESS 0x20
#define PCF_LED_ROJO_L  0
#define PCF_LED_ROJO_R  1
#define PCF_LED_AMBAR_L 2
#define PCF_LED_AMBAR_R 3
#define PCF_LED_RGB_R   4 
#define PCF_LED_RGB_G   5 
#define PCF_LED_RGB_B   6 

// ==========================================
// FEEDBACK DIRECTO (ESP-32)
// ==========================================
#define BUZZER_PIN 13

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

#endif // CONFIG_H
