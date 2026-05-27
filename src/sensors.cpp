#include "sensors.h"

SensorsControl::SensorsControl() {
    duration = 0;
    distanceCm = 0.0;
}

void SensorsControl::begin() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
    for (int i = 0; i < 8; i++) {
        pinMode(IR_PINS[i], INPUT);
    }
    Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println("Sensores inicializados (Ultrasónico, IR, GPS).");
}

uint8_t SensorsControl::readIRArray() {
    uint8_t sensorState = 0;
    for (int i = 0; i < 8; i++) {
        if (digitalRead(IR_PINS[i]) == HIGH) {
            sensorState |= (1 << i);
        }
    }
    return sensorState;
}

float SensorsControl::getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0) {
        return 999.0;
    }
    distanceCm = duration * 0.034 / 2.0;
    return distanceCm;
}

void SensorsControl::updateGPS() {
    while (Serial2.available() > 0) {
        gps.encode(Serial2.read());
    }
}

bool SensorsControl::getGPSLocation(double &latitude, double &longitude) {
    if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        return true;
    }
    return false;
}

double SensorsControl::getDistanceTo(double targetLat, double targetLng) {
    if (gps.location.isValid()) {
        return TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), targetLat, targetLng);
    }
    return 99999.0;
}

double SensorsControl::getCourseTo(double targetLat, double targetLng) {
    if (gps.location.isValid()) {
        return TinyGPSPlus::courseTo(gps.location.lat(), gps.location.lng(), targetLat, targetLng);
    }
    return 0.0;
}
