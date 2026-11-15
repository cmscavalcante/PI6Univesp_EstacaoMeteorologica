#ifndef PLUVIOMETRO_H
#define PLUVIOMETRO_H

#include <Arduino.h>
#include <time.h>

#define MAX_PULSOS_24H 1000 // Tamanho do buffer de histórico de pulsos

class Pluviometro {
public:
    // Construtor: pino digital, mm por pulso e tempo de debouncing (ms)
    Pluviometro(uint8_t gpioPin, float mmPerPulse, uint32_t debounceMs = 200);

    void begin();

    // Getters para os acumulados móveis (retornam em mm de chuva)
    float getChuvaUltimoMinuto();
    float getChuvaUltimaHora();
    float getChuvaUltimas24Horas();

    // Função estática de interrupção (ISR)
    static void IRAM_ATTR handleInterrupt();

private:
    uint8_t _pin;
    float _mmPerPulse;
    uint32_t _debounceMs;

    // Estrutura do buffer circular para timestamps
    static volatile time_t _pulseTimestamps[MAX_PULSOS_24H];
    static volatile size_t _head;
    static volatile size_t _tail;
    static volatile size_t _count;
    static volatile uint32_t _lastInterruptTime;
    static uint8_t _isrPin;

    void limparPulsosAntigos(time_t agora);
    float calcularAcumulado(uint32_t intervaloSegundos);
};

#endif // PLUVIOMETRO_H