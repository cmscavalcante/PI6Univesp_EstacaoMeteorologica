#include "pluviometro.h"

// Inicialização dos membros estáticos para acesso dentro da ISR
volatile time_t Pluviometro::_pulseTimestamps[MAX_PULSOS_24H];
volatile size_t Pluviometro::_head = 0;
volatile size_t Pluviometro::_tail = 0;
volatile size_t Pluviometro::_count = 0;
volatile uint32_t Pluviometro::_lastInterruptTime = 0;
uint8_t Pluviometro::_isrPin = 255;

Pluviometro::Pluviometro(uint8_t gpioPin, float mmPerPulse, uint32_t debounceMs) {
    _pin = gpioPin;
    _mmPerPulse = mmPerPulse;
    _debounceMs = debounceMs;
    _isrPin = gpioPin;
}

void Pluviometro::begin() {
    pinMode(_pin, INPUT_PULLUP);
    // Configura a interrupção no pino escolhido (transição FALLING/RISING conforme o sensor)
    attachInterrupt(digitalPinToInterrupt(_pin), Pluviometro::handleInterrupt, FALLING);
}

// Rotina de Interrupção (ISR) executada em IRAM para resposta ultra-rápida
void IRAM_ATTR Pluviometro::handleInterrupt() {
    uint32_t nowMs = millis();
    if (nowMs - _lastInterruptTime >= 200) { // Debounce por tempo
        _lastInterruptTime = nowMs;

        time_t agora = time(NULL); // Obtém o horário POSIX atual

        // Insere o registro no buffer circular
        _pulseTimestamps[_head] = agora;
        _head = (_head + 1) % MAX_PULSOS_24H;

        if (_count < MAX_PULSOS_24H) {
            _count++;
        } else {
            _tail = (_tail + 1) % MAX_PULSOS_24H; // Sobrescreve o mais antigo se encher
        }
    }
}

// Remove registros com mais de 24 horas para liberar memória do buffer
void Pluviometro::limparPulsosAntigos(time_t agora) {
    noInterrupts();
    while (_count > 0) {
        time_t timestampMaisAntigo = _pulseTimestamps[_tail];
        if (agora - timestampMaisAntigo > 86400) { // Mais de 24h (86400 segundos)
            _tail = (_tail + 1) % MAX_PULSOS_24H;
            _count--;
        } else {
            break;
        }
    }
    interrupts();
}

float Pluviometro::calcularAcumulado(uint32_t intervaloSegundos) {
    time_t agora = time(NULL);
    limparPulsosAntigos(agora);

    uint32_t pulsosNoIntervalo = 0;

    noInterrupts();
    for (size_t i = 0; i < _count; i++) {
        size_t idx = (_tail + i) % MAX_PULSOS_24H;
        if (agora - _pulseTimestamps[idx] <= intervaloSegundos) {
            pulsosNoIntervalo++;
        }
    }
    interrupts();

    return (float)pulsosNoIntervalo * _mmPerPulse;
}

float Pluviometro::getChuvaUltimoMinuto() {
    return calcularAcumulado(60); // 60 segundos
}

float Pluviometro::getChuvaUltimaHora() {
    return calcularAcumulado(3600); // 1 hora
}

float Pluviometro::getChuvaUltimas24Horas() {
    return calcularAcumulado(86400); // 24 horas
}