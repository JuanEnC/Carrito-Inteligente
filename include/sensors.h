#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "config.h"
#include <TinyGPS++.h>

class SensorsControl
{
public:
    SensorsControl();
    void begin();
    uint8_t readIRArray();
    float getDistance();
    void updateGPS();
    double getDistanceTo(double targetLat, double targetLng);
    double getCourseTo(double targetLat, double targetLng);
    bool getGPSLocation(double &latitude, double &longitude);
private:
    TinyGPSPlus gps;
    long duration;
    float distanceCm;
};
extern SensorsControl Sensors;

#endif // SENSORS_H
