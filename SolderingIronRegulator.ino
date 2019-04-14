#include <TM1637.h>
#include <EEPROM.h>

#define KEY 1
#define BTN_UP 0
#define BTN_DOWN 2
#define DIO 3
#define CLK 4

TM1637 disp(CLK, DIO);
const long timePeriodMax = 1000;
const long timePeriodMin = 500;
int timeOn;
bool btnUpPressed;
bool btnDnPressed;

void setup() {
  timeOn = readTimeOn();
  pinMode(KEY, OUTPUT);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  disp.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  disp.init();

  show(1000);
  digitalWrite(KEY, HIGH);
  delay(1000 * 60 * 2); // 2 minute
  delay(1000);
}

void loop() {
  btnUpPressed = !digitalRead(BTN_UP);
  btnDnPressed = !digitalRead(BTN_DOWN);

  if (btnUpPressed && btnDnPressed)
  {
    saveTimeOn(timeOn);
  }
  else if (btnUpPressed)
  {
    timeOn = increaseTimeOn(timeOn);
  }
  else if (btnDnPressed)
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
  number = number / 10;
  byte s = number / 100;
  number = number - s * 100;
  byte d = number / 10 ;
  byte e = number - d * 10;

  disp.display(0, 0);
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
  doBlink();
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
