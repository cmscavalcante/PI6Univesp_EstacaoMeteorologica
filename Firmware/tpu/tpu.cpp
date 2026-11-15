#include "tpu.h"
#include <iostream>
#include <charconv>
#include <array>
#include <string>


TPU::TPU(float myAltitude):_myAltitude(myAltitude){}


bool TPU::begin() {
  _ahtError = !_aht.begin();
  _bmpError = !_bmp.begin();
  
  // Retorna true se pelo menos um dos sensores funcionou
  return (!_ahtError || !_bmpError);
}

String TPU::getTemperature() {
  if (_ahtError && _bmpError) {
    return "Erro"; // Retorna "Erro" se a inicialização de ambos os sensores falhar
  }
  float tempAHT;
  float tempBMP;
  if (!_ahtError) {
    sensors_event_t tempEvent;
    _aht.getEvent(nullptr, &tempEvent); // Obtém o evento de temperatura
    tempAHT = tempEvent.temperature;
  }
  if (!_bmpError) {
    tempBMP = _bmp.readTemperature();
  }
  if (_bmpError) {
    return formatFloat(tempAHT); // Retorna a temperatura apenas do AHT10
  }
  else if (_ahtError) {
    return formatFloat(tempBMP); // Retorna a temperatura apenas do BMP085
  }
  else{
    return formatFloat((tempAHT + tempBMP) / 2.0); // Retorna a média das leituras de ambas as temperaturas
  }
}

String TPU::getPressure() {
  if (_bmpError) {
    return "Erro"; // Retorna NaN se a inicialização do BMP085 falhar
  }
  return formatFloat((_bmp.readPressure() / pow((1.0 - (_myAltitude / 44330.0)), 5.255 ))/100); // Obtém a pressão a nível do mar usando a altitude fornecida
}

String TPU::getHumidity() {
  if (_ahtError) {
    return "Erro"; // Retorna "Erro" se a inicialização do AHT10 falhar
  }
  sensors_event_t humidityEvent;
  _aht.getEvent(&humidityEvent, nullptr); // Obtém o evento de umidade
  return formatFloat(humidityEvent.relative_humidity); // Retorna a umidade relativa
}