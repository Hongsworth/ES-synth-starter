#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>
#include <cmath>
#include <chrono>
#include <ES_CAN.h>

//Constants
  const uint32_t interval = 100; //Display update interval

  bool reciever = false;
  bool soloMode = false;
  char prevMessage = 'R';

  int OCTAVE = 6;
  int originalOCTAVE;

  volatile int KEYNUM = 0;
  volatile uint8_t currentKey=0;
  volatile uint32_t currentFreq;
  volatile uint32_t currentPeriod;
  volatile int32_t currentStepSize = 0;
  volatile int32_t currentStepSizes[3] = {0};

  volatile int32_t keyArray[7];  // has 7 rows and a 1d array
  const uint32_t base = pow(2, 32)/22000;
  uint32_t stepSizes[12] = {base * 261, base * 329, base * 415, base * 293, base * 370,
   base* 466, base*  277, base* 349, base * 440, base* 311, base* 392, base* 493}; //octave4

  uint32_t stepSizesoct2[12] = {12769719, 16088557,20270293,14333476,18058385,22751613,13529147,17045162,21474836,15184661,19132127,24104528};//{base * 65, base * 69, base * 73,  base * 78, base * 82, base * 87, base * 92, base * 98, base * 104, base * 110, base * 117, base * 123};
  uint32_t stepSizesoct3[12] = {25537484,32175161,40538633,28665002,36116770,45503226,27056340,34088374,42949672,30369322,38264254,48209056};//{base * 131, base * 139, base * 147, base * 156, base * 165, base * 175, base * 185, base * 196, base * 208, base * 220, base * 233, base * 247};
  uint32_t stepSizesoct5[12] = {102151892,128704553,162156487,114661964,144465127,182014860,108227318,136357406,171798691,121479245,153055062,192838178};//{base * 523, base * 554, base * 587,  base * 622, base * 659, base * 698, base * 740, base * 784, base * 831, base * 880, base * 932, base * 988};
  uint32_t stepSizesoct6[12] = {204303785,257407153,324312974,229323928,288930255,364029721,216452682,272712859,343597383,242960445,306110124,385674403};//{base * 1047, base * 1109, base * 1175, base * 1245, base * 1319, base * 1397, base * 1480, base * 1568, base * 1661, base * 1760, base * 1865, base * 1980};
  // octave 2 {12769719, 16088557,20270293,14333476,18058385,22751613,13529147,17045162,21474836,15184661,19132127,24104528}
  // octave 3 {25537484,32175161,40538633,28665002,36116770,45503226,27056340,34088374,42949672,30369322,38264254,48209056};
  // octave 4 {51076923,64352276,81077266,57330005,72231586,91006453,54112681,68178703,85899345,60740599,76528508,96418112};
  // octave 5 {102151892,128704553,162156487,114661964,144465127,182014860,108227318,136357406,171798691,121479245,153055062,192838178}
  // octave 6 {204303785,257407153,324312974,229323928,288930255,364029721,216452682,272712859,343597383,242960445,306110124,385674403}

  uint32_t orderedoct2[12] = {base * 65, base * 69, base * 73,  base * 78, base * 82, base * 87, base * 92, base * 98, base * 104, base * 110, base * 117, base * 123};
  uint32_t orderedoct3[12] = {base * 131, base * 139, base * 147, base * 156, base * 165, base * 175, base * 185, base * 196, base * 208, base * 220, base * 233, base * 247};
  uint32_t orderedoct5[12] = {base * 523, base * 554, base * 587,  base * 622, base * 659, base * 698, base * 740, base * 784, base * 831, base * 880, base * 932, base * 988};
  uint32_t orderedoct6[12] = {base * 1047, base * 1109, base * 1175, base * 1245, base * 1319, base * 1397, base * 1480, base * 1568, base * 1661, base * 1760, base * 1865, base * 1980};
  uint32_t orderedoct4[12] = {base * 261, base*  277, base * 293, base* 311, base * 329, base* 349, base * 370, base* 392, base * 415, base * 440, base* 466, base* 493};


   const uint32_t periods[12] = {22000/261, 22000/329, 22000/415, 22000/293, 22000/370, 
   22000/466, 22000/277, 22000/349, 22000/440, 22000/311, 22000/392, 22000/493};


  uint8_t RX_Message[8] = {0};
//useless, should remove.
   float rootRet(int power) //return a power of the root of 12
{
  return pow(2, power/12);
}

//LAB2
SemaphoreHandle_t keyArrayMutex;
//global handle for a FreeRTOS mutex that can be used by different threads to access the mutex object:
  
//CAN
QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;

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

void CAN_RX_ISR (void) {
	uint8_t RX_Message_ISR[8];
	uint32_t ID;
	CAN_RX(ID, RX_Message_ISR);
	xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

std::string toBinary(int n)
{
   //only need these but a more elegant solution is preferred. 
   //this is a brute force approach that is a placeholder for later (not a priority. )
  if (n == 0){
    return "000";
  }
  if (n == 1){
    return "001";
  }
  if (n == 2){
    return "010";
  }

  if (n == 3){
    return "011";
  }

  if (n == 4){
    return "100";
  }

  if (n == 5){
    return "101";
  }
    
}

std::string noteSelect(){
std::string notes = "";
int j = 0;
uint32_t stepSizesLoc[12];
//Serial.println(OCTAVE);
if (OCTAVE == 2){
    for (int i = 0; i < 12; i++)
    {stepSizesLoc[i] = stepSizesoct2[i];}
  }
  if (OCTAVE == 3){
    for (int i = 0; i < 12; i++)
    {stepSizesLoc[i] = stepSizesoct3[i];}
  }
  if (OCTAVE == 4){
    for (int i = 0; i < 12; i++)
    {stepSizesLoc[i] = stepSizes[i];}
    
  }
  if (OCTAVE == 5){
    for (int i = 0; i < 12; i++)
    {stepSizesLoc[i] = stepSizesoct5[i];}
  }
  if (OCTAVE == 6){
    for (int i = 0; i < 12; i++)
    {stepSizesLoc[i] = stepSizesoct6[i];}
  }

  for (int i =0; i < 3; i++){

    
        
      if (keyArray[i] == 112){
        if (i == 0) {; currentKey = 0; currentStepSizes[j] = stepSizesLoc[0]; ;notes+="C";j ++;}
        if (i == 1) {;currentKey = 1; currentStepSizes[j] = stepSizesLoc[1]; ;notes+= "E";j ++;}
        if (i == 2) {;currentKey = 2; currentStepSizes[j] = stepSizesLoc[2]; ;notes+= "G#";j ++;}
        
      } //c2
      if (keyArray[i] == 208){
        if (i == 0) {currentKey = 3;currentStepSizes[j] = stepSizesLoc[3]; notes+= "D";j ++;}
        if (i == 1) {currentKey = 4;currentStepSizes[j] = stepSizesLoc[4]; notes+= "F#";j ++;}
        if (i == 2) {currentKey = 5;currentStepSizes[j] = stepSizesLoc[5]; notes+= "A#";j ++;}

        
      } //c1
      if (keyArray[i] == 176){
        if (i == 0) {currentKey = 6;currentStepSizes[j] = stepSizesLoc[6]; notes+= "C#";j ++;}
        if (i == 1) {currentKey = 7;currentStepSizes[j] = stepSizesLoc[7]; notes+= "F";j ++;}
        if (i == 2) {currentKey = 8;currentStepSizes[j] = stepSizesLoc[8]; notes+= "A";j ++;}

      
      } //c3
      if (keyArray[i] == 224){
        if (i == 0) {currentKey = 9;currentStepSizes[j] = stepSizesLoc[9]; notes+= "D#";j ++;}
        if (i == 1) {currentKey = 10;currentStepSizes[j] = stepSizesLoc[10]; notes+= "G";j ++;}
        if (i == 2) {currentKey = 11;currentStepSizes[j] =stepSizesLoc[11]; notes+= "B"; j++;}

       
      }
      
   
      
  
}

if (keyArray[0] == 240 && keyArray[1] == 240 && keyArray[2] == 240 && (prevMessage != 'P' || soloMode))
{
  currentStepSizes[0] = 0;
}


//Serial.println(currentKey);
  // if (j == 0){
  //   currentStepSizes[0] = 0;
  //   currentStepSizes[1] = 0;
  //   currentStepSizes[2] = 0;
  // }
  // if (j == 1){
  //   currentStepSizes[1] = 0;
  //   currentStepSizes[2] = 0;
  // }
  // if (j == 2){
  //   currentStepSizes[2] = 0;
  // }
  return notes;
}


void setRow(uint8_t rowIdx){

  //make more efficient and optimal
  // parse the incoming value/ turn into binary, 
  std::string binaryIdx = toBinary(rowIdx);

  int ra0, ra1, ra2 = LOW;

  // ra0 = (binaryIdx[2] == '1') ? HIGH : LOW; //set values of ra pins 
  // ra1 = (binaryIdx[1] == '1') ? HIGH : LOW;
  // ra2 = (binaryIdx[0] == '1') ? HIGH : LOW;

  if (binaryIdx[2] == '1') {ra0 = HIGH;}
  else {ra0 = LOW;}
  if (binaryIdx[1] == '1') {ra1 = HIGH;}
  else {ra1 = LOW;}
  if (binaryIdx[0] == '1') {ra2 = HIGH;}
  else {ra2 = LOW;}

  // (binaryIdx[0] == '1') ? Serial.print("1"): Serial.print("0");
  // (binaryIdx[1] == '1') ? Serial.print("1"): Serial.print("0");
  // (binaryIdx[2] == '1') ? Serial.print("1"): Serial.print("0");
  
  digitalWrite(REN_PIN, LOW); //write low at start
  digitalWrite(RA0_PIN, ra0); //write ra pin values determined from ternary block.
  digitalWrite(RA1_PIN, ra1);
  digitalWrite(RA2_PIN, ra2);
  digitalWrite(REN_PIN, HIGH); //write high at end.
  

}

std::string decodeNote(){
  int noteNum = RX_Message[2];

  Serial.println("Here");

  switch(noteNum){
  

    case 1: 

      return "C";
    break;
   
    case 2: 

    return "C#";
    break;
    case 3: 

      return "D";
    break;
    case 4: 

      return "D#";
    break;
    case 5: 

      return "E";
    break;
    case 6: 

      return "F";
    break;
    case 7: 
 
    return "F#";
    break;
    case 8: 

    return "G";
    break;
    case 9: 

    return "G#";
    break;
    case 10: 

    return "A";
    break;
    case 11: 

    return "A#";
    break;
    case 12: 

    return "B";
    break;


    default: 
    
    return "No Note";

  }
}



void displayUpdateTask(void * param){

  const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

    uint32_t ID;
    

  while(1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency);
    //Update display
    u8g2.clearBuffer();         // clear the internal memory

    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(2,10,"Keyboard!");  // write something to the internal memory
    u8g2.setCursor(2,20);
    //u8g2.print(count++);

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    u8g2.setCursor(2,20);
    u8g2.print(keyArray[0],HEX); 
    //u8g2.sendBuffer();          // transfer internal memory to the display
    // xSemaphoreGive(keyArrayMutex);
    // xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    u8g2.setCursor(22,20);
    u8g2.print(keyArray[1],HEX); 
    //u8g2.sendBuffer();    
    // xSemaphoreGive(keyArrayMutex);
    // xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    u8g2.setCursor(42,20);
    u8g2.print(keyArray[2],HEX);

    u8g2.drawStr(80,10,"Vol:");
    u8g2.setCursor(110,10);
    u8g2.print(keyArray[3]);

    u8g2.drawStr(2,30,"Wave:");
    u8g2.setCursor(35,30);
    u8g2.print(keyArray[4]);


    xSemaphoreGive(keyArrayMutex);

    
    while (CAN_CheckRXLevel())
	    //CAN_RX(ID, RX_Message);

    //u8g2.print((char) TX_Message[0]);
    u8g2.drawStr(30,30,"Oct"); 
    u8g2.setCursor(50,30);
    //if(currentStepSize != 0){

   
    u8g2.print(OCTAVE);
   
    //}

    u8g2.drawStr(60,30,"LastKey:"); 
    u8g2.setCursor(110,30);

    //if(currentStepSizes[0] == 0){
    u8g2.print(RX_Message[2]);
    //}
    // else{
    //   u8g2.print(KEYNUM);
    // }
    //u8g2.print();
  //std::string dummy = noteSelect();
  //prints the current note. Strings not compatible (WHYYYY????) So have to do this tedious
  //char conversion. Would use pointers but causes headaches. 
  // if(currentStepSizes[0] != 0){
   
  std::string note; 
    
if (reciever){
  note = noteSelect();
  if (currentStepSizes[0] != 0){
  
  for (int i = 0; i < note.size(); i++){
    char a = note[i];
    u8g2.setCursor(62 + i*5, 20);
    u8g2.print(a); 
   }
    }
   else if (char(RX_Message[0]) == 'P'){
  note = decodeNote();
  for (int i = 0; i < note.size(); i++){
    char a = note[i];
    u8g2.setCursor(62 + i*5, 20);
    u8g2.print(a); 
   }
   }else {
    currentStepSizes[0] = 0;
   }



    u8g2.drawStr(90,20,"REC"); 

      }
      
      else if (!reciever && !soloMode) {
        u8g2.drawStr(60,30,"TRA"); 
      }
      else if (soloMode){
        note = noteSelect();
        for (int i = 0; i < note.size(); i++){
          char a = note[i];
          u8g2.setCursor(62 + i*5, 20);
          u8g2.print(a); 
          }
          u8g2.drawStr(60,30,"SOLO"); 
        


      }

      

   //}
  
  // currentFreq = 22000*(currentStepSize>>32);
  // currentPeriod = periods[currentKey];


    //Serial.println((currentStepSize >> 24) + 128);
    u8g2.sendBuffer();  
    
 

    //Toggle LED
    digitalToggle(LED_BUILTIN);
  
  
}

}


//__________________________________________ask angelo______________________________
volatile uint32_t output = 0;

void scanKeys(void * pvParameters){
  const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  volatile int c0 = HIGH;
  volatile int c1 = HIGH;
  volatile int c2 = HIGH;
  volatile int c3 = HIGH;

  int8_t knob3Prev = 0;
  int8_t knob4Prev = 0;

  int8_t conv;

  int8_t keyNum=0;
  
  int change= 0;

  uint8_t TX_Message[8] = {0};




  while(1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    for (int i = 0; i < 6; i++)
  {
    conv = 0;
    
    setRow(i);

    c0 = digitalRead(C0_PIN);
    c1 = digitalRead(C1_PIN);
    c2 = digitalRead(C2_PIN);
    c3 = digitalRead(C3_PIN);

    output = 0;

    //removed ternary ops
    

    if (c0 == HIGH){
      output+=pow(2, 7);
      conv+=2;
      }
      else{
        change = 0;
      }
    if (c1 == HIGH){
      output+=pow(2, 6);
      conv+=1;
      }
      else{
        change = 1;
      }
    if (c2 == HIGH){
      output+=pow(2, 5);
      
    }
    else{
        change = 2;
      }
    if (c3 == HIGH){
      output+=pow(2, 4);
      
    }
    else{
        change = 3;
      }
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    if (i < 3 )
    {
    if (output < keyArray[i]){ //check the previous vs the current state (output)
    //need to check which key was pressed.
    //to this effect we can find this based on the current row and the 
    //calculated output value.
    //i.e if output == F0, no keys
    //output == 70 && i == 0 key pressed was the first (C)
      keyNum = i*4;
      if (!reciever){
      TX_Message[0] = 'P';
      TX_Message[1] = OCTAVE; //octave
      TX_Message[2] = keyNum + change;  //key num
      }
     
      KEYNUM = keyNum + change;
      //CAN_TX(0x123, TX_Message);
      //xQueueSend( msgOutQ, TX_Message, portMAX_DELAY);
    }
    if (output > keyArray[i]){
      keyNum = i*4;
      if (!reciever)
      {TX_Message[0] = 'R';
      TX_Message[1] = OCTAVE;} //octave
      //TX_Message[2] = keyNum + change;  //key num
      //CAN_TX(0x123, TX_Message); //DO NOT UPDATE KEY PRESSED HERE!!!
      //xQueueSend( msgOutQ, TX_Message, portMAX_DELAY);
    }

    keyArray[i] = output;
    }
  
     else if (i == 3){
      if (knob3Prev == 0 && conv == 1){
        knob3Prev = conv;
        if (keyArray[i] > 0){keyArray[i]-= 1;}
      }

      else if (knob3Prev == 0 && conv == 2){
        knob3Prev = conv;
      }

      else if (knob3Prev == 1 && conv == 0){
        knob3Prev = conv;
        if (keyArray[i] < 8){keyArray[i]+= 1;}
      }

      else if (knob3Prev == 1 && conv == 3){
        knob3Prev = conv;
      }

      else if (knob3Prev == 2 && conv == 3){
        knob3Prev = conv;
        if (keyArray[i] < 8){keyArray[i]+= 1;}
       
      }

      else if (knob3Prev == 2 && conv == 0){
        knob3Prev = conv;
      }

      else if (knob3Prev == 3 && conv == 2){
        knob3Prev = conv;
        if (keyArray[i] >0){keyArray[i]-= 1;}
      }

      else if (knob3Prev == 3 && conv == 1){
        knob3Prev = conv;
      }


     
      }

      else if (i == 4){
      if (knob4Prev == 0 && conv == 1){
        knob4Prev = conv;
        if (keyArray[i] > 0){keyArray[i]-= 1;}
      }

      else if (knob4Prev == 0 && conv == 2){
        knob4Prev = conv;
      }

      else if (knob4Prev == 1 && conv == 0){
        knob4Prev = conv;
        if (keyArray[i] < 8){keyArray[i]+= 1;}
      }

      else if (knob4Prev == 1 && conv == 3){
        knob4Prev = conv;
      }

      else if (knob4Prev == 2 && conv == 3){
        knob4Prev = conv;
        if (keyArray[i] < 8){keyArray[i]+= 1;}
       
      }

      else if (knob4Prev == 2 && conv == 0){
        knob4Prev = conv;
      }

      else if (knob4Prev == 3 && conv == 2){
        knob4Prev = conv;
        if (keyArray[i] >0){keyArray[i]-= 1;}
      }

      else if (knob4Prev == 3 && conv == 1){
        knob4Prev = conv;
      }


     
      }
      //for changing the wave type! //0 will be sawtooth for example. 
      else if (i == 4){
      if (knob4Prev == 0 && conv == 1){
        knob4Prev = conv;
        if (keyArray[i] > 0){keyArray[i]-= 1;}
      }

      else if (knob4Prev == 0 && conv == 2){
        knob4Prev = conv;
      }

      else if (knob4Prev == 1 && conv == 0){
        knob4Prev = conv;
        if (keyArray[i] < 8){keyArray[i]+= 1;}
      }

      else if (knob4Prev == 1 && conv == 3){
        knob4Prev = conv;
      }

      else if (knob4Prev == 2 && conv == 3){
        knob4Prev = conv;
        if (keyArray[i] < 8){keyArray[i]+= 1;}
       
      }

      else if (knob4Prev == 2 && conv == 0){
        knob4Prev = conv;
      }

      else if (knob4Prev == 3 && conv == 2){
        knob4Prev = conv;
        if (keyArray[i] >0){keyArray[i]-= 1;}
      }

      else if (knob4Prev == 3 && conv == 1){
        knob4Prev = conv;
      }


      //knob3Prev = conv;
      }
    xSemaphoreGive(keyArrayMutex);
    }
    TX_Message[5] = keyArray[3]; //volume is txmessage 5 now
    xQueueSend( msgOutQ, TX_Message, portMAX_DELAY);
  }

}

//___________________________________ ask angelo_______________________________________


uint8_t readCols(uint8_t row){
  
  setRow(row);
  
  delay(0.5);

  int c0 = digitalRead(C0_PIN);
  int c1 = digitalRead(C1_PIN);
  int c2 = digitalRead(C2_PIN);
  int c3 = digitalRead(C3_PIN);

  //LOW WHEN PRESSED_________________________________________________________________________
  
  std::string out = "";
  uint8_t output;

  output += (c0 == HIGH)?  pow(2, 7): 0; 
  output += (c1 == HIGH)?  pow(2, 6): 0; 
  output += (c2 == HIGH)?  pow(2, 5): 0; 
  output += (c3 == HIGH)?  pow(2, 4): 0; 

  return output;

}

static uint32_t timer = 0;

void sampleISR() { // so this is added because the key is only shown up on the display but doesn't give audio output, thats where this function comes in.
  static uint32_t phaseAcc = 0; //so this being static means that it is only initialised at the start of the program.
  static uint32_t phaseAcc2 = 0; 
  static uint32_t phaseAcc3 = 0; 
  int j=0;

  //This is for the sawtooth function
  uint32_t nonstatPhase;//pow(2, 32);

  
  //It generates a linear line. 
  if (keyArray[4] == 0)
  {
    phaseAcc += currentStepSizes[0];

    int32_t Vout = ((phaseAcc) >> 24) - 128; 

    Vout = Vout >> (8 - keyArray[3]);

    analogWrite(OUTR_PIN, Vout + 128);

    j++;
    if(j>22000){
    phaseAcc=0;
    }

  }
  else if (keyArray[4] == 1 && currentStepSize != 0)
  {
    //period of one wave is 1/440, 0.002 seconds approx. 
    //this is equivalent to 50 / 22000

    //uint32_t freq = currentFreq;

    uint32_t period = periods[currentKey];


    if (timer%period < period/2){
      nonstatPhase = pow(2, 32);
    }
    else {nonstatPhase = 0;}

    int32_t Vout = (nonstatPhase >> 24) - 128; 

  Vout = Vout >> (8 - keyArray[3]);

  analogWrite(OUTR_PIN, Vout + 128);
  }

  else if (keyArray[4] == 2 && currentStepSize != 0){
    //weird bug, fix!
    //in progress, sounds like square wave?

    uint32_t period = periods[currentKey];


    if (timer%period < period/2){ 
      nonstatPhase += currentStepSize;
    }
    else {nonstatPhase -= currentStepSize;}

    int32_t Vout = (nonstatPhase >> 24) - 128; 

    Vout = Vout >> (8 - keyArray[3]);

    analogWrite(OUTR_PIN, Vout + 128);
  }

  else {nonstatPhase = 0;}


timer += 1;

  
}

void decodeTask(void * param){
  //Serial.begin(9600);

  while(1){
    xQueueReceive(msgInQ, RX_Message, portMAX_DELAY);
    //Serial.println(char(RX_Message[0]));
      
      if (char(RX_Message[0]) == 'R' && reciever && prevMessage != 'R'){
        currentStepSizes[0] = 0;
        prevMessage = 'R';
        OCTAVE = originalOCTAVE;
      }
      

      if (char(RX_Message[0]) == 'P' && reciever && prevMessage != 'P'){
        OCTAVE = RX_Message[1];
        int keyNum = RX_Message[2];
        
        int vol = RX_Message[5];
        uint32_t stepSizesLoc[12];
//Serial.println(OCTAVE);
        if (OCTAVE == 2){
            for (int i = 0; i < 12; i++)
            {stepSizesLoc[i] = orderedoct2[i];}
          }
          if (OCTAVE == 3){
            for (int i = 0; i < 12; i++)
            {stepSizesLoc[i] = orderedoct3[i];}
          }
          if (OCTAVE == 4){
            for (int i = 0; i < 12; i++)
            {stepSizesLoc[i] = orderedoct4[i];}
            
          }
          if (OCTAVE == 5){
            for (int i = 0; i < 12; i++)
            {stepSizesLoc[i] = orderedoct5[i];}
          }
          if (OCTAVE == 6){
            for (int i = 0; i < 12; i++)
            {stepSizesLoc[i] = orderedoct6[i];}
          }
        

        currentStepSizes[0] = stepSizesLoc[keyNum];
        prevMessage = 'P';
      
        //currentStepSize = stepSizes[keyNum];
        //currentStepSize *= pow(2, octave - 4);
      }


  }

}

void drawRect(int x, int y, int height, int width){
  for (int i = x; i < x+width; i++){
    for (int j = y; j < y+height; j++){
      u8g2.drawPixel(i, j);
    }
  }
}



void onStart(int offset){ //A short demo function that does nothing. 
  u8g2.clearBuffer(); 
   u8g2.setFont(u8g2_font_ncenB08_tr);
  
  drawRect(5, 5, 5, 140);
  drawRect(5, 25, 5, 140);
  
  u8g2.drawStr(0+offset, 20, "SynthMaster"); 
  u8g2.sendBuffer(); 
}

int prevState = 0;
int prevJoyY = 500;
int prevJoyX = 500;
int prevC2State = HIGH;


int readJoyStick(){
  setRow(5);
  //delay(0.5);
  int C2State = digitalRead(C2_PIN);
  double joyX =  analogRead(JOYX_PIN);
  double joyY =  analogRead(JOYY_PIN);
  
  u8g2.setCursor(130, 30); 
  u8g2.print(joyY); 

  if (joyY < 150 && prevJoyY > 150 && prevState > 1){
  prevState -= 2;
 }

  else if (joyY > 850 && prevJoyY < 850 && prevState < 4){
  prevState += 2;
 }

 if (joyX < 150 && prevJoyX > 150 && prevState%2 == 0)
 {
  prevState += 1;
 }
 if (joyX > 850 && prevJoyX <850 && prevState%2 == 1)
 {
  prevState -= 1;
 }
 

 if (prevState == 0){
  u8g2.drawStr( 5, 10, ">"); 
 }

 if (prevState == 2){
  u8g2.drawStr( 5, 20, ">"); 
 }

 if (prevState == 4){
  u8g2.drawStr( 5, 30, ">"); 
 }
 if (prevState == 1){
  u8g2.drawStr( 80, 10, ">"); 
 }
 if (prevState == 3){
  u8g2.drawStr( 80, 20, ">"); 
 }
 if (prevState == 5){
  u8g2.drawStr( 80, 30, ">"); 
 }
 

if (C2State == LOW ){
  
  return prevState;
}
 prevJoyY = joyY;
 prevJoyX = joyX;
 //prevC2State = C2State;
return -1;

}

int selection = -1;
bool start = false;
void setupMenu(){


//readJoyStick();
  u8g2.clearBuffer(); 
   u8g2.setFont(u8g2_font_ncenB08_tr);
  
  if (selection != -1){
    if (selection == 0){
      u8g2.drawStr( 10, 20, "Receiver Selected"); 
      delay(1000);
      start = true;
      reciever = true;
      //soloMode = true;
    }
    if (selection == 2){
      u8g2.drawStr( 10, 20, "Transmitter Selected"); 
      delay(1000);
      start = true;
      reciever = false;
    }
    if (selection == 4){
      u8g2.drawStr( 10, 20, "Solo Selected"); 
      delay(1000);
      start = true;
      reciever = false;
      soloMode = true;
    }
    if (selection == 1){
      if (OCTAVE < 6)
      {OCTAVE+=1;}
      else {OCTAVE = 2;}
      selection = -1;
      originalOCTAVE = OCTAVE;
      delay(250);
    }

    
  }
  else {
    selection = readJoyStick();
    u8g2.drawStr( 10, 30, "Solo"); 
    u8g2.drawStr( 85, 10, "OCT"); 
  u8g2.drawStr( 10, 10, "Receiver"); 
  u8g2.drawStr( 85, 20, "GAME"); 
  u8g2.drawStr( 10, 20, "Transmitter"); 
  u8g2.drawStr( 85, 30, "CREDS"); 

  u8g2.setCursor(120, 10);
  u8g2.print(OCTAVE);
  }
  
  u8g2.sendBuffer(); 





}


SemaphoreHandle_t CAN_TX_Semaphore;


void CAN_TX_Task (void * pvParameters) {
	uint8_t msgOut[8];
	while (1) {
		xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
		xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
		CAN_TX(0x123, msgOut);
	}
}

void CAN_TX_ISR (void) {
	xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}




void setup() {
  // put your setup code here, to run once:

  //alterSetup(); //changes the step functions

   Serial.begin(9600);

   TIM_TypeDef *Instance = TIM1;
   HardwareTimer *sampleTimer = new HardwareTimer(Instance);

  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

 

  //CAN part 3

  msgInQ = xQueueCreate(36,8);
  msgOutQ = xQueueCreate(36,8);
  CAN_TX_Semaphore = xSemaphoreCreateCounting(3,3);

  

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


   while(millis() < 5000){ //does onstart
    int offset = (millis() < 2500) ?  int(millis() / 50) : 50;
    onStart(offset);
  }

  
  while(!start){
      setupMenu();
  }

  CAN_Init(soloMode); //for looping set to true

  CAN_RegisterRX_ISR(CAN_RX_ISR);

  CAN_RegisterTX_ISR(CAN_TX_ISR);

  setCANFilter(0x123,0x7ff);
  CAN_Start();


  TaskHandle_t scanKeysHandle = NULL;
  xTaskCreate(
scanKeys,		/* Function that implements the task */
"scanKeys",		/* Text name for the task */
64,      		/* Stack size in words, not bytes */
NULL,			/* Parameter passed into the task */
2,			/* Task priority */
&scanKeysHandle );  /* Pointer to store the task handle */

TaskHandle_t displayUpdateHandle= NULL;
  xTaskCreate(
displayUpdateTask,		/* Function that implements the task */
"displayUpdate",		/* Text name for the task */
64,      		/* Stack size in words, not bytes */
NULL,			/* Parameter passed into the task */
1,			/* Task priority */
&displayUpdateHandle );  /* Pointer to store the task handle */


TaskHandle_t decodeHandle= NULL; //PROBLEM HERE -> FIGURE OUT
  xTaskCreate(
decodeTask,		/* Function that implements the task */
"decode",		/* Text name for the task */
64,      		/* Stack size in words, not bytes */
NULL,			/* Parameter passed into the task */
3,			/* Task priority */
&decodeHandle ); 


TaskHandle_t taskTXHandle= NULL; //PROBLEM HERE -> FIGURE OUT
  xTaskCreate(
CAN_TX_Task,		/* Function that implements the task */
"send",		/* Text name for the task */
64,      		/* Stack size in words, not bytes */
NULL,			/* Parameter passed into the task */
4,			/* Task priority */
&taskTXHandle ); 


  //Initialise UART
 
  //Serial.println("Hello World");

  //MUTEX
  keyArrayMutex = xSemaphoreCreateMutex();

  vTaskStartScheduler();
}

void loop() {
  }



