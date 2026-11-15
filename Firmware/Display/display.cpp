#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Wire.h>

#include "display.h"

#define DISPLAY_WIDTH 128 // Largura do display em pixels
#define DISPLAY_HEIGHT 64 // Altura do display em pixels
#define DISPLAY_RESET_PIN -1 //-1 para não uso de pino de reset
#define DISPLAY_ADDRESS 0x3C //Endereço I2C do display

//Construtores
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, DISPLAY_RESET_PIN);
U8G2_FOR_ADAFRUIT_GFX u8g2;

//Construtor da classe
Display::Display(void){
    
}

int displayFontH = 1;
int displayLines[]{26, 41, 56, 71};

const int statusBarHeight = 15;
int yInit = statusBarHeight + 1;
int yEnd = DISPLAY_HEIGHT - yInit + 1;

//Ícone MedicomLibras

// meteorologyBW  48 x 48  (mono, SSD1306 vertical pages, LSB=top)
const int meteorologyBW_width  = 48;
const int meteorologyBW_height = 48;
const unsigned char meteorologyBW[] PROGMEM = {
0xFC,0xFD,0xFF,0xFF,0xFF,0x3F,
  0x06,0x00,0x80,0x01,0x00,0x70,
  0x03,0x00,0x80,0x01,0x00,0xC0,
  0x03,0x10,0x80,0x01,0x00,0x83,
  0x01,0x10,0x80,0x01,0x00,0x87,
  0x01,0x10,0x80,0x01,0x00,0x86,
  0xC1,0x00,0x84,0xE1,0xFF,0x83,
  0x81,0x00,0x86,0xE1,0xFF,0x81,
  0x01,0x7C,0x80,0x01,0x00,0x80,
  0x01,0xC6,0x80,0xE1,0xE3,0x83,
  0x01,0x83,0x81,0xE1,0xE7,0x87,
  0x01,0x03,0x81,0x01,0x00,0x80,
  0x31,0x01,0xB9,0xE1,0x7F,0x80,
  0x01,0x03,0x81,0xE1,0xFF,0x80,
  0x01,0x82,0x81,0x01,0x00,0x80,
  0x01,0xCE,0x80,0xE1,0x01,0x80,
  0x01,0x78,0x80,0xE1,0x03,0x80,
  0x81,0x00,0x86,0x01,0x03,0x80,
  0xC1,0x00,0x84,0x81,0x03,0x80,
  0x01,0x10,0x80,0x81,0x00,0x80,
  0x01,0x10,0x80,0x01,0x00,0x80,
  0x01,0x00,0x80,0x01,0x00,0x80,
  0x01,0x00,0x80,0x01,0x00,0x80,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0x01,0x00,0x80,0x01,0x00,0x80,
  0x01,0x00,0x80,0x01,0x00,0x80,
  0x01,0x00,0x83,0x01,0x00,0x80,
  0x01,0x80,0x87,0x01,0x00,0x80,
  0x01,0x80,0x84,0x01,0x0F,0x80,
  0x01,0xC0,0x8C,0x81,0x19,0x80,
  0x01,0x40,0x88,0xC1,0x30,0x80,
  0x01,0x40,0x98,0x41,0xF0,0x81,
  0x01,0x4E,0x98,0x41,0x00,0x81,
  0x01,0xCB,0x8C,0x61,0x00,0x83,
  0x81,0x99,0x87,0x31,0x00,0x87,
  0x81,0x30,0x80,0x19,0x00,0x8C,
  0xC1,0x30,0x80,0x19,0x00,0x88,
  0x41,0x60,0x80,0x19,0x00,0x8C,
  0x41,0x60,0x80,0x31,0x00,0x8C,
  0x61,0x60,0x80,0xE1,0xFF,0x87,
  0x41,0x60,0x80,0x01,0x00,0x80,
  0xC1,0x20,0x80,0x01,0x00,0x80,
  0x81,0x3B,0x80,0x01,0x00,0x80,
  0x03,0x0F,0x80,0x01,0x00,0xC0,
  0x03,0x00,0x80,0x01,0x00,0xC0,
  0x0E,0x00,0x80,0x01,0x00,0x70,
  0xFC,0xFF,0xFF,0xFF,0xCF,0x3F
};



void Display::displayInit(void){
    Wire.begin(SDA, SCL);
    Wire.setClock(400000); // Define velocidade de 400kHz para o SSD1306
    if(!display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS)) {
        Serial.println(F("Falha na alocação do display"));
    }
    u8g2.begin(display);
    display.clearDisplay();
    displayLogo();
    delay(1000);
    display.display();
}

void Display::statusBar(int wifiConn, bool qrCodeRead, char* dia, char* hora){
    display.fillRect(0, 0, 128, 16, SSD1306_BLACK);//apaga a área da barra de status
    display.setTextColor(SSD1306_BLACK);
    //exibe intensidade do sinal wifi
    u8g2.setFont(u8g2_font_siji_t_6x10);
    int y = ( 16 - ( u8g2.getFontAscent() - u8g2.getFontDescent() ) ) / 2; //centraliza a fonte
    //intensidade do sinal wifi
    if (wifiConn > 66) {       
        u8g2.drawGlyph(0, 10, 0x0e04b);
    }
    else if (wifiConn > 33 && wifiConn <= 66) {
        u8g2.drawGlyph(0, 10, 0x0e04a);
    }
    else {
        u8g2.drawGlyph(0, 10, 0x0e049);
    }
    //relógio
    std::string relogioStr = std::string(dia) + " - " + std::string(hora);
    const char* relogio = relogioStr.c_str();
    u8g2.setFont(u8g2_font_rosencrantz_nbp_tr);
    y = ( 16 - ( u8g2.getFontAscent() - u8g2.getFontDescent() ) ) / 2; //centraliza a fonte
    u8g2.setCursor( 127 - u8g2.getUTF8Width(relogio), 12 );//posicionando o relógio à direita do display
    u8g2.println(relogio);
    display.display();
}

void Display::displayPrint(const char* text, int line) {
    u8g2.setFont(u8g2_font_helvR08_te);
    int lineHeight = u8g2.getFontAscent() - u8g2.getFontDescent() + 1;
    int baseLineY = u8g2.getFontAscent() + yInit;
    int lineY = baseLineY + line * lineHeight;
    int current_x = 0;
    int maxLines = (yEnd - yInit) / lineHeight;
    display.fillRect(0, yInit + lineHeight * line - 1, DISPLAY_WIDTH, yEnd, SSD1306_BLACK); // Apaga a área de escrita
    
    char *word;
    char temp_text[1024];
    strncpy(temp_text, text, sizeof(temp_text) - 1);
    temp_text[sizeof(temp_text) - 1] = '\0';

    word = strtok(temp_text, " ");

    while (word != NULL) {
        int word_width = u8g2.getUTF8Width(word);
        int space_width = u8g2.getUTF8Width(" ");

        if (current_x + word_width > 128) {
            line++;
            if (line >= maxLines) {
                break;
            }
            lineY = baseLineY + line * lineHeight;
            current_x = 0;
        }

        u8g2.setCursor(current_x, lineY);
        u8g2.print(word);
        current_x += word_width + space_width;

        word = strtok(NULL, " ");
    }
    display.display();
}

void Display::displayLogo(void){
    display.fillRect(0, 16, 128, 48, SSD1306_BLACK);//apaga a área de escrita
    display.drawXBitmap(( DISPLAY_WIDTH - 48 ) / 2, 16, meteorologyBW, 48, 48, SSD1306_WHITE);

    // display.drawBitmap(( DISPLAY_WIDTH - 48 ) / 2, 16, meteorologyBW, 48, 48, SSD1306_WHITE);
    display.display();
}

void Display::displayProgressBar(unsigned int progress){
    float progresspc = (float)(progress) / 100;
    int barra = (127) * progresspc;

    display.drawRect(0, 50, 128, 14, SSD1306_WHITE);
    display.fillRect(0, 50, barra, 14, SSD1306_WHITE);
    display.display();
}