#include <Arduino.h>
#include "Config.h"
#include "Motors.h"
#include "Sensors.h"
#include "Network.h"
#include "Feedback.h"

// ==========================================
// INSTANCIACIÓN DE OBJETOS GLOBALES
// ==========================================
// Estas son las instancias reales que prometimos con 'extern' en los archivos .h
MotorsControl Motors;
SensorsControl Sensors;
NetworkControl Network;
FeedbackControl Feedback;

// Variable global para controlar en qué modo se encuentra el carro
// Por seguridad, iniciamos en MODO_ESPERA
CarMode currentMode = MODO_ESPERA;

// Variables para almacenar el destino del Modo 4 (GPS)
double targetLat = 0.0;
double targetLng = 0.0;

void setup()
{
    // Iniciar comunicación serial para depuración
    Serial.begin(115200);
    Serial.println("\n--- Iniciando Smart Car ESP-32 ---");

    // 1. Inicializar todos los módulos
    Motors.begin();
    Sensors.begin();
    Feedback.begin();
    Network.begin();

    // 2. Dar retroalimentación de que el setup terminó correctamente
    Feedback.playMelody();
    Serial.println("Sistema inicializado. Esperando comandos MQTT para cambiar de modo...");
}

void loop()
{
    // ==========================================
    // TAREAS EN SEGUNDO PLANO (NO BLOQUEANTES)
    // ==========================================
    // Estas tres líneas son críticas. Mantienen la red viva, el GPS actualizado
    // y las luces/sonidos funcionando sin importar en qué modo estemos.
    Network.loop();
    Sensors.updateGPS();
    Feedback.update();

    // ==========================================
    // MÁQUINA DE ESTADOS PRINCIPAL
    // ==========================================
    switch (currentMode)
    {

    case MODO_ESPERA:
        // El carro está encendido pero inactivo
        Motors.stop(); // Esto automáticamente enciende los LEDs rojos
        break;

    case MODO_SEGUIDOR_LINEA:
    {
        // Leemos el byte completo de la matriz IR (1 = Negro/Línea, 0 = Blanco/Pista)
        uint8_t irState = Sensors.readIRArray();

        // Lógica de control diferencial
        // Suponemos que los sensores del centro son los bits 3 y 4 (0b00011000)
        if (irState == 0b00011000 || irState == 0b00001000 || irState == 0b00010000)
        {
            // Línea detectada en el centro: Avanzar recto
            Motors.setSpeeds(150, 150);
        }
        // Evaluamos la mitad izquierda de la matriz usando compuertas AND bit a bit
        else if (irState & 0b11100000)
        {
            // Línea a la izquierda: Frenar llanta izquierda, acelerar derecha para girar
            Motors.setSpeeds(50, 150);
        }
        // Evaluamos la mitad derecha de la matriz
        else if (irState & 0b00000111)
        {
            // Línea a la derecha: Acelerar izquierda, frenar derecha para girar
            Motors.setSpeeds(150, 50);
        }
        else if (irState == 0b00000000)
        {
            // Perdió la línea: Por seguridad, detenerse (o podrías programar un giro de búsqueda)
            Motors.stop();
        }
        break;
    }

    case MODO_EVASION:
    {
        // Leer la distancia en centímetros
        float distancia = Sensors.getDistance();

        // Si el obstáculo está a más de 25 cm (o si devolvió 999 por estar muy lejos)
        if (distancia > 25.0)
        {
            // Camino despejado, avanzar
            Motors.drive(DIR_FORWARD, 150);
        }
        else
        {
            // Obstáculo inminente
            // Como no usamos delay(), el loop es tan rápido que el carro seguirá
            // girando a la derecha en su propio eje MIENTRAS el obstáculo siga enfrente.
            // En cuanto el camino se despeje, el 'if' superior volverá a disparar el avance.
            Motors.drive(DIR_RIGHT, 150);
        }
        break;
    }

    case MODO_MQTT:
        // En este modo no hacemos nada aquí.
        // Las instrucciones de movimiento llegan por Network.cpp -> mqttCallback -> Motors.drive()
        break;

    case MODO_GPS:
    {
        // Validamos que la aplicación web nos haya enviado un destino real
        if (targetLat != 0.0 && targetLng != 0.0)
        {

            // Calculamos cuántos metros nos faltan
            double distanciaDestino = Sensors.getDistanceTo(targetLat, targetLng);

            // Si estamos a menos de 3 metros, consideramos que llegamos al objetivo
            if (distanciaDestino < 3.0)
            {
                Motors.stop();
                Serial.println("¡Destino GPS alcanzado!");

                // Opcional: Cambiar automáticamente a modo espera al llegar
                // changeMode(MODO_ESPERA);
            }
            else
            {
                // Aún no llegamos. Calculamos el ángulo hacia el objetivo.
                double anguloDestino = Sensors.getCourseTo(targetLat, targetLng);

                // NOTA DE HARDWARE: Para una navegación GPS 100% autónoma se requiere
                // un magnetómetro (brújula digital como la HMC5883L) para conocer
                // la orientación actual del carrito cuando está detenido.
                // Como el hardware listado en el PDF no cuenta con uno,
                // implementamos un avance básico continuo mientras la distancia sea mayor a 3m.
                Motors.drive(DIR_FORWARD, 150);
            }
        }
        else
        {
            // Si entramos al modo GPS pero no hemos tocado el mapa en la web, nos quedamos quietos
            Motors.stop();
        }
        break;
    }
    }
}

// Función auxiliar para que desde Network.cpp (cuando llegue un comando de la app)
// podamos cambiar el modo de operación de forma segura.
void changeMode(CarMode newMode)
{
    if (currentMode != newMode)
    {
        currentMode = newMode;
        Motors.stop(); // Por seguridad, al cambiar de modo siempre detenemos los motores primero
        Feedback.updateModeIndicator(currentMode);
        Serial.print("Cambio de modo a: ");
        Serial.println(currentMode);
    }
}