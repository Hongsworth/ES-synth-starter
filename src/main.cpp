#include <Arduino.h>
#include <U8g2lib.h>

//Constants
  const uint32_t interval = 100; //Display update interval

  volatile int32_t currentStepSize;
  int keyArray[7];

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
   //only need these but a more elegant solution is preferred. 
   //this is a brute force approach that is a placeholder for later (not a priority. )
  if (n == 0)
  {
    return "000";
  }
  if (n == 1){
    return "001";

  }
  if (n == 2){
    return "010";
  }
    
}

void setRow(uint8_t rowIdx){

  //make more efficient and optimal
  // parse the incoming value/ turn into binary, 
  std::string binaryIdx = toBinary(rowIdx);

  int ra0, ra1, ra2; 

  ra0 = (binaryIdx[2] == '1') ? HIGH : LOW; //set values of ra pins 
  ra1 = (binaryIdx[1] == '1') ? HIGH : LOW;
  ra2 = (binaryIdx[0] == '1') ? HIGH : LOW;

  (binaryIdx[0] == '1') ? Serial.print("1"): Serial.print("0");
  (binaryIdx[1] == '1') ? Serial.print("1"): Serial.print("0");
  (binaryIdx[2] == '1') ? Serial.print("1"): Serial.print("0");



  digitalWrite(REN_PIN, LOW); //write low at start
  digitalWrite(RA0_PIN, ra0); //write ra pin values determined from ternary block.
  digitalWrite(RA1_PIN, ra1);
  digitalWrite(RA2_PIN, ra2);
  digitalWrite(REN_PIN, HIGH); //write high at end.

}

uint8_t readCols(uint8_t row){
  
  //set ra0 - 2 pins to low, ren to high
  // digitalWrite(RA0_PIN, LOW );
  // digitalWrite(RA1_PIN, LOW );
  // digitalWrite(RA2_PIN, LOW );
  // digitalWrite(REN_PIN,HIGH);
  setRow(row);
  
  delay(0.5);

  int c0 = digitalRead(C0_PIN);
  int c1 = digitalRead(C1_PIN);
  int c2 = digitalRead(C2_PIN);
  int c3 = digitalRead(C3_PIN);

  //LOW WHEN PRESSED
  
  std::string out = "";
  uint8_t output;
  
// (c0 == LOW)?  Serial.print("1") : Serial.print("0"); 
// (c1 == LOW)?  Serial.print("1") : Serial.print("0"); 
// (c2 == LOW)?  Serial.print("1") : Serial.print("0"); 
// (c3 == LOW)?  Serial.print("1") : Serial.print("0"); 

  output += (c0 == HIGH)?  pow(2, 7): 0; 
  output += (c1 == HIGH)?  pow(2, 6): 0; 
  output += (c2 == HIGH)?  pow(2, 5): 0; 
  output += (c3 == HIGH)?  pow(2, 4): 0; 
  Serial.println();

  return output;

}

void sampleISR() {
  static int32_t phaseAcc = 0;
  phaseAcc += currentStepSize;

  int32_t Vout = phaseAcc >> 19; //change for volume to increase! (12 is the highest I reccomend, quite loud )

  analogWrite(OUTR_PIN, Vout + 128);
}

void setup() {
  // put your setup code here, to run once:

  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);

  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

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

std::string noteSelect(int keyArr[7], const int32_t stepSizes[12]){
  for (int i =0; i < 3; i++){
    
    if (keyArr[i] == 112){
      if (i == 0) {currentStepSize = stepSizes[0]; return "C";}
      if (i == 1) {currentStepSize = stepSizes[1]; return "E";}
      if (i == 2) {currentStepSize = stepSizes[2]; return "G#";}
    }
    if (keyArr[i] == 208){
      if (i == 0) {currentStepSize = stepSizes[3]; return "D";}
      if (i == 1) {currentStepSize = stepSizes[4]; return "F#";}
      if (i == 2) {currentStepSize = stepSizes[5]; return "A#";}
    }
    if (keyArr[i] == 176){
      if (i == 0) {currentStepSize = stepSizes[6]; return "C#";}
      if (i == 1) {currentStepSize = stepSizes[7]; return "F";}
      if (i == 2) {currentStepSize = stepSizes[8]; return "A";}
    }
    if (keyArr[i] == 224){
      if (i == 0) {currentStepSize = stepSizes[9]; return "D#";}
      if (i == 1) {currentStepSize = stepSizes[10]; return "G";}
      if (i == 2) {currentStepSize = stepSizes[11]; return "B";}
    }
  }
  currentStepSize = 0;
  return "No Note";
}

void loop() {
  // put your main code here, to run repeatedly:
  const float diff = pow(2, 1/12);
  const int32_t base = 2^32;
  static uint32_t next = millis();
  static uint32_t count = 0;
  const int32_t stepSizes[12] = {base * (440 - 8*diff), base * (440 - 7*diff), base * (440 - 6*diff),
  base * (440 - 5*diff), base * (440 - 4*diff), base * (440 - 3*diff), base * (440 - 2*diff), base * (440 - 1*diff),
  base * (440), base * (440 + 1*diff), base * (440 + 2*diff), base * (440 + 3*diff)};
 

  

  
  

  for (int i = 0; i <= 2; i++)
  {
    keyArray[i] = readCols(i);
    delay(3);
  }

  

  if (millis() > next) {
    next += interval;

    

    //Update display
    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(2,10,"Hello World!");  // write something to the internal memory
    u8g2.setCursor(2,20);
    //u8g2.print(count++);

    u8g2.setCursor(2,20);
    u8g2.print(keyArray[0],HEX); 
    //u8g2.sendBuffer();          // transfer internal memory to the display
    delay(3);
    u8g2.setCursor(22,20);
    u8g2.print(keyArray[1],HEX); 
    //u8g2.sendBuffer();    
    delay(3);
    u8g2.setCursor(42,20);
    u8g2.print(keyArray[2],HEX);


  
  //prints the current note. Strings not compatible (WHYYYY????) So have to do this tedious
  //char conversion. Would use pointers but causes headaches. 
  std::string note = noteSelect(keyArray, stepSizes);
  for (int i = 0; i < note.size(); i++){
    char a = note[i];
    u8g2.setCursor(62 + i*5, 20);
    u8g2.print(a); 
  }






    u8g2.sendBuffer();  
    delay(3);

    

    //Toggle LED
    digitalToggle(LED_BUILTIN);

    

    if (count > 200){
      exit(0);
    }
    
    
  }
}