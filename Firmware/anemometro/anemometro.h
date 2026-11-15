#ifndef ANEMOMETRO_H
#define ANEMOMETRO_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

class Anemometro
{
public:
    explicit Anemometro(uint8_t pin);

    void begin();

    // Retorna a velocidade média desde a última chamada, em km/h
    float getVelocidade();

    // Retorna a frequência média desde a última chamada, em Hz
    float getFrequencia();

    float getUltimaVelocidade() const
    {
        return _ultimaVelocidade;
    }

    float getUltimaFrequencia() const
    {
        return _ultimaFrequencia;
    }

    void reset();

    void setFrequenciaMinima(float freqHz)
    {
        _frequenciaMinima = freqHz;
    }

    void setTimeoutParada(uint32_t timeoutMs)
    {
        _timeoutParadaUs =
            static_cast<uint64_t>(timeoutMs) * 1000ULL;
    }

private:

    uint8_t _pin;

    volatile uint64_t _ultimoPulsoUs;
    volatile uint64_t _somaPeriodosUs;
    volatile uint32_t _numeroPeriodos;

    portMUX_TYPE _mux;

    float _ultimaVelocidade;
    float _ultimaFrequencia;

    float _frequenciaMinima;
    uint64_t _timeoutParadaUs;

    static void IRAM_ATTR isrHandler(void* arg);

    void IRAM_ATTR handlePulse();

    float calcularVelocidade(float frequencia) const;
};

#endif // ANEMOMETRO_H