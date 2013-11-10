//**************************************************************//
//  Name    : shiftOutCode, Hello World                                
//  Author  : Carlyn Maw,Tom Igoe, David A. Mellis 
//  Date    : 25 Oct, 2006    
//  Modified: 23 Mar 2010                                 
//  Version : 2.0                                             
//  Notes   : Code for using a 74HC595 Shift Register           //
//          : to count from 0 to 255                           
//****************************************************************

//Constants
const boolean LEFT_SIDE = 0;
const boolean RIGHT_SIDE = 1;

const boolean SCORE_BUTTON = 0;
const boolean PENALTY_BUTTON = 1; 

const int THREASHOLD = 30;

//Game mode determines the behavior of incScore and incPenalty
//gameMode 0: Friendly game. Inc penalty and score
//gameMode 1: Competative game. Inc score decreses opponent score if they have no-zero score
  //If both players have zero then score is incremnted to 1
  //Penalties do not cancel eachother out.
int gameMode = 0;  

const byte numeral[21] = {
    B00000000,  // NULL (Off) --No purpose for zero number. "Off" is zero.
    B01100000,  // 1
    B11011010,  // 2
    B11110010,  // 3
    B01100110,  // 4
    B10110110,  // 5
    B10111110,  // 6
    B11100000,  // 7
    B11111110,  // 8
    B11100110,  // 9
    B11111101,  // 10 (Decimal Set)
    B01100001,  // 11
    B11011011,  // 12
    B11110011,  // 13
    B01100111,  // 14
    B10110111,  // 15
    B10111111,  // 16
    B11100001,  // 17
    B11111111,  // 18
    B11100111,  // 19
    B00000000,  //NULL (Off)
};

int scorePin = 2; //Pin for score button
int penaltyPin = 3; //Pin for penalty button.

int configPin_0 = 4;
int configPin_1 = 5;

//Pin connected to SH_CP of 74HC595
int clockPin = 12;

//Pin connected to ST_CP of 74HC595
int C_latchPin = 7;
////Pin connected to DS of 74HC595
int C_dataPin = 6;
byte C_data = B00000000;

int AB_latchPin = 10;
////Pin connected to DS of 74HC595
int AB_dataPin = 11;
byte A_data = B11111111;
byte B_data = B11111111;

int leftPunchSensor = A0; // the piezo is connected to analog pin 0
int rightPunchSensor = A1; // the piezo is connected to analog pin 0
//Penatly Counters
int penaltyRight = 0;
int penaltyLeft = 0;
//Score counters
int scoreRight = 0;
int scoreLeft = 0;



void setup() {
  Serial.begin(9600);      // open the serial port at 9600 bps:  
  //set pins to output so you can control the shift register
  
  pinMode(clockPin, OUTPUT);
  pinMode(C_latchPin, OUTPUT);
  pinMode(C_dataPin, OUTPUT);

  pinMode(AB_dataPin, OUTPUT);
  pinMode(AB_latchPin, OUTPUT);
  
  pinMode(scorePin, INPUT);
  pinMode(penaltyPin, INPUT);
  
  pinMode(configPin_0, INPUT);
  pinMode(configPin_1, INPUT);
  
  int configPin_0_state = digitalRead(configPin_0);
  int configPin_1_state = digitalRead(configPin_1);
  
  if(configPin_0_state == LOW && configPin_1_state == LOW) //0
  {
    gameMode = 0;
  }
  else if(configPin_0_state == LOW && configPin_1_state == LOW) //1
  {
    gameMode = 1;
  }
  else
  {
     //2,3 --> default to gameMode 0.
     gameMode = 0;   
  }
  
  C_data = B11111100;
  shitOutC();
  delay(500);
  C_data = B00000000;
  cycleNumbers(100);
  shitOutC();

   
}

void loop() {
  // count from 0 to 255 and display the number 
  // on the LEDs
  // take the latchPin low so 
  // the LEDs don't change while you're sending in bits:
    //digitalWrite(C_latchPin, LOW);
    // shift out the bits:
    //shiftOut(C_dataPin, clockPin, MSBFIRST, C_data);  
    //take the latch pin high so the LEDs will light up:
    //digitalWrite(C_latchPin, HIGH);
    // pause before next value:
    boolean sideDetected;
    while(true) //Wait until a sensor is triggered.
    {
      int leftSensorValue = analogRead(leftPunchSensor);
      int rightSensorValue = analogRead(rightPunchSensor);
      if(leftSensorValue>=THREASHOLD)
      {
        //LEFT DETECTED
        sideDetected = LEFT_SIDE;
        C_data = C_data | B00000100; //Set score left
        Serial.print("LEFT - ");
        Serial.print(leftSensorValue);
        Serial.println("");
        break;
      }
      else if(rightSensorValue>=THREASHOLD)
      {
        //RIGHT DETECTED
        sideDetected = RIGHT_SIDE;
        Serial.print("RIGHT - ");
        Serial.print(rightSensorValue);
        Serial.println("");
        C_data = C_data | B00001000; //Set score right
        break;    
      }
      else
      {
         //Not detected. Check for score decrement
         int scoreState_LeftDec= digitalRead(scorePin);
         int penaltyState_RightDec = digitalRead(penaltyPin);
         if(scoreState_LeftDec == HIGH)
         {
              //Decrement socre left
              decScore(LEFT_SIDE);
              waitForbuttonRelease(SCORE_BUTTON);
         }
         else if (penaltyState_RightDec == HIGH)
         {
              //Decrement socre right
              decScore(RIGHT_SIDE);
              waitForbuttonRelease(PENALTY_BUTTON);
         }
         continue; //Restart loop
      }
    }  //End While loop to check for snesor detection
    
    shitOutC(); //Light score LED
    boolean score = isScore(); //Wait for score or penalty button to be pushed
    unsetScoreLEDs();
    if(score)
    {
        //Increment counter
        incScore(sideDetected);
        waitForbuttonRelease(SCORE_BUTTON);
    }
    else
    {
       //Penalty
       incPenalty(sideDetected);
       waitForbuttonRelease(PENALTY_BUTTON);
    }
}//End loop


boolean isScore(){
 int score = 0; //O is unset
 while(score==0)
  {
    int scoreState = digitalRead(scorePin);
    int penaltyState = digitalRead(penaltyPin);
    if(scoreState == HIGH)
    {
         return true;
    }
    else if (penaltyState == HIGH)
    {
         return false;
    }
  }
}

void waitForbuttonRelease(boolean button)
{
   if(button == PENALTY_BUTTON)
   {
       //Wait until button is releaed
       while(digitalRead(penaltyPin) == HIGH)
       {
          //wait for button to go low
       }
       Serial.println("Penalty button relased");
       return;
   }
   else
   {
       //Wait until button is released
       while(digitalRead(scorePin) == HIGH)
       {
          //wait for button to go low
       }
       Serial.println("Score button relased");
       return; 
   }
 }
  

void unsetScoreLEDs(){
    C_data = C_data & B11110011; //Set score right
    shitOutC();
}

void decScore(boolean sideOfCar)
{
  if(sideOfCar == RIGHT_SIDE)
  {
      scoreRight--;
      Serial.print("SCORE RIGHT - ");
      Serial.print(scoreRight);
      Serial.println("");
      setScoreRight(scoreRight);
  }
  else //LEFT_SIDE 
  {
      scoreLeft--;
      Serial.print("SCORE LEFT - ");
      Serial.print(scoreLeft);
      Serial.println("");
      setScoreLeft(scoreLeft);
  }
}

void incScore(boolean sideOfCar)
{
  if(sideOfCar == RIGHT_SIDE)
  {
    if(gameMode == 0)
    {
      scoreRight++;
      Serial.print("SCORE RIGHT - ");
      Serial.print(scoreRight);
      Serial.println("");
    }
    else if(gameMode == 1)
    {
      if(scoreRight == 0 && scoreLeft == 0)
      {
          //inc score right
          scoreRight++;
          Serial.println("Right ahead or both zero. INC RIGHT");
      }
      else
      {
         //Score left must be higher. Dec left score
         Serial.println("Left ahead. DEC LEFT");
         scoreLeft--; 
      }
    }
  }
  else
  {
    if(gameMode == 0){ 
      //Left side 
      scoreLeft++;
      Serial.print("SCORE LEFT - ");
      Serial.print(scoreLeft);
      Serial.println("");
    }
    else if(gameMode == 1)
    {
      if(scoreRight == 0 && scoreLeft == 0)
      {
          //inc score left
          scoreLeft++;
          Serial.println("Left ahead or both zero. INC LEFT");
      }
      else
      {
         //Score right must be higher. Dec right score
         scoreRight--; 
         Serial.println("Right ahead. DEC RIGHT");
      }
    }
  }
  //Set score displays
  setScoreRight(scoreRight);
  setScoreLeft(scoreLeft);
}

void incPenalty(boolean sideOfCar) {
  if(sideOfCar == RIGHT_SIDE)
  {
    penaltyRight++;
    Serial.print("PENALTY RIGHT - ");
    Serial.print(penaltyRight);
    Serial.println("");
    if(penaltyRight<=1)
    {
        //Set penalty 1
        C_data = C_data | 101000000;
         Serial.print(C_data, BIN);
        shitOutC();
        return;
    }
    else
    {
         //Set penalty 1 and 2
        C_data = C_data | 11000000;
         Serial.print(C_data, BIN);
        shitOutC();
        return;
    }
  }
  else //Left side
  {
    penaltyLeft++;
    Serial.print("PENALTY LEFT - ");
    Serial.print(penaltyLeft);
    Serial.println("");
    if(penaltyLeft<=1)
    {
        //Set penalty 1
        C_data = C_data | B00010000;
        Serial.print(C_data, BIN);;
        shitOutC();
        return;
    }
    else
    {
         //Set penalty 1 and 2
        C_data = C_data | B00110000;
        Serial.print(C_data, BIN);
        shitOutC();
        return;
    }
  }

}
void setScoreLeft(int num)
{
  //A_data
  A_data = B11111111 ^ numeral[num]; //Bitwise XOR to invert each value for active low enable
  shiftOutAB();
}

void setScoreRight(int num)
{
  //B_data
  B_data = B11111111 ^ numeral[num]; //Bitwise XOR to invert each value for active low enable
  shiftOutAB();
}

void shitOutC()
{
  digitalWrite(C_latchPin, LOW);
  // shift out the bits:
  shiftOut(C_dataPin, clockPin, MSBFIRST, C_data);  
  //take the latch pin high so the LEDs will light up:
  digitalWrite(C_latchPin, HIGH);
}

void shiftOutAB()
{
  digitalWrite(AB_latchPin, LOW);
  // shift out the bits for highbyte (A)
  shiftOut(AB_dataPin, clockPin, MSBFIRST, A_data); 
  // shift out lowbyte (B)
  shiftOut(AB_dataPin, clockPin, MSBFIRST, B_data);
  digitalWrite(AB_latchPin, HIGH);
}

void cycleNumbers(int cycleInterval)
{
  for(int i = 0; i <21; i++)
  {
    setScoreRight(i);
    setScoreLeft(i);
    if(i!=20)
    {
      delay(cycleInterval);
    }
  }
}
