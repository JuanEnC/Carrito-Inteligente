#include "Feedback.h"

// Implementación del constructor
FeedbackControl::FeedbackControl() {
    currentSignal = SIGNAL_OFF;
    hornActive = false;
    blinkState = false;
    previousMillisBlink = 0;
    
    playingMelody = false;
    melodyStep = 0;
    previousMillisMelody = 0;
}

void FeedbackControl::begin() {
    // 1. Configurar pines de luces y claxon
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_ROJO_L, OUTPUT);
    pinMode(LED_ROJO_R, OUTPUT);
    pinMode(LED_AMBAR_L, OUTPUT);
    pinMode(LED_AMBAR_R, OUTPUT);

    // 2. Configurar pines del nuevo LED RGB (Indicador de Modos)
    pinMode(LED_RGB_R, OUTPUT);
    pinMode(LED_RGB_G, OUTPUT);
    pinMode(LED_RGB_B, OUTPUT);

    // 3. Asegurar que todo inicie apagado
    digitalWrite(LED_ROJO_L, LOW);
    digitalWrite(LED_ROJO_R, LOW);
    digitalWrite(LED_AMBAR_L, LOW);
    digitalWrite(LED_AMBAR_R, LOW);
    noTone(BUZZER_PIN); 
    
    // Iniciar con el color de Modo Espera
    updateModeIndicator(MODO_ESPERA);
}

void FeedbackControl::update() {
    unsigned long currentMillis = millis();

    // ==========================================
    // 1. LÓGICA DE PARPADEO (LUCES ÁMBAR)
    // ==========================================
    if (currentSignal != SIGNAL_OFF) {
        // Verifica si ya pasó el tiempo de intervalo (ej. 500ms)
        if (currentMillis - previousMillisBlink >= blinkInterval) {
            previousMillisBlink = currentMillis;   // Guarda el tiempo actual
            blinkState = !blinkState;              // Invierte el estado (ON a OFF, o OFF a ON)

            // Aplica el estado dependiendo de la señal activa
            if (currentSignal == SIGNAL_LEFT) {
                digitalWrite(LED_AMBAR_L, blinkState);
                digitalWrite(LED_AMBAR_R, LOW);
            } else if (currentSignal == SIGNAL_RIGHT) {
                digitalWrite(LED_AMBAR_R, blinkState);
                digitalWrite(LED_AMBAR_L, LOW);
            } else if (currentSignal == SIGNAL_HAZARD) { // Preventivas
                digitalWrite(LED_AMBAR_L, blinkState);
                digitalWrite(LED_AMBAR_R, blinkState);
            }
        }
    }

    // ==========================================
    // 2. LÓGICA DE LA MELODÍA (NO BLOQUEANTE)
    // ==========================================
    if (playingMelody) {
        // Avanza la nota cada 150 milisegundos
        if (currentMillis - previousMillisMelody >= 150) {
            previousMillisMelody = currentMillis;
            
            // Frecuencias para un sonido de encendido rápido (tipo Mario Bros coin o inicio de sistema)
            int notes[] = {880, 1047, 1319}; // A5, C6, E6
            
            if (melodyStep < 3) {
                tone(BUZZER_PIN, notes[melodyStep]);
                melodyStep++;
            } else {
                // Termina la melodía
                noTone(BUZZER_PIN);
                playingMelody = false;
                
                // Si el usuario estaba presionando el claxon manual al mismo tiempo, lo restauramos
                if (hornActive) tone(BUZZER_PIN, 1000);
            }
        }
    }
}

void FeedbackControl::setBrakeLights(bool state) {
    // Las luces rojas se encienden al frenar [cite: 37]
    digitalWrite(LED_ROJO_L, state ? HIGH : LOW);
    digitalWrite(LED_ROJO_R, state ? HIGH : LOW);
}

void FeedbackControl::setTurnSignal(SignalState state) {
    currentSignal = state;
    
    // Si la instrucción es apagar las luces, lo hacemos inmediatamente
    // sin esperar a que el 'update()' haga su ciclo
    if (currentSignal == SIGNAL_OFF) {
        digitalWrite(LED_AMBAR_L, LOW);
        digitalWrite(LED_AMBAR_R, LOW);
    } else {
        // Reiniciamos el temporizador y forzamos encendido inmediato para mejor respuesta visual
        previousMillisBlink = millis();
        blinkState = HIGH;
        
        if (currentSignal == SIGNAL_LEFT) digitalWrite(LED_AMBAR_L, HIGH);
        if (currentSignal == SIGNAL_RIGHT) digitalWrite(LED_AMBAR_R, HIGH);
        if (currentSignal == SIGNAL_HAZARD) {
            digitalWrite(LED_AMBAR_L, HIGH);
            digitalWrite(LED_AMBAR_R, HIGH);
        }
    }
}

void FeedbackControl::setHorn(bool state) {
    hornActive = state;
    // El buzzer pasivo requiere frecuencias (tone), no solo un voltaje HIGH.
    // 1000 Hz es una frecuencia estridente, perfecta para un claxon.
    if (hornActive) {
        tone(BUZZER_PIN, 1000); 
    } else {
        if (!playingMelody) { // Evitar apagar el buzzer si justo está sonando la melodía de inicio
            noTone(BUZZER_PIN);
        }
    }
}

void FeedbackControl::playMelody() {
    playingMelody = true;
    melodyStep = 0;
    previousMillisMelody = millis();
}

void FeedbackControl::updateModeIndicator(CarMode mode) {
    switch (mode) {
        case MODO_ESPERA:          setRGB(50, 50, 50);   break; // Blanco tenue
        case MODO_SEGUIDOR_LINEA:  setRGB(0, 255, 0);    break; // Verde
        case MODO_EVASION:         setRGB(0, 0, 255);    break; // Azul
        case MODO_MQTT:            setRGB(255, 255, 0);  break; // Amarillo
        case MODO_GPS:             setRGB(255, 0, 255);  break; // Magenta
        default:                   setRGB(0, 0, 0);      break; // Apagado
    }
}

void FeedbackControl::setRGB(int r, int g, int b) {
    // En el ESP-32, si usas un LED de cátodo común (más común), 
    // 255 es encendido total. Si es ánodo común, sería 255 - valor.
    analogWrite(LED_RGB_R, r);
    analogWrite(LED_RGB_G, g);
    analogWrite(LED_RGB_B, b);
}