// ARDUBOY VERSION
#include <SPI.h>

#define CLK 18
#define MOSI 19
#define MISO 16
#define DC 17
#define CS 16
#define RST 20

const uint8_t PROGMEM display_init_data[] = {
  0xAE,0x20,0x00,0x40,0xA1,0xA8,39,0xC8,0xD3,0x00,0xDA,0x12,0xD5,0x80,0xD9,
  0xF1,0xDB,0x20,0x81,0x7F,0xA4,0xA6,0x8D,
  0x14,0xAD,0x30,0xAF,
  0x21, 28,99, // horizontal range
  0x22, 0,4 // vertical range
};

int button_pins[6] = {4,6,3,5,24,27}; // up,down,left,right,A,B


arduino::MbedSPI _spi(4, 19, 18);

void core_init()
{  
  pinMode(CLK, OUTPUT);
  pinMode(DC, OUTPUT);
  pinMode(CS, OUTPUT);
  // Setup reset pin direction (used by both SPI and I2C)  
  pinMode(RST, OUTPUT);
  digitalWrite(RST, HIGH);
  delay(1);       // VDD (3.3V) goes high at start, lets just chill for a ms
  digitalWrite(RST, LOW); // bring reset low
  delay(10);        // wait 10ms
  digitalWrite(RST, HIGH);  // bring out of reset
  
  _spi.begin();
  _spi.beginTransaction(SPISettings(10*1024*1024, MSBFIRST, SPI_MODE0));

  for (int i=0; i<6; i++) pinMode(button_pins[i], INPUT_PULLUP);
 
  digitalWrite(CS,HIGH);
  digitalWrite(DC,LOW);
  digitalWrite(CS,LOW); // into intruction mode
  
  for (int i = 0; i < sizeof(display_init_data); i++)
  {  
    int a=pgm_read_byte(&display_init_data[i]);
    _spi.transfer(a);
  }
}

void display_dataMode(void){
  digitalWrite(CS,HIGH);
  digitalWrite(DC,HIGH);
  digitalWrite(CS,LOW);
}



//-------------------------------------------------------------------------------------------------------------------

void display_init(int mode,int x,int y){
 digitalWrite(CS,HIGH);  
 digitalWrite(DC,LOW);  
 digitalWrite(CS,LOW);
    
  _spi.transfer(0x20); // Set Memory Mode v
  _spi.transfer(mode); //   horizontal or vertical  Addressing
  _spi.transfer(0x21); _spi.transfer(x+28); _spi.transfer(99); // rows 0-71 
  _spi.transfer(0x22); _spi.transfer(y); _spi.transfer(4); // cols 0-4
  digitalWrite(CS,HIGH);
  

  display_dataMode();
}

//------------------------------
void ssd1306_xfer_start(void){
  digitalWrite(DC,HIGH); 
  digitalWrite(CS,LOW);
}

void ssd1306_xfer_stop(void){ 
}

//----------------------------------------
void i2c_sendByte(uint8_t byte){
   _spi.transfer(byte); 
}


void ssd1306_send_command(uint8_t command){
  digitalWrite(CS,HIGH);
  digitalWrite(DC,LOW);
  digitalWrite(CS,LOW);
  _spi.transfer(command);
  digitalWrite(CS, HIGH); // pull up the CS pin
}


void i2c_stop(void){
}

//-------------------------------------------------------------------------------------------------------------------

void arduboy_screen_wipe(){
   int i;
  
  digitalWrite(CS,HIGH);
  digitalWrite(DC,LOW);
  digitalWrite(CS, LOW); //pull down the CS pin
  _spi.transfer(0x20); // Set Memory Mode v
  _spi.transfer(0); //   Horizontal Addressing
  _spi.transfer(0x21); _spi.transfer(28); _spi.transfer(99); // rows 28-99
  _spi.transfer(0x22); _spi.transfer(0); _spi.transfer(4); // cols 0-4

  digitalWrite(CS, HIGH); // pull up the CS pin
  digitalWrite(DC,HIGH);
  digitalWrite(CS,LOW);
  for (i=0; i<72*5; i++) _spi.transfer(128);
  digitalWrite(CS, HIGH); // pull up the CS pin
}
