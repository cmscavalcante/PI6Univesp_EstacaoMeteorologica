#include "anemometro.h"

#include "esp_timer.h"

// ============================================================
// FUNÇÃO DE TRANSFERÊNCIA
// ============================================================
//
// v(km/h) = A*f + B + C/(f + F0)
//
// f = frequência dos pulsos ópticos em Hz
//
// Coeficientes obtidos experimentalmente
// ============================================================

static constexpr float TF_A  =  3.05354550f;
static constexpr float TF_B  = -1.00222905f;
static constexpr float TF_C  =  6.55969028f;
static constexpr float TF_F0 =  0.01000000f;


// ============================================================
// CONSTRUTOR
// ============================================================

Anemometro::Anemometro(uint8_t pin)
    : _pin(pin),
      _ultimoPulsoUs(0),
      _somaPeriodosUs(0),
      _numeroPeriodos(0),
      _mux(portMUX_INITIALIZER_UNLOCKED),
      _ultimaVelocidade(0.0f),
      _ultimaFrequencia(0.0f),
      _frequenciaMinima(1.0f),
      _timeoutParadaUs(2000000ULL)
{
}


// ============================================================
// BEGIN
// ============================================================

void Anemometro::begin()
{
    pinMode(_pin, INPUT_PULLUP);

    reset();

    attachInterruptArg(
        _pin,
        Anemometro::isrHandler,
        this,
        FALLING
    );
}


// ============================================================
// ISR ESTÁTICA
// ============================================================

void IRAM_ATTR Anemometro::isrHandler(void* arg)
{
    Anemometro* self =
        static_cast<Anemometro*>(arg);

    self->handlePulse();
}


// ============================================================
// ISR DO SENSOR
// ============================================================

void IRAM_ATTR Anemometro::handlePulse()
{
    const uint64_t agora =
        static_cast<uint64_t>(
            esp_timer_get_time()
        );

    portENTER_CRITICAL_ISR(&_mux);

    if (_ultimoPulsoUs != 0) {

        uint64_t periodo =
            agora - _ultimoPulsoUs;

        /*
         * Rejeição simples de glitches elétricos.
         *
         * 1000 us corresponde a 1000 Hz,
         * muito acima da frequência física possível.
         */
        if (periodo >= 1000) {

            _somaPeriodosUs += periodo;
            _numeroPeriodos++;
        }
    }

    _ultimoPulsoUs = agora;

    portEXIT_CRITICAL_ISR(&_mux);
}


// ============================================================
// FREQUÊNCIA MÉDIA
// ============================================================

float Anemometro::getFrequencia()
{
    uint64_t somaPeriodos;
    uint32_t numeroPeriodos;
    uint64_t ultimoPulso;

    // --------------------------------------------------------
    // Snapshot atômico
    // --------------------------------------------------------

    portENTER_CRITICAL(&_mux);

    somaPeriodos = _somaPeriodosUs;
    numeroPeriodos = _numeroPeriodos;
    ultimoPulso = _ultimoPulsoUs;

    // inicia nova janela
    _somaPeriodosUs = 0;
    _numeroPeriodos = 0;

    portEXIT_CRITICAL(&_mux);


    const uint64_t agora =
        static_cast<uint64_t>(
            esp_timer_get_time()
        );


    // --------------------------------------------------------
    // Detecta rotor parado
    // --------------------------------------------------------

    if (
        ultimoPulso == 0 ||
        (agora - ultimoPulso) > _timeoutParadaUs
    ) {

        _ultimaFrequencia = 0.0f;

        return 0.0f;
    }


    // --------------------------------------------------------
    // Nenhum período completo desde a chamada anterior
    // --------------------------------------------------------

    if (
        numeroPeriodos == 0 ||
        somaPeriodos == 0
    ) {

        return _ultimaFrequencia;
    }


    // --------------------------------------------------------
    // Frequência média
    //
    // f = N / soma(T)
    // --------------------------------------------------------

    _ultimaFrequencia =
        (
            static_cast<float>(numeroPeriodos)
            * 1000000.0f
        )
        /
        static_cast<float>(somaPeriodos);


    return _ultimaFrequencia;
}


// ============================================================
// FUNÇÃO DE TRANSFERÊNCIA
// ============================================================

float Anemometro::calcularVelocidade(
    float frequencia
) const
{
    /*
     * A função racional não deve ser extrapolada
     * para frequências próximas de zero.
     *
     * A calibração utilizada foi realizada para
     * aproximadamente f >= 1 Hz.
     */
    if (frequencia < _frequenciaMinima) {
        return 0.0f;
    }


    float velocidade =
        TF_A * frequencia
        + TF_B
        + TF_C / (frequencia + TF_F0);


    // proteção contra qualquer resultado não físico
    if (
        !isfinite(velocidade) ||
        velocidade < 0.0f
    ) {

        return 0.0f;
    }


    return velocidade;
}


// ============================================================
// VELOCIDADE
// ============================================================

float Anemometro::getVelocidade()
{
    float frequencia =
        getFrequencia();


    if (frequencia <= 0.0f) {

        _ultimaVelocidade = 0.0f;

        return 0.0f;
    }


    _ultimaVelocidade =
        calcularVelocidade(
            frequencia
        );


    return _ultimaVelocidade;
}


// ============================================================
// RESET
// ============================================================

void Anemometro::reset()
{
    portENTER_CRITICAL(&_mux);

    _ultimoPulsoUs = 0;
    _somaPeriodosUs = 0;
    _numeroPeriodos = 0;

    portEXIT_CRITICAL(&_mux);

    _ultimaVelocidade = 0.0f;
    _ultimaFrequencia = 0.0f;
}