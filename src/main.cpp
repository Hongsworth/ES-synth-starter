#include <Arduino.h>
#include <U8g2lib.h>

//Constants
  const uint32_t interval = 100; //Display update interval

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

//Function to set outputs using key matrix
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

std::string toBinary(int n)
{
    std::string r;
    while(n!=0) {r=(n%2==0 ?"0":"1")+r; n/=2;}
    return r;
}

void setRow(uint8_t rowIdx){

  //make more efficient and optimal
  // parse the incoming value/ turn into binary, 
  std::string binaryIdx = toBinary(rowIdx);

  int ra0, ra1, ra2; 

  ra0 = (binaryIdx[0] == '1') ? HIGH : LOW; //set values of ra pins 
  ra1 = (binaryIdx[1] == '1') ? HIGH : LOW;
  ra2 = (binaryIdx[2] == '1') ? HIGH : LOW;

  digitalWrite(REN_PIN, LOW); //write low at start
  digitalWrite(RA0_PIN, ra0); //write ra pin values determined from ternary block.
  digitalWrite(RA1_PIN, ra1);
  digitalWrite(RA2_PIN, ra2);
  digitalWrite(REN_PIN, HIGH); //write high at end.

}

uint8_t readCols(){
  
  //set ra0 - 2 pins to low, ren to high
  // digitalWrite(RA0_PIN, LOW );
  // digitalWrite(RA1_PIN, LOW );
  // digitalWrite(RA2_PIN, LOW );
  // digitalWrite(REN_PIN,HIGH);
  setRow(0);
  

  int c0 = digitalRead(C0_PIN);
  int c1 = digitalRead(C1_PIN);
  int c2 = digitalRead(C2_PIN);
  int c3 = digitalRead(C3_PIN);

  //LOW WHEN PRESSED
  
  std::string out = "";
  uint8_t output;
  
(c0 == LOW)?  Serial.print("1") : Serial.print("0"); 
(c1 == LOW)?  Serial.print("1") : Serial.print("0"); 
(c2 == LOW)?  Serial.print("1") : Serial.print("0"); 
(c3 == LOW)?  Serial.print("1") : Serial.print("0"); 

  output += (c0 == HIGH)?  pow(2, 7): 0; 
  output += (c1 == HIGH)?  pow(2, 6): 0; 
  output += (c2 == HIGH)?  pow(2, 5): 0; 
  output += (c3 == HIGH)?  pow(2, 4): 0; 
  Serial.println();


  
  return output;

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
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint32_t next = millis();
  static uint32_t count = 0;

  if (millis() > next) {
    next += interval;

    //Update display
    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(2,10,"Helllo World!");  // write something to the internal memory
    u8g2.setCursor(2,20);
    //u8g2.print(count++);
    uint8_t keys = readCols();
    u8g2.setCursor(2,20);
    u8g2.print(keys,HEX); 
    u8g2.sendBuffer();          // transfer internal memory to the display

    //Toggle LED
    digitalToggle(LED_BUILTIN);

    readCols();

    if (count > 200){
      exit(0);
    }
    
    
  }
}