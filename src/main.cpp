#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>

//Constants
  const uint32_t interval = 100; //Display update interval

  volatile int32_t currentStepSize;
  volatile int32_t keyArray[7];  // has 7 rows
  const int32_t base = (2^32);
  const int32_t stepSizes[12] = {base * 277, base * 293, base * 311, base *330, base *349,
   base* 369, base*391, base*415, base * 440, base*466, base*493, base*523};


//LAB2
SemaphoreHandle_t keyArrayMutex;
//global handle for a FreeRTOS mutex that can be used by different threads to access the mutex object:
  
//CAN
volatile uint8_t TX_Message[8] = {0};

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

  if (n == 3){
    return "011";
  }

  if (n == 4){
    return "100";
  }
    
}

std::string noteSelect(const int32_t stepSizes[12]){
  for (int i =0; i < 3; i++){

    if (keyArray[i] == 112){
      if (i == 0) {currentStepSize = stepSizes[0]; return "C";}
      if (i == 1) {currentStepSize = stepSizes[1]; return "E";}
      if (i == 2) {currentStepSize = stepSizes[2]; return "G#";}
    }
    if (keyArray[i] == 208){
      if (i == 0) {currentStepSize = stepSizes[3]; return "D";}
      if (i == 1) {currentStepSize = stepSizes[4]; return "F#";}
      if (i == 2) {currentStepSize = stepSizes[5]; return "A#";}
    }
    if (keyArray[i] == 176){
      if (i == 0) {currentStepSize = stepSizes[6]; return "C#";}
      if (i == 1) {currentStepSize = stepSizes[7]; return "F";}
      if (i == 2) {currentStepSize = stepSizes[8]; return "A";}
    }
    if (keyArray[i] == 224){
      if (i == 0) {currentStepSize = stepSizes[9]; return "D#";}
      if (i == 1) {currentStepSize = stepSizes[10]; return "G";}
      if (i == 2) {currentStepSize = stepSizes[11]; return "B";}
    }
    
  }
  currentStepSize = 0;
  return "No Note";
}

void setRow(uint8_t rowIdx){

  //make more efficient and optimal
  // parse the incoming value/ turn into binary, 
  std::string binaryIdx = toBinary(rowIdx);

  int ra0, ra1, ra2;; 

  ra0 = (binaryIdx[2] == '1') ? HIGH : LOW; //set values of ra pins 
  ra1 = (binaryIdx[1] == '1') ? HIGH : LOW;
  ra2 = (binaryIdx[0] == '1') ? HIGH : LOW;

  // (binaryIdx[0] == '1') ? Serial.print("1"): Serial.print("0");
  // (binaryIdx[1] == '1') ? Serial.print("1"): Serial.print("0");
  // (binaryIdx[2] == '1') ? Serial.print("1"): Serial.print("0");
  
  digitalWrite(REN_PIN, LOW); //write low at start
  digitalWrite(RA0_PIN, ra0); //write ra pin values determined from ternary block.
  digitalWrite(RA1_PIN, ra1);
  digitalWrite(RA2_PIN, ra2);
  digitalWrite(REN_PIN, HIGH); //write high at end.
  

}

void displayUpdateTask(void * param){

  const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

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

    xSemaphoreGive(keyArrayMutex);

    
    //u8g2.print((char) TX_Message[0]);
    u8g2.drawStr(30,30,"Oct"); 
    u8g2.setCursor(50,30);
    u8g2.print(TX_Message[1]);

    u8g2.drawStr(60,30,"LastKey:"); 
    u8g2.setCursor(110,30);
    u8g2.print(TX_Message[2]);
  
  //prints the current note. Strings not compatible (WHYYYY????) So have to do this tedious
  //char conversion. Would use pointers but causes headaches. 
  
  std::string note = noteSelect(stepSizes);
  for (int i = 0; i < note.size(); i++){
    char a = note[i];
    u8g2.setCursor(62 + i*5, 20);
    u8g2.print(a); 
  }


    //Serial.println((currentStepSize >> 24) + 128);
    u8g2.sendBuffer();  
 

    //Toggle LED
    digitalToggle(LED_BUILTIN);
  } 
  
}

volatile uint32_t output = 0;

void scanKeys(void * pvParameters){
  const TickType_t xFrequency = 10/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  volatile int c0 = HIGH;
  volatile int c1 = HIGH;
  volatile int c2 = HIGH;
  volatile int c3 = HIGH;

  int8_t knob3Prev = 0;

  int8_t conv;

  int8_t keyNum=0;
  
  int change= 0;

  while(1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    for (int i = 0; i < 5; i++)
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
        change = 1;
      }
    if (c1 == HIGH){
      output+=pow(2, 6);
   
      conv+=1;
      }
      else{
        change = 2;
      }
    if (c2 == HIGH){
      output+=pow(2, 5);
      
    }
    else{
        change = 3;
      }
    if (c3 == HIGH){
      output+=pow(2, 4);
      
    }
    else{
        change = 4;
      }

  
  

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    if (i < 3)
    {
    if (output < keyArray[i]){ //check the previous vs the current state (output)
    //need to check which key was pressed.
    //to this effect we can find this based on the current row and the 
    //calculated output value.
    //i.e if output == F0, no keys
    //output == 70 && i == 0 key pressed was the first (C)
    
      keyNum = i*4;
      TX_Message[0] = 'P';
      TX_Message[1] = 4; //octave
      TX_Message[2] = keyNum + change;  //key num
    }
    keyArray[i] = output;
    }
  
     else if (i == 3){
      // if (knob3Prev == conv - 1 || conv)
      // {
      //   //xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
      //   keyArray[i] += 1;
      //   //xSemaphoreGive(keyArrayMutex);
      // }
      // else if (knob3Prev == conv + 1 ){
      //   //xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
      //   keyArray[i] -= 1;
      //   //xSemaphoreGive(keyArrayMutex);
      // }

      //from 0: 



    

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


      //knob3Prev = conv;
      }
    xSemaphoreGive(keyArrayMutex);
    }
  }

}



uint8_t readCols(uint8_t row){
  
  setRow(row);
  
  delay(0.5);

  int c0 = digitalRead(C0_PIN);
  int c1 = digitalRead(C1_PIN);
  int c2 = digitalRead(C2_PIN);
  int c3 = digitalRead(C3_PIN);

  //LOW WHEN PRESSED
  
  std::string out = "";
  uint8_t output;

  output += (c0 == HIGH)?  pow(2, 7): 0; 
  output += (c1 == HIGH)?  pow(2, 6): 0; 
  output += (c2 == HIGH)?  pow(2, 5): 0; 
  output += (c3 == HIGH)?  pow(2, 4): 0; 

  return output;

}

void sampleISR() {
  static int32_t phaseAcc = 0;
  phaseAcc += currentStepSize;

  int32_t Vout = (phaseAcc >> 24) - 128; //change for volume to increase! (12 is the highest I reccomend, quite loud ) (12 is the highest volume, 24 is the lowest)
  // volume is louder the closer to 12.

  Vout = Vout >> (8 - keyArray[3]);

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

  //Initialise UART
  Serial.begin(9600);
  Serial.println("Hello World");

  //MUTEX
  keyArrayMutex = xSemaphoreCreateMutex();

  vTaskStartScheduler();
}



float rootRet(int power) //return a power of the root of 12
{
  return pow(2, power/12);
}

void loop() {
  
  }



