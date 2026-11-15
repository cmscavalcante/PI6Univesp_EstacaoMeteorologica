/*Bibliotecas ESP*/
#include <Arduino.h>
#include <WiFiManager.h>
#include <LittleFS.h>
#include <FS.h>


/* Bibliotecas C99 */
#include <cstdlib>
#include <stdio.h>
#include <stdbool.h>
#include <float.h>
#include <atomic>

#include "wifiManagement.h"

WiFiManager wm;                         //Construtor para a biblioteca WiFiManager


WiFiManagerParameter custom_field;      //para parâmetros de modo não-bloqueio

wifiManager::wifiManager(const char* hostname) : _hostname(hostname) {} //Construtor da classe wifiManager - minúsculas para diferenciar a classe da biblioteca

String wifiManager::readFile(fs::FS &fs, const char * path){ 
    // Leitura do arquivo de configuração WiFi
    Serial.printf("Lendo arquivo: %s\r\n", path);
    File file = fs.open(path, "r");     // Abre o arquivo na pasta "path"
    if(!file || file.isDirectory()){
        Serial.println("- arquivo vazio ou falha na abertura do arquivo");
        return String();
    }
    Serial.println("- lido do arquivo:");
    String fileContent;
    while (file.available()){
        fileContent+=String((char)file.read());
    }
    file.close();
    Serial.println(fileContent);
    return fileContent;
}

void wifiManager::writeFile(fs::FS &fs, const char * path, const char * message){
    //Gravação do arquivo de configuração WiFi
    Serial.printf("Gravando arquivo: %s\r\n", path);
    File file = fs.open(path, "w");
    if(!file){
        Serial.println("- falha na abertura do arquivo para gravação");
        return;
    }
    if(file.print(message)){
        Serial.println("- arquivo gravado");
    } else {
        Serial.println("- falha na gravação");
    }
    file.close();
}

void wifiManager::wifiInit(void){
    //Inicialização da conexão WiFi
    if (!LittleFS.begin(true)) {            //inicializa sistema de arquivos
        Serial.println("Falha de montagem do sistema de arquivos");
        return;
    }
    if (LittleFS.exists("/data.txt")) {
        data = readFile(LittleFS, "/data.txt").toInt();
    } else {
        Serial.println("Arquivo /data.txt nao encontrado. Criando padrao...");
        writeFile(LittleFS, "/data.txt", "0");
    }
    WiFi.mode(WIFI_STA);                //definição explícita do modo WiFi, evitando que o ESP32 entre no modo access point
    if(wm_nonblocking) wm.setConfigPortalBlocking(false);
    int customFieldLength = 40;         //adiciona um campo de entrada customizado
    //teste de html (rádio)
    const char* custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
    new (&custom_field) WiFiManagerParameter(custom_radio_str); // entrada html customizada
    wm.addParameter(&custom_field);
    wm.setSaveParamsCallback(
        std::bind(&wifiManager::saveParamCallback, this
        )); //Atribui uma função para gravação de parâmetros em callback
    //Menu via vetor
    std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
    wm.setMenu(menu);                   //Inicializa o menu
    wm.setClass("invert");              //tema escuro
    wm.setConfigPortalTimeout(30);      // fecha o portal automaticamente após n segundos
    // Configuração do scan de WiFi
    wm.setRemoveDuplicateAPs(false);    // não remova APs duplicados (true)
    wm.setMinimumSignalQuality(20);     // RSSI mínimo em % a ser exibido em scans, null = 8%
    wm.setShowInfoErase(false);         // não exibir o botão erase
    wm.setScanDispPerc(false);          // exibir RSSI como % e não ícones gráficos

    bool res;
    res = wm.autoConnect("Setup");
    if(!res) {
        Serial.println("Falha na conexão WiFi");
    }
}

void wifiManager::wifiLoop(void){
    //loop de controle de WiFi
    wm.setConfigPortalTimeout(timeout);
    if (!wm.startConfigPortal(_hostname)) {
        Serial.println("falha de conexão e tempo limite excedido");
        delay(3000);
        ESP.restart();
        delay(5000);
    }
    Serial.println("Conectado!");
}

void wifiManager::saveParamCallback(){
    //Callback para gravação de parâmetro de WiFi
    Serial.println("[CALLBACK] saveParamCallback acionada");
    Serial.println("PARAMETRO gravado: " + getParam("customfieldid"));
}

String wifiManager::getParam(String name){
    //Lê o parâmetro do servidor para entrada customhmtl
    String value;
    if(wm.server->hasArg(name)) {
      value = wm.server->arg(name);
    }
    return value;
}

//Obtém a intensidade do sinal wifi em %
int wifiManager::getWifiSignal(void){
    long rssi = WiFi.RSSI();
    //rssi varia de -90 a -30, convertendo para 0 a 100:
    int rssipc = map(rssi, -90, -30, 0, 100); 
    rssipc = constrain (rssipc, 0, 100);
    return rssipc;
}
