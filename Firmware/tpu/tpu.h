#ifndef TPU_H
#define TPU_H

#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP085.h>

class TPU {
  public:
    explicit TPU(float myAltitude);
    bool begin();
    String getTemperature();
    String getPressure();
    String getHumidity();

  private:
    Adafruit_AHTX0 _aht;  // AHT10 temperatura e umidade
    Adafruit_BMP085 _bmp; // BMP085 temperatura e pressão
    bool _bmpError = false;
    bool _ahtError = false;
    float _myAltitude;
    String formatFloat(float Float) {
        char buffer[10];
        snprintf(buffer, sizeof(buffer), "%.1f", Float);
        return String(buffer);
    }
};


#endif // TPU_H