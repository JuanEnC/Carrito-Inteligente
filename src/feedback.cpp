#include "Feedback.h"
#include "PCF8574.h" // Importamos la librería del expansor

// Inicializamos el objeto del expansor I2C con los datos de Config.h
PCF8574 pcf8574(PCF_ADDRESS, I2C_SDA, I2C_SCL);

// Implementación del constructor
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
    // ==========================================
    // 1. Configurar pines NATIVOS del ESP32
    // ==========================================
    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);

    // ==========================================
    // 2. Configurar pines del EXPANSOR I2C
    // ==========================================
    // Luces de Freno y Direccionales
    pcf8574.pinMode(PCF_LED_ROJO_L, OUTPUT);
    pcf8574.pinMode(PCF_LED_ROJO_R, OUTPUT);
    pcf8574.pinMode(PCF_LED_AMBAR_L, OUTPUT);
    pcf8574.pinMode(PCF_LED_AMBAR_R, OUTPUT);

    // Luces del LED RGB
    pcf8574.pinMode(PCF_LED_RGB_R, OUTPUT);
    pcf8574.pinMode(PCF_LED_RGB_G, OUTPUT);
    pcf8574.pinMode(PCF_LED_RGB_B, OUTPUT);

    // Iniciar la comunicación con el chip PCF8574
    pcf8574.begin();

    // Asegurar que TODAS las luces del expansor inicien apagadas
    pcf8574.digitalWrite(PCF_LED_ROJO_L, LOW);
    pcf8574.digitalWrite(PCF_LED_ROJO_R, LOW);
    pcf8574.digitalWrite(PCF_LED_AMBAR_L, LOW);
    pcf8574.digitalWrite(PCF_LED_AMBAR_R, LOW);
    pcf8574.digitalWrite(PCF_LED_RGB_R, LOW);
    pcf8574.digitalWrite(PCF_LED_RGB_G, LOW);
    pcf8574.digitalWrite(PCF_LED_RGB_B, LOW);

    // Iniciar con el color de Modo Espera
    updateModeIndicator(MODO_ESPERA);
}

void FeedbackControl::update()
{
    unsigned long currentMillis = millis();

    // ==========================================
    // 1. LÓGICA DE PARPADEO (LUCES ÁMBAR VÍA I2C)
    // ==========================================
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

    // ==========================================
    // 2. LÓGICA DE LA MELODÍA (NO BLOQUEANTE)
    // ==========================================
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
    // En digital, mandar 255 será HIGH. Usamos colores completos.
    case MODO_ESPERA:
        setRGB(255, 255, 255);
        break; // Blanco al 100%
    case MODO_SEGUIDOR_LINEA:
        setRGB(0, 255, 0);
        break; // Verde
    case MODO_EVASION:
        setRGB(0, 0, 255);
        break; // Azul
    case MODO_MQTT:
        setRGB(255, 255, 0);
        break; // Amarillo
    case MODO_GPS:
        setRGB(255, 0, 255);
        break; // Magenta
    default:
        setRGB(0, 0, 0);
        break; // Apagado
    }
}

void FeedbackControl::setRGB(int r, int g, int b)
{
    // Convertimos la señal (0-255) a encendido o apagado para el PCF8574.
    // Si la variable es mayor a 0, se enciende (HIGH), si es 0, se apaga (LOW).
    // NOTA: Si tu LED RGB es de Ánodo Común (se enciende con LOW), invierte la lógica: r > 0 ? LOW : HIGH
    pcf8574.digitalWrite(PCF_LED_RGB_R, r > 0 ? HIGH : LOW);
    pcf8574.digitalWrite(PCF_LED_RGB_G, g > 0 ? HIGH : LOW);
    pcf8574.digitalWrite(PCF_LED_RGB_B, b > 0 ? HIGH : LOW);
}