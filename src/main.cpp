#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ES_CAN.h>
extern "C" void CAN1_RX0_IRQHandler(void);

const uint32_t TX_Interval = 1000;

//Pin definitions

//Row select and enable
const int RA0_PIN = D3;
const int RA1_PIN = D6;
const int RA2_PIN = D12;
const int REN_PIN = A5;

//Matrix input and output
const int C0_PIN = A2;
const int C1_PIN = D9;
const int C2_PIN = A6;
const int C3_PIN = D1;
const int OUT_PIN = D11;

//Audio analogue out
const int OUTL_PIN = A4;
const int OUTR_PIN = A3;

//Joystick analogue in
const int JOYY_PIN = A0;
const int JOYX_PIN = A1;

//Output multiplexer bits
const int DEN_BIT = 3;
const int DRST_BIT = 4;
const int HKOW_BIT = 5;
const int HKOE_BIT = 6;

//Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

//Function to set outputs via matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value) {
      digitalWrite(REN_PIN,LOW);
      digitalWrite(RA0_PIN, bitIdx & 0x01);
      digitalWrite(RA1_PIN, bitIdx & 0x02);
      digitalWrite(RA2_PIN, bitIdx & 0x04);
      digitalWrite(OUT_PIN,value);
      digitalWrite(REN_PIN,HIGH);
      delayMicroseconds(2);
      digitalWrite(REN_PIN,LOW);
}

uint8_t RX_data[8] = {0,0,0,0,0,0,0,0};
void CAN_ISR() {
  uint32_t ID;
  CAN_RX(ID,RX_data);
  digitalToggle(LED_BUILTIN);
}

void setup() {
  // put your setup code here, to run once:

  //Set pin directions
  pinMode(RA0_PIN, OUTPUT);
  pinMode(RA1_PIN, OUTPUT);
  pinMode(RA2_PIN, OUTPUT);
  pinMode(REN_PIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(OUTL_PIN, OUTPUT);
  pinMode(OUTR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(C0_PIN, INPUT);
  pinMode(C1_PIN, INPUT);
  pinMode(C2_PIN, INPUT);
  pinMode(C3_PIN, INPUT);
  pinMode(JOYX_PIN, INPUT);
  pinMode(JOYY_PIN, INPUT);

  //Initialise display
  setOutMuxBit(DRST_BIT, LOW);  //Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH);  //Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  //Enable display power supply

  //Initialise UART
  Serial.begin(9600);
  Serial.println("Hello World");

  //Initialise CAN
  uint32_t status = CAN_Init();
  Serial.print(status);

  status = setCANFilter();
  Serial.print(status);

  CAN_RegisterISR(CAN_ISR);

  status = CAN_Start();
  Serial.print(status);

}

void loop() {
  // put your main code here, to run repeatedly:
  static uint32_t nextTX = millis();
  static uint8_t dataByte = 0;
  uint8_t TX_data[8];

  if (CAN_CheckRXLevel()){
    uint32_t RX_ID;
    CAN_RX(RX_ID, RX_data);

    Serial.print(RX_ID);
    for (int i=0; i<8; i++)
      Serial.print(RX_data[i]);
    Serial.println();

  }

  if (millis() > nextTX) {
    nextTX += TX_Interval;

    //Update display
    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(2,10,"Helllo World!");  // write something to the internal memory
    u8g2.setCursor(2,20);
    
    for (int i=0; i<8; i++)
      u8g2.print(RX_data[i],HEX);
    u8g2.sendBuffer();          // transfer internal memory to the display

    for (int i=0; i<8; i++)
      TX_data[i] = dataByte++;

    //CAN_TX(0x123,TX_data);

    //Toggle LED
    //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}