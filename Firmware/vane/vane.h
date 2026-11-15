#ifndef VANE_H
#define VANE_H

#include <Arduino.h>
#include <float.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>


class Vane {
  public:
    String getWindDirection();
    bool begin();

  private:
    Adafruit_ADS1115 _ads_E; // ADC de efeito Hall da biruta lado Leste (E)
    Adafruit_ADS1115 _ads_W; // ADC de efeito Hall da biruta lado Oeste (W)
    
    bool _adsEerror=false;
    bool _adsWerror=false;
    
    float _volts[8];
    int16_t _adcs[8];
    // static const String dir16[16] = {"N", "N-NE", "NE", "E-NE", "E", "E-SE", "SE", "S-SE", 
    static constexpr float offsets[8] = {1.67, 1.68, 1.59, 1.63, 1.54, 1.73, 1.68, 1.57};
    // Ângulos dos sensores em graus: N=0, NE=45, E=90, SE=135, S=180, SW=225, W=270, NW=315
    static constexpr float sensorAnglesDeg[8] = {0, 45, 90, 135, 180, 225, 270, 315};

    static constexpr uint8_t ADS_W_ADD = 0x48; // Endereço I2C do ADC lado Oeste (W)
    static constexpr uint8_t ADS_E_ADD = 0x49; // Endereço I2C do ADC lado Leste (E)
};

#endif // VANE_H