#include <TM1637.h>
#include <EEPROM.h>

#define BTN_INCREASE 0    // increase time
#define KEY 1
#define BTN_DECREASE 2  // decrease time
#define DIO 3
#define CLK 4
#define BTN_ONOFF 0 // analog 0

#define STATE_OFF 0
#define STATE_ON 1
#define STATE_START 2
#define STATE_STOP 3

TM1637 disp(CLK, DIO);
const long timePeriodMax = 1000;
const long timePeriodMin = 500;
const int startTimeLong = 120;
int timeOn;
bool btnIncPressed;
bool btnDecPressed;
bool btnOnOffIsPressed = false;
bool btnOnOffWasPressed = false;
byte state = 0;
unsigned long timeStart;

void setup() {
  timeOn = readTimeOn();
  pinMode(KEY, OUTPUT);
  pinMode(BTN_INCREASE, INPUT_PULLUP);
  pinMode(BTN_DECREASE, INPUT_PULLUP);
  disp.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  disp.init();
  state = STATE_STOP;
}

void loop()
{
  int i = analogRead(BTN_ONOFF);
  //  showOnOffLevel(i);
  btnOnOffIsPressed = 900 > i;

  if (btnOnOffIsPressed && !btnOnOffWasPressed)
  {
    if (state == STATE_OFF)
    {
      state = STATE_START;
      timeStart =  millis();
      timeOn = timePeriodMax;
      show(timeOn);
      digitalWrite(KEY, HIGH);
    }
    else
    {
      state = STATE_STOP;
    }
  }

  btnOnOffWasPressed = btnOnOffIsPressed;

  switch  (state)
  {
    case  STATE_STOP:
      {
        state = STATE_OFF;
        timeOn = 0;
        digitalWrite(KEY, timeOn);
        showOff();
      }
      break;
    case  STATE_START:
      {
        long t = (millis() - timeStart) / 1000;
        if ( t > startTimeLong)
        {
          state = STATE_ON;
          timeOn = readTimeOn();
        }
      }
      break;
    case  STATE_ON:
      {
        btnIncPressed = !digitalRead(BTN_INCREASE);
        btnDecPressed = !digitalRead(BTN_DECREASE);
        if (btnIncPressed && btnDecPressed)
        {
          saveTimeOn(timeOn);
        }
        else if (btnIncPressed)
        {
          timeOn = increaseTimeOn(timeOn);
        }
        else if (btnDecPressed)
        {
          timeOn = decreaseTimeOn(timeOn);
        }

        disp.point(POINT_ON);
        show(timeOn);
        digitalWrite(KEY, 1);
        delay(timeOn);

        if (timePeriodMax > timeOn)
        {
          digitalWrite(KEY, 0);
          disp.point(POINT_OFF);
          show(timeOn);
          delay(timePeriodMax - timeOn);
        }
      }
      break;
  }
}

int increaseTimeOn(int timeOn)
{
  return changeTimeOn(timeOn, 1);
}

int decreaseTimeOn(int timeOn)
{
  return changeTimeOn(timeOn, 0);
}


int changeTimeOn(int timeOn, bool up)
{
  byte delta = 20; // 20 ms
  if (up)
  {
    timeOn = timeOn + delta;
  }
  else
  {
    timeOn = timeOn - delta;
  }
  timeOn = checkRange(timeOn);
  return timeOn;
}

int checkRange(int timeOn)
{
  timeOn = min(timePeriodMax, timeOn);
  timeOn = max(timePeriodMin, timeOn);
  return timeOn;
}

void show(int number)
{
  byte blank = 0x7f;
  number = number / 10;
  byte s = number / 100;
  number = number - s * 100;
  byte d = number / 10 ;
  byte e = number - d * 10;

  if (s == 0) s = blank;
  if (s + d == 0) d = blank;

  disp.display(0, blank);
  disp.display(1, s);
  disp.display(2, d);
  disp.display(3, e);
}

void saveTimeOn(int r)
{
  EEPROM.write(1, r / 256);
  EEPROM.write(2, r % 256);
  doBlink();
}

int readTimeOn()
{
  int r = 0;
  r = EEPROM.read(1) * 256;
  r = r + EEPROM.read(2);
  r = checkRange(r);
  //doBlink();
  return r;
}

void doBlink()
{
  for (int i = 0; i < 3; i++)
  {
    disp.clearDisplay();
    delay(300);
    show(timeOn);
    delay(300);
  }
}

void showOff()
{
  disp.point(POINT_ON);
  disp.clearDisplay();
}

void showOnOffLevel (int i)
{
  for (int j = 0; j < 5; j++)
  {
    disp.clearDisplay();
    delay(50);
    show(i * 10);
    delay(50);
  }
}
