#include <stdarg.h>
#include <math.h>


#include <IRremote.h>
#include <IRremoteInt.h>
#include <boarddefs.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

#define  ZERO   0xFF6897
#define  ONE    0xFF30CF
#define  TWO    0xFF18E7
#define  THREE  0xFF7A85
#define  FOUR   0xFF10EF
#define  FIVE   0xFF38C7
#define  SIX    0xFF5AA5
#define  SEVEN  0xFF42BD
#define  EIGHT  0xFF4AB5
#define  NINE   0xFF52AD
#define  AUTO   0xFF9867//100+ button
const int outpin = 10;
const int trigPin = 11;
const int echoPin = 12;
const int IRsensor = 13;
bool autoMode = false;
IRrecv irrecv(IRsensor);
decode_results results;
bool specialEffectTriggered = false;



class meter
{
  private:
    int numOfTiers;
    int tierPins[11];//11 is max availible pins, minus one for a sensor.
    int currentLevel = 0;//IMPORTANT: CurrentLevel is the highest tier lights that are on.
    void allOn()
    {
      for (int iii = 0; iii < numOfTiers; iii++) //Used iii just in case.
      {
        digitalWrite(tierPins[iii], LOW);
      }
    }
    void allOff()
    {
      for (int iii = 0; iii < numOfTiers; iii++) //Used iii just in case.
      {
        digitalWrite(tierPins[iii], HIGH);
      }
    }
    void oddOn()
    {
      allOff();
      for (int iii = 1; iii < numOfTiers; iii = iii + 2) //Used iii just in case.
      {
        digitalWrite(tierPins[iii], LOW);
      }
    }
    void evenOn()
    {
      allOff();
      for (int iii = 0; iii < numOfTiers; iii = iii + 2) //Used iii just in case.
      {
        digitalWrite(tierPins[iii], LOW);
      }
    }
  public:
    meter(int tiers, ...);
    void setLevel(int level);
    void flashyBlinky() const;
    void beepBeep() const;
    int getCurrentLevel() const;
    int getNumOfTiers() const;
} weightLightMeter(8, 2, 3, 4, 5, 6, 7, 8, 9);//First variable is number of levels, after that, it's pins in order from bottom to top.




meter::meter(int tiers, ...)
{
  int tierNum = 0;//Measures tier at time of declaration
  va_list args;
  va_start(args, tiers);
  numOfTiers = tiers;
  while (tierNum <= numOfTiers)//Until the list ends
  {
    int pin = va_arg(args, int);
    tierPins[tierNum] = pin;
    pinMode(tierPins[tierNum], OUTPUT);
    digitalWrite(tierPins[tierNum], HIGH);//SCREW YOU, REVERSE LOGIC!
    tierNum++;
  }
  va_end(args);
  for (int ii = tierNum; ii < 11; ii++)
  {
    tierPins[ii] = 0;
  }
}

void meter::setLevel(int levelToSet)
{
  Serial.print("Setting level to ");
  Serial.println(levelToSet);
  if (levelToSet >= numOfTiers || levelToSet < 0)
  {
    Serial.print("Invalid number; levelToSet is ");
    Serial.print(levelToSet);
    Serial.print(" and numOfTiers is ");
    Serial.println(numOfTiers);
  }
  while (currentLevel > levelToSet && levelToSet < numOfTiers)
  {
    if (tierPins[currentLevel] == 0)
    {
      Serial.println("AN ERROR OCCURRED");//Check that there is a value associated with tierPins[ii - 1]
#if 1
      Serial.print("You are in loop 1 of 2. currentLevel is ");
      Serial.print(currentLevel);
      Serial.println(" and the numbers in tierPins are");
      for (int ii = 0; ii < numOfTiers; ii++)
      {
        Serial.println(tierPins[numOfTiers]);
      }
#endif
      break;
    }
    digitalWrite(tierPins[currentLevel - 1], HIGH);//Reverse logic is WEIRD.
    currentLevel--;
    Serial.print("Level lowered to ");
    Serial.println(currentLevel);
    delay(100);
  }
  while (currentLevel < levelToSet && levelToSet >= 0)
  {
    //Note: I don't have tierPins[currentLevel - 1] because that wouldn't work out.
    if (( tierPins[currentLevel]) == 0 )
    {
      Serial.println("AN ERROR OCCURRED");//Check that there is a value associated with tierPins[ii - 1]
#if 1
      Serial.print("You are in loop 2 of 2. currentLevel is ");
      Serial.println(tierPins[currentLevel]);
#endif
    }
    digitalWrite(tierPins[currentLevel], LOW);//Reverse logic is WEIRD.
    currentLevel++;
    Serial.print("Level raised to ");
    Serial.println(currentLevel);
    delay(100);
  }
}
void meter::flashyBlinky() const
{
  for (int jj = 0; jj < 2; jj++)
  {
    allOff();
    for (int kk = 0; kk < 2; kk++)
    {
      for (int ii = 0; ii < numOfTiers; ii++)
      {
        digitalWrite(tierPins[ii], LOW);
        delay(75);
        digitalWrite(tierPins[ii], HIGH);
      }
      for (int ii = numOfTiers - 2; ii > 0; ii--)
      {
        digitalWrite(tierPins[ii], LOW);
        delay(75);
        digitalWrite(tierPins[ii], HIGH);
      }
    }
    oddOn();
    delay(250);
    evenOn();
    delay(250);
    oddOn();
    delay(250);
    evenOn();
  }
  allOff();
  for (int ii = 0; ii < getCurrentLevel(); ii++)
  {
    digitalWrite(tierPins[ii], LOW);
  }
}
void meter::beepBeep() const
{
  allOn();
  delay(500);
  allOff();
  for (int ii = 0; ii < getCurrentLevel(); ii++)
  {
    digitalWrite(tierPins[ii], LOW);
  }
}
int meter::getCurrentLevel() const
{
  return currentLevel;
}

int meter::getNumOfTiers() const
{
  return numOfTiers;
}

void specialEffect()
{
  if (specialEffectTriggered == false)
  {
    digitalWrite(13, HIGH);//Just pretend 12 is the special effect trigger
    specialEffectTriggered = true;
  }
}
int measureDistance()
{

  long duration, cm, inches;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  cm = (duration / 2) / 29.1;   // Divide by 29.1 or multiply by 0.0343
  return cm;
}
void setup()
{
  Serial.begin(9600);
  pinMode(outpin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(IRsensor, INPUT);
  irrecv.enableIRIn();
  weightLightMeter.beepBeep();
}

void loop()
{
  if  (irrecv.decode(&results))
  {
    Serial.println(results.value, HEX);
    switch (results.value)
    {
      case ZERO:
        weightLightMeter.setLevel(0);
        break;
      case ONE:
        weightLightMeter.setLevel(1);
        break;
      case TWO:
        weightLightMeter.setLevel(2);
        break;
      case THREE:
        weightLightMeter.setLevel(3);
        break;
      case FOUR:
        weightLightMeter.setLevel(4);
        break;
      case FIVE:
        weightLightMeter.setLevel(5);
        break;
      case SIX:
        weightLightMeter.setLevel(6);
        break;
      case SEVEN:
        weightLightMeter.setLevel(7);
        break;
      case EIGHT:
        weightLightMeter.setLevel(8);
        break;
      case NINE:
        weightLightMeter.flashyBlinky();
        break;
      case AUTO:
        weightLightMeter.beepBeep();
        autoMode = true;
        break;
    }
    irrecv.resume();
  }
  if (autoMode == true)
  {
    if (measureDistance() < 45)
    {
      digitalWrite(outpin, HIGH);
      weightLightMeter.flashyBlinky();
      autoMode = false;//"Disarm" the thingy
      digitalWrite(outpin, LOW);
    }
  }
}
