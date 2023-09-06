/*MIT License

Copyright (c) 2023 Tel @ TelsBench

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, incluing without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUD_ING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//VERSION 1.0
//Includes
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <EasyButton.h>
#include <EEPROM.h>

//Defines
#define SCREEN_WIDTH    128 // OLED display width, in pixels
#define SCREEN_HEIGHT    32 // OLED display height, in pixels
#define OLED_RESET       -1 
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//DRAM Controls        GPIO PINS
#define D_IN            9  
#define D_OUT          10 
#define CAS_NOT        11  
#define RAS_NOT        12   
#define R_W_NOT        13    

#define D8              8  //9th Bit - needed for 256k x 1 DRAMS

#define RUNNING        14  //NANO PIN  A0    
#define PASS           16  //NANO PIN  A2    GREEN
#define FAIL           15  //NANO PIN  A1    RED

#define ADDR_LEN_MIN    7
#define ADDR_LEN_MAX    9

#define BITSWITCH       17  //Low on reset causes 'Setup' of Adrress Bits Selector to launch on OLED

/***********    INITIAL SETTINGS  *************/
int ADDR_LENGTH    =    8; 
int BITS_COUNT; //Num ROW OR COL DRAM Bits + 1 Used for for loops in (While condition) set after ADDR_LENGTH Is Set.
bool exitSetup = false; //Causes .'Setup' of address bits to exit, save settngs and continue to tests
EasyButton button(BITSWITCH);

//Method Prototypes
void testEqual();
void testAlternate();
void testMarchingValues();

void initialiseDisplay()
{
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

  display.clearDisplay();
  display.setTextSize(1);                 // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);    // Draw white text
  display.display();
}

void oledDisplay(int x, int y, String message, bool clearscreen)
{
  if(clearscreen) display.clearDisplay();

  display.setCursor(x,y);           
  display.println(message);
  display.display();
}

void oledFailedTest(int row , int col, int expected, int actual){

        int add = (row*256) + col;
        oledDisplay(0,10,String("Expect:") + String(expected) + String(", Actual :") + String(actual),false);
        oledDisplay(0,20, String("FAIL - Addr & ") + String(add,HEX),false);
}

void oledPassedTest(){
    oledDisplay(0,10,"** PASSED **",false);
}

void displaySplashScreen()
{
  oledDisplay(0,0, "DRAM TEST STARTING !!", true);
  delay(3000);
}

//SETUP Helpers - Used to change the BIT ADDR_LENGTH
void cycleAddrlength()
{
   int addrlength  = ADDR_LENGTH;
   if(addrlength==ADDR_LEN_MAX)  {(ADDR_LENGTH = ADDR_LEN_MIN); return;}
   ADDR_LENGTH++;
}

void onPressed()
{
  cycleAddrlength();
}

void onPressedFor()
{
  exitSetup=true;
}

void setupBitLength()
{

  button.onPressed(onPressed);
  button.onPressedFor(1500,onPressedFor);
  display.setTextSize(2); 
  oledDisplay(12,0,String(" Release"),false);
  display.setTextSize(2); 
  oledDisplay(12,17,String(" Button"),false);
  delay(2000);
 
  button.begin();  
  while(1)
  {
    display.setTextSize(3); 
    oledDisplay(0,0,String(ADDR_LENGTH),true);
    display.setTextSize(2);
    oledDisplay(12,0,String(" CLK >"),false);
    oledDisplay(12,17,String(" LNG-Prss >>"),false);
    delay(200);
    if( exitSetup )  return;
    button.read();
  }
}

//Main Setup
void setup() 
{
  //Check Memory for a stored Bit Length
  if(EEPROM.read(0)==255) 
    EEPROM.write(0,ADDR_LENGTH);
  else
    ADDR_LENGTH=EEPROM.read(0);
   //Check If user wants to change the Bit Length
  pinMode(BITSWITCH,INPUT_PULLUP);
  if( digitalRead(BITSWITCH)==LOW )
  {
    initialiseDisplay();
    setupBitLength();
    EEPROM.write(0,ADDR_LENGTH); 
  }
  button.disableInterrupt();
  BITS_COUNT=(1 << ADDR_LENGTH);

  //*** NORMAL OPERATIONS **********************
  initialiseDisplay();
  oledDisplay(0,0,String("Address Bits : ") + String(ADDR_LENGTH),false);
  delay(3000);

  //DRAM CONROLS & DATA DIRECTION
  pinMode(D_IN,OUTPUT);
  pinMode(D_OUT,INPUT);
  pinMode(CAS_NOT,OUTPUT);
  pinMode(RAS_NOT,OUTPUT);
  pinMode(R_W_NOT,OUTPUT);

  //DRAM ADDRESS - DATA DIRECTION
  DDRD = 0xff;         //D0-D7
  pinMode(D8,OUTPUT);  //D8

  //LED INDICATORS DATA DIRECTION
  pinMode(RUNNING,OUTPUT);
  pinMode(FAIL,OUTPUT);
  pinMode(PASS,OUTPUT);

  //DRAM CONTROL LINES - INITIAL STATE
  digitalWrite(CAS_NOT,HIGH);
  digitalWrite(RAS_NOT,HIGH);
  digitalWrite(R_W_NOT,HIGH);

  //LED INDICATORS -  INITIAL STATE
  digitalWrite(RUNNING,HIGH); //ON Means RUNNING 
  digitalWrite(FAIL,LOW);
  digitalWrite(PASS,LOW);

  displaySplashScreen();
}

//Main Loop
void loop() 
{
  //Quick Tests
  testEqual();
  testAlternate();
  //Longer Tests
  testMarchingValues();

  //ALL RUN TESTS PASSSED
  digitalWrite(RUNNING, LOW);
  digitalWrite(PASS, HIGH);   

  //Some Humour and a final result :)
  oledDisplay(0,0,"* That's DRAM good!",true);

  //Wait for reset button in an infinate loop.
  while (1) {};

}

//Test helper methods
void writeToRowCol(int row , int col, int val)
{

  //Present the value to be written on D_IN
  digitalWrite(D_IN, val);

  //MUST BE IN ROW COL ORDER
  PORTD = row;
  digitalWrite(D8, (row >> 8)); 
  digitalWrite(RAS_NOT, LOW);

  PORTD = col;
  digitalWrite(D8, (col >> 8)); 
  digitalWrite(CAS_NOT, LOW);

  digitalWrite(R_W_NOT, LOW);
  digitalWrite(R_W_NOT, HIGH);

  digitalWrite(CAS_NOT, HIGH);
  digitalWrite(RAS_NOT, HIGH);
}

int readRowCol(int row, int col)
{
  //MUST BE IN ROW COL ORDER
  PORTD = row;
  digitalWrite(D8, (row >> 8)); 
  digitalWrite(RAS_NOT, LOW);

  PORTD = col;
  digitalWrite(D8, (col >> 8)); 
  digitalWrite(CAS_NOT, LOW);

  int val = digitalRead(D_OUT);
 
  digitalWrite(CAS_NOT, HIGH);
  digitalWrite(RAS_NOT, HIGH);
  return val;
}

void fail()
{
  digitalWrite(RUNNING, LOW);
  digitalWrite(FAIL, HIGH);
  while (1) {};
}

void wipeMemoryWith( int val )
{
  for (int row=0; row<BITS_COUNT; row++) 
  {
    for (int col=0; col<BITS_COUNT; col++) 
    {     
      writeToRowCol(row,col,val);
    }
  }
}

//Tests
void testEqual()
{

  for( int val =0;val<2; val++){

    oledDisplay(0,0,String("Test Equal : ") + String( val ),true);

    for (int row=0; row<BITS_COUNT; row++)
      for (int col=0; col<BITS_COUNT; col++)
        writeToRowCol(row, col, val);

    int rdVal=0;
    for (int row=0; row<BITS_COUNT; row++)
      for (int col=0; col<BITS_COUNT; col++)
        if ((rdVal=readRowCol(row, col)) != val){   
            oledFailedTest(row,col,val, rdVal);
          fail();
        }

    oledPassedTest();
    delay(2000);

  }
  return;

}

void testAlternate()
{
  for( int start=0; start<2; start++)
  {
    oledDisplay(0,0,String("Test Alternate : ") + String( start ),true);

    int altVal = start;
    for(int row=0;row<BITS_COUNT;row++) 
    {
      for(int col=0;col<BITS_COUNT;col++) 
      {
        writeToRowCol(row, col, altVal);
        altVal=!altVal;     
      }
    }

    int rdVal;
    altVal=start; 
    for(int row=0; row<BITS_COUNT;row++) 
    {
      for(int col=0;col<BITS_COUNT;col++) 
      { 
        if((rdVal=readRowCol(row,col)) !=altVal )
        {
          oledFailedTest(row,col,altVal,rdVal);
          fail();
        }
        altVal = !altVal;
      }
    }

    oledPassedTest();
    delay(2000);

  }

  return;
}

void testMarchingValues()
{

  for( int marchValue=0; marchValue <2; marchValue++){
    //Display the step name
    if(marchValue==HIGH)
      oledDisplay(0, 0, "Test Marching HIGH",true);
    else
      oledDisplay(0, 0, "Test Marching LOW",true);

    int row,col;
    int fillValue = marchValue==HIGH ? LOW : HIGH;

    //Fill Memory
    wipeMemoryWith(fillValue);

    //March it through
    for (row=0; row<BITS_COUNT; row++) 
    {
      for (col=0; col<BITS_COUNT; col++) 
      {   
         writeToRowCol(row, col,marchValue);

        //Read the marchedValue    
        int readVal;        
        if ((readVal=readRowCol(row, col)) != marchValue){
          oledFailedTest(row, col, marchValue,readVal);
          fail();
        }
        else
        {
          //Replace the written value with the fillValue
          writeToRowCol(row,col,fillValue);    
        }
      }
    }
  
   oledPassedTest();
   delay(2000);
  }
   return;
}


