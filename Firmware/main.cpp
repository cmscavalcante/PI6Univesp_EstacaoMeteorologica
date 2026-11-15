/* Bibliotecas ESP */
#include <Arduino.h>
#include <AsyncDelay.h>
#include <Adafruit_NeoPixel.h>

/* Bibliotecas C99 */
#include <cstdlib>
#include <stdio.h>
#include <stdbool.h>
#include <float.h>
#include <atomic>
#include <time.h>

/* Classes proprietárias */
  #include "iot_configs.h"
  #include "wifiManagement/wifiManagement.h"
  #include "OTA/ota.h"
  #include "MQTT/mqttSys.h"
  #include "Time/hora.h"
  #include "Display/display.h"
  #include "tpu/tpu.h"
  #include "pluviometro/pluviometro.h"
  #include "anemometro/anemometro.h"

//Garante que o flash da placa de desenvolvimento esteja apagado
    // #define FLASH_LIGHT_PIN 4
    #define RAIN_PIN 2          // Pino de entrada do pluviômetro
    #define MM_POR_PULSO 0.15    // Exemplo: 1 pulso = 0.15 mm de chuva
    #define FLASH_PIXELS 1
    #define ANEMOMETRO_PIN 12 // Pino de entrada do anemômetro
    // Adafruit_NeoPixel flashLight(FLASH_PIXELS, FLASH_LIGHT_PIN, NEO_RBG + NEO_KHZ800);
    #define INT_LED_PIN 33 //LED interno da placa, somente para monitoramento
  // #define FLASH_LIGHT_PIN 4 //Flash para câmera

/*Construtores das classes*/
  wifiManager wM(HOSTNAME);
  otaHandle OTA_handle;
  mqtt Mqtt;
  Hora hora;
  Display meuDisplay;
  TPU tpu(850.0); // Altitude em metros, ajuste conforme necessário
  Pluviometro pluviometro(RAIN_PIN, MM_POR_PULSO);
  Anemometro anemometro(ANEMOMETRO_PIN);
  
  bool INT_LED;
  bool FLASH_LIGHT;
  bool displayLogoFlag;
  bool qrCodeReadFlag;

  AsyncDelay relogioDelay; //intervalo de refresh do relogio
  AsyncDelay displayLogoInterval; //iintervalo para exibição do logo
  /*Intervalos de medição*/
    AsyncDelay measTempInterval; //intervalo de medição de temperatura
    AsyncDelay measPressInterval; //intervalo de medição de pressão
    AsyncDelay measUmidInterval; //intervalo de medição de umidade
    AsyncDelay measVaneInterval; //intervalo de medição de direção do vento
  AsyncDelay mqttSendInterval; //intervalo de envio de dados via MQTT

  /*Constantes de tempo dos instrumentos*/
    const unsigned long TEMP_INTERVAL = 20000; // 20 segundos
    const unsigned long PRESS_INTERVAL = 2000; // 2 segundos
    const unsigned long U MID_INTERVAL = 40000; // 40 segundos
    const unsigned long VANE_INTERVAL = 5000; // 5 segundos

  /*Variaveis de armazenamento de dados*/ 
    float temperatura = 0.0;
    float tempAccInt = 0.0;
    int tempAccN = 0;
    float pressao = 0.0;
    float pressAccInt = 0.0;
    int pressAccN = 0;
    float umidade = 0.0;
    float umidAccInt = 0.0;
    int umidAccN = 0;
    float pluv24h = 0.0;
    float pluvLastHour = 0.0;
    float pluvLastMinute = 0.0;
    float anemometro = 0.0;
    float vane = 0.0;

/* Inicialização da máquina */
void setup() {
  Serial.begin(115200);
  Serial.println("oi");

  pinMode(INT_LED_PIN, OUTPUT);
  
  /*Inicializações*/
    meuDisplay.displayInit(); //inicialização de display
    meuDisplay.displayPrint("Display inicializado",0);
    Serial.println("Fim da inicialização de Display");
    delay(500);

    meuDisplay.displayPrint("Iniciando WiFi",0);
    wM.wifiInit();          //inicialização de wifi
    meuDisplay.displayPrint("WiFi inicializado",1);
    delay(1000);
    Serial.println("Fim da inicialização de WiFi");

    meuDisplay.displayPrint("Iniciando Relógio",0);
    hora.initTimes();       //inicialização de relógio
    meuDisplay.displayPrint("Relógio inicializado",1);
    delay(1000);
    Serial.println("Fim da inicialização de Relógio");

    meuDisplay.displayPrint("Iniciando MQTT",0);
    Mqtt.mqttInit();        //inicialização de MQTT
    meuDisplay.displayPrint("MQTT inicializado",1);
    delay(1000);
    Serial.println("Fim da inicialização de MQTT");
    
    meuDisplay.displayPrint("Iniciando TPU",0);
    if (!tpu.begin()) {
        meuDisplay.displayPrint("Erro ao inicializar TPU",1);
    }
    else{
        meuDisplay.displayPrint("TPU inicializado",1);
    }
    delay(1000);

    meuDisplay.displayPrint("Iniciando Pluviometro", 0);
    pluviometro.begin();
    delay(500);

    meuDisplay.displayPrint("Iniciando Anemometro", 0);
    anemometro.begin();
    delay(500);

    meuDisplay.displayPrint("Iniciando Biruta", 0);
    vane.begin();
    delay(500);

    meuDisplay.displayPrint("Iniciando OTA",0);
    otaHandle::otaInit(meuDisplay); //inicialização de OTA
    meuDisplay.displayPrint("OTA inicializado",1);
    delay(1000);
    Serial.println("Fim da inicialização de OTA");

    //inicialização de delays
    relogioDelay.start(1000, AsyncDelay::MILLIS);
    displayLogoInterval.start(3000, AsyncDelay::MILLIS);
    measTempInterval.start(TEMP_INTERVAL, AsyncDelay::MILLIS);
    measPressInterval.start(PRESS_INTERVAL, AsyncDelay::MILLIS);
    measUmidInterval.start(U MID_INTERVAL, AsyncDelay::MILLIS);
    measVaneInterval.start(VANE_INTERVAL, AsyncDelay::MILLIS);
    mqttSendInterval.start(60000, AsyncDelay::MILLIS);

    meuDisplay.statusBar(wM.getWifiSignal(), true, hora.getDate(), hora.getHour());
    Serial.println("Barra de status");
}

/* Função loop, executada a cada ciclo de máquina */
void loop() {
  ArduinoOTA.handle(); //verifica se há atualização OTA
  // Leitura dos acumulados em milímetros:
    pluvLastHour = pluviometro.getChuvaUltimoMinuto();
    pluvLastMinute = pluviometro.getChuvaUltimoMinuto();
    pluv24h = pluviometro.getChuvaUltimas24Horas();
  
  if (measTempInterval.isExpired()){
    float temperatura = tpu.getTemperature().toFloat();
    tempAccInt += temperatura;
    tempAccN++;
    measTempInterval.repeat();
  }

  if (measPressInterval.isExpired()){
    float pressao = tpu.getPressure().toFloat();
    pressAccInt += pressao;
    pressAccN++;
    measPressInterval.repeat();
  }

  if (measUmidInterval.isExpired()){
    float umidade = tpu.getHumidity().toFloat();
    umidAccInt += umidade;
    umidAccN++;
    measUmidInterval.repeat();
  }


  if (displayLogoInterval.isExpired()){
    Serial.println("logotimer");
    displayLogoFlag = !displayLogoFlag;
    displayLogoInterval.repeat();
  }
  
  if (displayLogoFlag){
    if (measurementInterval.isExpired()){
      char mensagem[60];

      strcpy(mensagem, "Temperatura: "); 
      strcat(mensagem, tpu.getTemperature().c_str());
      strcat(mensagem, " °C");
      meuDisplay.displayPrint(mensagem, 0);

      strcpy(mensagem, "Pressão: "); 
      strcat(mensagem, tpu.getPressure().c_str());
      strcat(mensagem, " hPa");
      meuDisplay.displayPrint(mensagem, 1); 

      strcpy(mensagem, "Umidade: "); 
      strcat(mensagem, tpu.getHumidity().c_str());
      strcat(mensagem, " %");
      meuDisplay.displayPrint(mensagem, 2);
      
      strcpy(mensagem, "Chuva: ");
      strcat(mensagem, String(pluv24h).c_str());
      strcat(mensagem, " mm");
      meuDisplay.displayPrint(mensagem, 3);

      
      meuDisplay.displayPrint(mensagem, 4);

      String direcaoVento = vane.getWindDirection();
      strcpy(mensagem, "Vento: ");
      float velocidade = anemometro.getVelocidade();
      strcpy(mensagem, String(velocidade).c_str());
      strcat(mensagem, " km/h - ");
      strcat(mensagem, direcaoVento.c_str());
      meuDisplay.displayPrint(mensagem, 5);

      measurementInterval.repeat();
    }
  }
  else if (!displayLogoFlag){meuDisplay.displayLogo();}
  meuDisplay.statusBar(wM.getWifiSignal(), true, hora.getDate(), hora.getHour());
  if (mqttSendInterval.isExpired()){
    //Média das últimas leituras de temperatura, pressão e umidade
    temperatura = tempAccInt / tempAccN;
    pressao = pressAccInt / pressAccN;
    umidade = umidAccInt / umidAccN; 
    //Reset dos registradores de dados acumulados
    tempAccInt = 0.0;
    tempAccN = 0;
    pressAccInt = 0.0;
    pressAccN = 0;
    umidAccInt = 0.0;
    umidAccN = 0;
    Mqtt.mqttSendData(temperatura, pressao, umidade, pluv24h, pluvLastHour, pluvLastMinute, anemometro.getVelocidade(), vane.getWindDirection(), hora.getTime());
    mqttSendInterval.repeat();
  }

}