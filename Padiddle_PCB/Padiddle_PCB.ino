//**************************************************************//
//  Name    : Padiddle                              
//  Author  : Kevin J DOlan
//  Date    : 1/16/2014  
//  Modified: 1/16/2014                              
//  Version : 1.0                                                         
//****************************************************************

//Constants
const boolean LEFT_SIDE = 0;
const boolean RIGHT_SIDE = 1;

const boolean SCORE_BUTTON = 0;
const boolean PENALTY_BUTTON = 1; 


const int THREASHOLD = 45;

//Pinout
const int SERIAL_PIN = 13; //previously AB_dataPin
const int CLOCK_PIN = 12; //previously clockPin
const int LATCH_PIN = 11; //previously AB_latchPin
const int SCORE_PIN = 10;
const int PENALTY_PIN = 9;
const int LEFT_SCORE_LED = 8;
const int LEFT_PENALTY_1_LED = 7;
const int LEFT_PENALTY_2_LED = 6;
const int RIGHT_SCORE_LED = 5;
const int RIGHT_PENALTY_1_LED = 4;
const int RIGHT_PENALTY_2_LED = 3;
const int BUTTON_LED = 2;

//Pin connected to ST_CP of 74HC595
int latchPin = 11;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 13;

const int LEFT_PUNCH_SENSOR = A0; // the piezo is connected to analog pin 0
const int RIGHT_PUNCH_SENSOR = A5; // the piezo is connected to analog pin 1


//Game mode determines the behavior of incScore and incPenalty
//gameMode 0: Friendly game. Inc penalty and score
//gameMode 1: Competitive game. Inc score decreases opponent score if they have no-zero score
  //If both players have zero then score is incremented to 1
  //Penalties do not cancel each other out.
//gameMode 2: Diagnostic. Randomly light all penalty LEDs and time difference from the first
	//sensor activating to the second sensor activating. Print result to screen in milliseconds.
//gameMode 3: Penalty. Same as friendly game except penalties decrease score by 1. Penalty LEDs lit
	//Designates negative score. Max negative score is -2.
const int GM_FRIENDLY = 0; //GM is abbreviation for Game Mode
const int GM_COMPETATIVE = 1;
const int GM_DIAG = 2;
const int GM_PENALTY = 3;

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
    B10011110,  //E
   //ABCDEFGd
};

const byte DISPLAY_LOWERCASE_C = B11100101;//B00011010; //xxxDExGx


//DATA
byte A_data   = B11111111;
byte B_data   = B11111111;
/*
LED PIN MATRIX for 
7(MSB)	: Left Score LED
6		: Left Penalty 1 LED
5		: Left Penalty 2 LED
4		: Right Score LED
3		: Right Penalty 1 LED
2		: Right Penalty 2 LED
1		: Button LED
0(LSB)	: Not Used (Does not correspond to pin -- disconnected)
*/
int LED_pins[8] = {1,1,1,1,1,1,1,1}; //See matrix above


//Penalty Counters
int penaltyRight = 0;
int penaltyLeft = 0;
//Score counters
int scoreRight = 0;
int scoreLeft = 0;


//CLASS DEC
class Game {
  public:
    virtual void incScore(boolean sideOfCar);
    inline virtual void incPenalty(boolean sideOfCar)
	{
		if(sideOfCar == RIGHT_SIDE)
		{
			penaltyRight++;
		}
		else
		{
			penaltyLeft++;
		}
	};
    inline virtual void decScore(boolean sideOfCar)
    {
      if(sideOfCar == RIGHT_SIDE)
      {
            scoreRight--;
            if(scoreRight < 0)
            {
               scoreRight = 19; 
            }
       }
       else //LEFT_SIDE 
       {
            scoreLeft--;
            if(scoreLeft < 0)
            {
               scoreLeft = 19; 
            }
       } 
    };
    inline virtual int getScoreLeft()
    {
      return scoreLeft;
    };
    inline virtual int getScoreRight()
    {
      return scoreRight;
    };
    inline virtual int getPenaltyLeft()
    {
      return penaltyLeft;
    };
    inline virtual int getPenaltyRight()
    {
      return penaltyRight;
    };
  protected:
    int scoreLeft;
    int scoreRight;
    int penaltyLeft;
    int penaltyRight;
};

//Friendly Game
class FriendlyGame : public Game {
  public:
    inline FriendlyGame(){
        scoreLeft = 0;
        scoreRight = 0;
        penaltyLeft = 0;
        penaltyRight = 0;
    }
    virtual void incScore(boolean sideDetected);
};

void FriendlyGame::incScore(boolean sideDetected)
{
  if(sideDetected == RIGHT_SIDE)
  {
      scoreRight++;
  }
  else
  {
      scoreLeft++;
  }
}

//End Friendly Game
//CompetitiveGame
class CompetitiveGame : public Game {
  public:
    inline CompetitiveGame(){
        scoreLeft = 0;
        scoreRight = 0;
        penaltyLeft = 0;
        penaltyRight = 0;
    }
    virtual void incScore(boolean sideDetected);
};

void CompetitiveGame::incScore(boolean sideDetected)
{
  if(sideDetected == RIGHT_SIDE)
  {
	 if(scoreRight >= 0 && scoreLeft == 0)
	  {
		  //inc score right
		  scoreRight++;
		  Serial.println("Right ahead or both zero. INC RIGHT");
	  }
	  else
	  {
		 //Score left must be higher. Dec left score
		 Serial.println( "Left ahead. DEC LEFT");
		 scoreLeft--; 
	  }
  }
  else
  {
	  if(scoreRight == 0 && scoreLeft >= 0)
	  {
		  //inc score left
		  scoreLeft++;
		  Serial.println("Left ahead or both zero. INC LEFT");
	  }
	  else
	  {
		 //Score right must be higher. Dec right score
		 scoreRight--; 
		 Serial.println( "Right ahead. DEC RIGHT");
	  }
  }
}
//End CompetitiveGame

//PenaltyGame
class PenaltyGame : public Game {
  public:
    inline PenaltyGame(){
        scoreLeft = 0;
        scoreRight = 0;
        penaltyLeft = 0;
        penaltyRight = 0;
    }
    virtual void incScore(boolean sideDetected);
    virtual void incPenalty(boolean sideDetected);
};

void PenaltyGame::incScore(boolean sideDetected)
{
	if(sideDetected == RIGHT_SIDE)
	{
		if(penaltyRight == 0)
		{
			scoreRight++;
		}
		else
		{
			penaltyRight--;
		}
	}
	else
	{
		if(penaltyLeft == 0)
		{
			scoreLeft++;
		}
		else
		{
			penaltyLeft--;
		}
	}
}

void PenaltyGame::incPenalty(boolean sideDetected)
{
	if(sideDetected == RIGHT_SIDE)
	{
		if(scoreRight == 0)
		{
			penaltyRight++;
                        if(penaltyRight > 2)
                        {
                          penaltyRight = 2;
                        }
		}
		else
		{
			scoreRight--;
		}
	}
	else
	{
		if(scoreLeft == 0)
		{
			penaltyLeft++;
                        if(penaltyLeft > 2)
                        {
                          penaltyLeft = 2;
                        }
		}
		else
		{
			scoreLeft--;
		}
	}
}

//Game Setup
Game* game;

FriendlyGame friendly;
CompetitiveGame competitive;
PenaltyGame penalty;



void setup() {
  Serial.begin(9600);      // open the serial port at 9600 bps:  
  Serial.println("Begin setup");
  
  //set pins to output so you can control the shift register
  //PIN SETUP
  pinMode(SERIAL_PIN, OUTPUT); //13
  pinMode(CLOCK_PIN, OUTPUT); //12
  pinMode(LATCH_PIN, OUTPUT); //11
  pinMode(SCORE_PIN, INPUT); //10
  pinMode(PENALTY_PIN, INPUT); //9
  pinMode(LEFT_SCORE_LED, OUTPUT); //8
  pinMode(LEFT_PENALTY_1_LED, OUTPUT); //7
  pinMode(LEFT_PENALTY_2_LED, OUTPUT); //6
  pinMode(RIGHT_SCORE_LED, OUTPUT); //5 
  pinMode(RIGHT_PENALTY_1_LED, OUTPUT); //4
  pinMode(RIGHT_PENALTY_2_LED, OUTPUT); //3
  pinMode(BUTTON_LED, OUTPUT); //2
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  
  //Test display (All LEDs on for 250ms)
  Serial.println("Test display");
  //Test 7-Seg Displays
  //Test LEDs
  setLEDs();
  
  //All Status LEDs are still lit. Test 7-Seg displays with number cycle
  Serial.println("Number Cycle");
  cycleNumbers(100);
  //Clear status LEDs. Light buttons
  setLEDs(0,0,0,0,0,0,1);
  Serial.println("End Setup");
  
  //GAME MODE SELECT
  Serial.println("Begin game select mode");
  B_data = DISPLAY_LOWERCASE_C;
  shiftOutScore();
  //Countdown 3...2...1... on Right display
  
  countdown(3); //Countdown from 3
  
  int configPin_0_state = digitalRead(PENALTY_PIN); //The bottom button (penalty) in config mode is pin0
  int configPin_1_state = digitalRead(SCORE_PIN); //The top button (score) in config mode is pin1
  
  //Determine GameMode
  if(configPin_1_state == LOW  && configPin_0_state == LOW) //00 (0)
  {
    gameMode = GM_PENALTY;
    game = &penalty;
  }
  else if(configPin_1_state == LOW  && configPin_0_state == HIGH ) //01 (1)
  {
    gameMode = GM_COMPETATIVE;
    game = &competitive;
  }
  else if(configPin_1_state == HIGH  && configPin_0_state == LOW ) //10 (2)
  {
    gameMode = GM_DIAG;
  }
  else //11 (3)
  {
     gameMode = GM_FRIENDLY;
     game = &friendly;     
  }
  Serial.print("Game Mode selected. Current game mode: ");   Serial.println(gameMode, DEC);
  
  //Display selected gamemode on B display for 500 ms.
  setScoreRight(gameMode);
  setScoreLeft(0); //Clear left dispaly
  setLEDs(0,0,0,0,0,0,0); //Unset button LED
  delay(2000);
  setScoreRight(0);
  setScoreLeft(0);
  Serial.println("End setup");
  //Begin game!

   
}

void loop() {
    Serial.println("Enter game loop");
    boolean sideDetected;
    if(gameMode == GM_DIAG)
    {
        Serial.print("Start Diag Mode");
        //Diagnostic loop
        while(true)
        {
           unsigned long startTime = millis(); //Record time when light is lit
           unsigned long firstHit;
           unsigned long secondHit;
           boolean firstSide;
           countdown(3); //Countdown from 3
           setScoreRight(0);
           shiftOutScore();
           int timeDelay = random(1000, 10000); //delay between 1 and 10 sec
           delay(timeDelay);
           
           setLEDs(0,1,1,0,1,1,0); //Turn on all penalty lights
           
           //Wait for first punch
            
           while(true)
           {
              int leftSensorValue = analogRead(LEFT_PUNCH_SENSOR);
              int rightSensorValue = analogRead(RIGHT_PUNCH_SENSOR);
              if(leftSensorValue>=THREASHOLD)
              {
                //LEFT DETECTED
                setScoreLED(LEFT_SIDE); //Set score left
                firstHit = millis();
                firstSide = LEFT_SIDE;
                break;
              }
              else if(rightSensorValue>=THREASHOLD)
              {
                //RIGHT DETECTED
                setScoreLED(RIGHT_SIDE); //Set score left
                firstHit = millis();
                firstSide = RIGHT_SIDE;
                break;
              }
           }
           Serial.print("Reaction Time: ");
           Serial.print(firstHit - startTime - timeDelay);
           Serial.println("ms"); 
         //Wait for second punch
           while(true)
           {
              int leftSensorValue = analogRead(LEFT_PUNCH_SENSOR);
              int rightSensorValue = analogRead(RIGHT_PUNCH_SENSOR);
              if(leftSensorValue>=THREASHOLD && firstSide == RIGHT_SIDE)
              {
                //LEFT DETECTED
                secondHit = millis();
                break;
              }
              else if(rightSensorValue>=THREASHOLD && firstSide == LEFT_SIDE)
              {
                //Right DETECTED
                secondHit = millis();
                break;
              }
           }
           Serial.print("Delay Between Hits: ");
           Serial.print(secondHit - firstHit);
           Serial.println("ms");
           Serial.println("/-----------------------\\");
           setLEDs(0,0,0,0,0,0,0); //Clear all lights
        }
    }
    while(true) //Wait until a sensor is triggered.
    {
      pushData();
      int leftSensorValue = analogRead(LEFT_PUNCH_SENSOR);
      int rightSensorValue = analogRead(RIGHT_PUNCH_SENSOR);
      if(leftSensorValue>=THREASHOLD)
      {
        //LEFT DETECTED
        sideDetected = LEFT_SIDE;
        setScoreLED(LEFT_SIDE); //Set score left
        Serial.print("LEFT - ");
        Serial.print(leftSensorValue);
        Serial.println("");
        break;
      }
      else if(rightSensorValue>=THREASHOLD)
      {
        //RIGHT DETECTED
        sideDetected = RIGHT_SIDE;
        setScoreLED(RIGHT_SIDE); //Set score right
        Serial.print("RIGHT - ");
        Serial.print(rightSensorValue);
        Serial.println("");
        break;    
      }
      else
      {
         //Not detected. Check for score decrement
         int scoreState_LeftDec= digitalRead(SCORE_PIN);
         int penaltyState_RightDec = digitalRead(PENALTY_PIN);
         if(scoreState_LeftDec == HIGH)
         {
              //Decrement socre left
              game->decScore(LEFT_SIDE);
              waitForbuttonRelease(SCORE_BUTTON);
         }
         else if (penaltyState_RightDec == HIGH)
         {
              //Decrement socre right
              game->decScore(RIGHT_SIDE);
              waitForbuttonRelease(PENALTY_BUTTON);

         }
         continue; //Restart loop
      }
    }  //End While loop to check for sensor detection. Wait for button input
    boolean score = isScore(); //Wait for score or penalty button to be pushed
    unsetScoreLEDs();
    if(score)
    {
        //Increment counter
        game->incScore(sideDetected);
        pushData();
        waitForbuttonRelease(SCORE_BUTTON);
    }
    else
    {
       //Penalty
       game->incPenalty(sideDetected);
       pushData();
       waitForbuttonRelease(PENALTY_BUTTON);
    }
}//End loop


boolean isScore(){
 int score = 0; //0 is unset
 while(score==0)
  {
    int scoreState = digitalRead(SCORE_PIN);
    int penaltyState = digitalRead(PENALTY_PIN);
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
       //Wait until button is released
       while(digitalRead(PENALTY_PIN) == HIGH)
       {
          //wait for button to go low
       }
       Serial.println("Penalty button released");
       return;
   }
   else
   {
       //Wait until button is released
       while(digitalRead(SCORE_PIN) == HIGH)
       {
          //wait for button to go low
       }
       Serial.println("Score button released");
       return; 
   }
 }


void setScoreLeft(int num)
{
  //B_data
  B_data = B11111111 ^ numeral[num]; //Bitwise XOR to invert each value for active low enable
  shiftOutScore();
}

void setScoreRight(int num)
{
  //A_data
  A_data = B11111111 ^ numeral[num]; //Bitwise XOR to invert each value for active low enable
  shiftOutScore();
}

/*
LED PIN MATRIX for 
7(MSB)	: Left Score LED
6		: Left Penalty 1 LED
5		: Left Penalty 2 LED
4		: Right Score LED
3		: Right Penalty 1 LED
2		: Right Penalty 2 LED
1		: Button LED
0(LSB)	: Not Used (Does not correspond to pin -- disconnected)
*/
void setLEDs(int leftScore, int leftPenalty1, int leftPenalty2, int rightScore, int rightPenalty1, 
	int rightPenalty2, int buttonLED) //previously shiftOutC()
{
	LED_pins[7] = leftScore;
	LED_pins[6] = leftPenalty1;
	LED_pins[5] = leftPenalty2;
	LED_pins[4] = rightScore;
	LED_pins[3] = rightPenalty1;
 	LED_pins[2] = rightPenalty2;
	LED_pins[1] = buttonLED;

	setLEDs();
}

void setLEDs() //previously shiftOutC()
{
	digitalWrite(LEFT_SCORE_LED, LED_pins[7]);
	digitalWrite(LEFT_PENALTY_1_LED, LED_pins[6]);
	digitalWrite(LEFT_PENALTY_2_LED, LED_pins[5]);
	digitalWrite(RIGHT_SCORE_LED, LED_pins[4]);
	digitalWrite(RIGHT_PENALTY_1_LED, LED_pins[3]);
	digitalWrite(RIGHT_PENALTY_2_LED, LED_pins[2]);
	digitalWrite(BUTTON_LED, LED_pins[1]);
}

void setScoreLED(boolean side)
{
	if(side == LEFT_SIDE)
	{
		LED_pins[7] = 1;
	}
	else //RIGHT_SIDE
	{
		LED_pins[4] = 1;
	}
        LED_pins[1] = 1;
	setLEDs();
}

void unsetScoreLEDs(){
	LED_pins[7] = 0;
	LED_pins[4] = 0;
	setLEDs();
}


void shiftOutScore() //previously shiftOutAB()
{
  digitalWrite(LATCH_PIN, LOW);
  // shift out the bits for highbyte (A)
  shiftOut(SERIAL_PIN, CLOCK_PIN, MSBFIRST, A_data); 
  // shift out lowbyte (B)
  shiftOut(SERIAL_PIN, CLOCK_PIN, MSBFIRST, B_data);
  digitalWrite(LATCH_PIN, HIGH);
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
    setScoreRight(0);
    setScoreLeft(0);
}

void countdown(int startingNumber)
{
	int currentNum = startingNumber;
	while(currentNum > 0)
	{
		setScoreRight(currentNum);
		currentNum--;
		delay(1000); //Wait for 1 sec 
	}
}

void pushData()
{
  //Set score
   setScoreLeft(game->getScoreLeft());
   setScoreRight(game->getScoreRight());
  //Determine penalty LEDs to light
  int leftP1 = 0;
  int leftP2 = 0;
  int rightP1 = 0;
  int rightP2 = 0;
  //Left
  int penaltyLeft = game->getPenaltyLeft();
  if(penaltyLeft == 1)
  {
    leftP1 = 1;
  }
  else if(penaltyLeft == 2)
  {
    leftP1 = 1;
    leftP2 = 1;
  }
  //Right 
  int penaltyRight = game->getPenaltyRight();
  if(penaltyRight == 1)
  {
    rightP1 = 1;
  }
  else if(penaltyRight == 2)
  {
    rightP1 = 1;
    rightP2 = 1;
  }
  //Set LEDs
   setLEDs(0,leftP1,leftP2,0,rightP1,rightP2,0);
}


//End PenaltyGame

/*FriendlyGame friendly;
void setup() {
  // put your setup code here, to run once:
  Game* game = &friendly;
  game->incScore(LEFT_SIDE);
}

void loop() {
  // put your main code here, to run repeatedly: 
  
}*/




