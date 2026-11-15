#include "vane.h"

static const char* DIR16[16] = {
    "N", "N-NE", "NE", "E-NE", "E", "E-SE", "SE", "S-SE", 
    "S", "S-SW", "SW", "W-SW", "W", "W-NW", "NW", "N-NW"
};

bool Vane::begin() {
  _adsEerror = !_ads_E.begin(ADS_E_ADD);
  _adsWerror = !_ads_W.begin(ADS_W_ADD);

  return (!_adsEerror && !_adsWerror);; // Ambos os ADCs foram inicializados com sucesso
}

String Vane::getWindDirection() {
    if (_adsEerror || _adsWerror) {
        return "N/A"; // Retorna uma mensagem de erro se algum ADC falhar
    }
    else{
        for (int i = 0; i < 2; i++){
            for (int j = 0; j < 4; j++){
                if (i == 0) {
                    _adcs[j] = _ads_E.readADC_SingleEnded(j);
                    _volts[j] = _ads_E.computeVolts(_adcs[j]);
                } 
                else if (i == 1) {
                    _adcs[j + 4] = _ads_W.readADC_SingleEnded(j);
                    _volts[j + 4] = _ads_W.computeVolts(_adcs[j + 4]);
                }
                
            }
        }

        float totalX = 0;
        float totalY = 0;
        float threshold = 0.04; // Ignora variações menores que 40 mV (ruído)

        for (int k = 0; k < 8; k++) {
        float force = _volts[k] - offsets[k];
        float magnitude = abs(force);

        if (magnitude > threshold) {
            // FUNÇÃO DE COMPRESSÃO
            float magnitudeSuave = pow(magnitude, 0.5);
            float rad = sensorAnglesDeg[k] * M_PI / 180.0;
            totalX += magnitudeSuave * cos(rad);
            totalY += magnitudeSuave * sin(rad);
        }
        }

        float heading = 0;
        if (abs(totalX) > 0.0001 || abs(totalY) > 0.0001) {
        // atan2 resolve o quadrante automaticamente
        heading = atan2(totalY, totalX) * 180.0 / M_PI;
        
        // Converte de [-180, 180] para [0, 360]
        if (heading < 0) heading += 360;
        }

        // Para exibir a direção cardinal mais próxima (opcional)
        int indexProximo = (int)((heading + 11.25) / 22.5) % 16;
        
        return DIR16[indexProximo];
    }
}