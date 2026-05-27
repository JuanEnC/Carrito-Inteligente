#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <Arduino.h>
#include "config.h"

enum SignalState {
    SIGNAL_OFF = 0,
    SIGNAL_LEFT,
    SIGNAL_RIGHT,
    SIGNAL_HAZARD
};

class FeedbackControl
{
public:
    FeedbackControl();
    void begin();
    void update();
    void setBrakeLights(bool state);
    void setTurnSignal(SignalState state);
    void setHorn(bool state);
    void playMelody();
    void updateModeIndicator(CarMode mode);
private:
    void setRGB(int r, int g, int b);
    SignalState currentSignal;
    bool hornActive;
    unsigned long previousMillisBlink;
    const long blinkInterval = 500;
    bool blinkState;
    unsigned long previousMillisMelody;
    int melodyStep;
    bool playingMelody;
};
extern FeedbackControl Feedback;

#endif // FEEDBACK_H
