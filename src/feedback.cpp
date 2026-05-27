#include "feedback.h"
#include <Wire.h>
#include "PCF8574.h"

// Inicializamos el objeto del expansor I2C con los datos de config.h
PCF8574 pcf8574(PCF_ADDRESS, I2C_SDA, I2C_SCL);

FeedbackControl::FeedbackControl()
{
    currentSignal = SIGNAL_OFF;
    hornActive = false;
    blinkState = false;
    previousMillisBlink = 0;
    playingMelody = false;
    melodyStep = 0;
    previousMillisMelody = 0;
}

void FeedbackControl::begin()
{
    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);
    pcf8574.pinMode(PCF_LED_ROJO_L, OUTPUT);
    pcf8574.pinMode(PCF_LED_ROJO_R, OUTPUT);
    pcf8574.pinMode(PCF_LED_AMBAR_L, OUTPUT);
    pcf8574.pinMode(PCF_LED_AMBAR_R, OUTPUT);
    pcf8574.pinMode(PCF_LED_RGB_R, OUTPUT);
    pcf8574.pinMode(PCF_LED_RGB_G, OUTPUT);
    pcf8574.pinMode(PCF_LED_RGB_B, OUTPUT);
    pcf8574.begin();
    pcf8574.digitalWrite(PCF_LED_ROJO_L, LOW);
    pcf8574.digitalWrite(PCF_LED_ROJO_R, LOW);
    pcf8574.digitalWrite(PCF_LED_AMBAR_L, LOW);
    pcf8574.digitalWrite(PCF_LED_AMBAR_R, LOW);
    pcf8574.digitalWrite(PCF_LED_RGB_R, LOW);
    pcf8574.digitalWrite(PCF_LED_RGB_G, LOW);
    pcf8574.digitalWrite(PCF_LED_RGB_B, LOW);
    updateModeIndicator(MODO_ESPERA);
}

void FeedbackControl::update()
{
    unsigned long currentMillis = millis();
    if (currentSignal != SIGNAL_OFF)
    {
        if (currentMillis - previousMillisBlink >= blinkInterval)
        {
            previousMillisBlink = currentMillis;
            blinkState = !blinkState;
            if (currentSignal == SIGNAL_LEFT)
            {
                pcf8574.digitalWrite(PCF_LED_AMBAR_L, blinkState);
                pcf8574.digitalWrite(PCF_LED_AMBAR_R, LOW);
            }
            else if (currentSignal == SIGNAL_RIGHT)
            {
                pcf8574.digitalWrite(PCF_LED_AMBAR_R, blinkState);
                pcf8574.digitalWrite(PCF_LED_AMBAR_L, LOW);
            }
            else if (currentSignal == SIGNAL_HAZARD)
            {
                pcf8574.digitalWrite(PCF_LED_AMBAR_L, blinkState);
                pcf8574.digitalWrite(PCF_LED_AMBAR_R, blinkState);
            }
        }
    }
    if (playingMelody)
    {
        if (currentMillis - previousMillisMelody >= 150)
        {
            previousMillisMelody = currentMillis;
            int notes[] = {880, 1047, 1319};
            if (melodyStep < 3)
            {
                tone(BUZZER_PIN, notes[melodyStep]);
                melodyStep++;
            }
            else
            {
                noTone(BUZZER_PIN);
                playingMelody = false;
                if (hornActive)
                    tone(BUZZER_PIN, 1000);
            }
        }
    }
}

void FeedbackControl::setBrakeLights(bool state)
{
    pcf8574.digitalWrite(PCF_LED_ROJO_L, state ? HIGH : LOW);
    pcf8574.digitalWrite(PCF_LED_ROJO_R, state ? HIGH : LOW);
}

void FeedbackControl::setTurnSignal(SignalState state)
{
    currentSignal = state;
    if (currentSignal == SIGNAL_OFF)
    {
        pcf8574.digitalWrite(PCF_LED_AMBAR_L, LOW);
        pcf8574.digitalWrite(PCF_LED_AMBAR_R, LOW);
    }
    else
    {
        previousMillisBlink = millis();
        blinkState = HIGH;
        if (currentSignal == SIGNAL_LEFT)
            pcf8574.digitalWrite(PCF_LED_AMBAR_L, HIGH);
        if (currentSignal == SIGNAL_RIGHT)
            pcf8574.digitalWrite(PCF_LED_AMBAR_R, HIGH);
        if (currentSignal == SIGNAL_HAZARD)
        {
            pcf8574.digitalWrite(PCF_LED_AMBAR_L, HIGH);
            pcf8574.digitalWrite(PCF_LED_AMBAR_R, HIGH);
        }
    }
}

void FeedbackControl::setHorn(bool state)
{
    hornActive = state;
    if (hornActive)
    {
        tone(BUZZER_PIN, 1000);
    }
    else
    {
        if (!playingMelody)
            noTone(BUZZER_PIN);
    }
}

void FeedbackControl::playMelody()
{
    playingMelody = true;
    melodyStep = 0;
    previousMillisMelody = millis();
}

void FeedbackControl::updateModeIndicator(CarMode mode)
{
    switch (mode)
    {
    case MODO_ESPERA:
        setRGB(255, 255, 255);
        break;
    case MODO_SEGUIDOR_LINEA:
        setRGB(0, 255, 0);
        break;
    case MODO_EVASION:
        setRGB(0, 0, 255);
        break;
    case MODO_MQTT:
        setRGB(255, 255, 0);
        break;
    case MODO_GPS:
        setRGB(255, 0, 255);
        break;
    default:
        setRGB(0, 0, 0);
        break;
    }
}

void FeedbackControl::setRGB(int r, int g, int b)
{
    pcf8574.digitalWrite(PCF_LED_RGB_R, r > 0 ? HIGH : LOW);
    pcf8574.digitalWrite(PCF_LED_RGB_G, g > 0 ? HIGH : LOW);
    pcf8574.digitalWrite(PCF_LED_RGB_B, b > 0 ? HIGH : LOW);
}
