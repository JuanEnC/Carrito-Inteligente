#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include "config.h"

// Enumerador de direcciones de movimiento
enum MoveDirection {
    DIR_STOP = 0,
    DIR_FORWARD,
    DIR_BACKWARD,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_FORWARD_LEFT,
    DIR_FORWARD_RIGHT,
    DIR_BACKWARD_LEFT,
    DIR_BACKWARD_RIGHT
};

class MotorsControl {
public:
    MotorsControl();
    void begin();
    void setSpeeds(int leftSpeed, int rightSpeed);
    void drive(MoveDirection dir, int speed = 255);
    void stop();
private:
    const int pwmFreq = 5000;
    const int pwmResolution = 8;
};

extern MotorsControl Motors;

#endif // MOTORS_H
