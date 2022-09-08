
#define OSX 0
#define WINDOWS 1
#define UBUNTU 2

int platform = UBUNTU;

#include "Keyboard.h"

#include <LiquidCrystal_I2C.h>
const LiquidCrystal_I2C lcd(0x27, 16, 4);

#include "state_button.h"

// order
#define RTL 0
#define LTR 2
#define R 4
#define L 6

// led side
#define SIDE 0
#define BACK 2
#define BOTH 4

// mode
#define AUTO 0
#define PANEL 2
#define PEDAL 4
#define CALIB 6

void test(StateButton *self)
{
  lcd.clear();
  char str[4];
  sprintf(str, "%d", self->counter);
  lcd.print(str);
}

void switch_ord(StateButton *self)
{
  if (self->counter > 7)
    self->counter = RTL;

  printSettings();
}

void switch_led_side(StateButton *self)
{
  if (self->counter > 5)
    self->counter = SIDE;

  printSettings();
}

void switch_mode(StateButton *self)
{
  if (self->counter > 7)
    self->counter = AUTO;

  printSettings();
}

void nothing(){};

void scan(StateButton *self);
void handle_glass_down(StateButton *self);

unsigned short CRADLE_OPEN_SENSOR = 12;
unsigned short CRADLE_CLOSE_SENSOR = 11;
unsigned short CRADLE_OPEN_MOTOR = 10;
unsigned short CRADLE_CLOSE_MOTOR = 9;

unsigned short GLASS_DOWN_SENSOR = 8;
unsigned short GLASS_UP_SENSOR = 7;
unsigned short GLASS_MOTOR_UP = 6;
unsigned short GLASS_MOTOR_DOWN = 5;

struct StateButton stateButtons[] = {
    {04U, 0U, 0, 0, 0, 0U, nothing},          // 0 TEST
    {13U, 0U, 0, 0, 0, 0U, switch_led_side},  // 1 LED Mode
    {A0, 0U, 0, 1, 1, 0U, test},              // 2 Cradle Open
    {A1, 0U, 0, 0, 0, 0U, nothing},           // 3 Cradle Close
    {A2, 0U, 0, 0, 0, 0U, nothing},           // 4 Pedal
    {A3, 0U, 0, 0, 0, 0U, switch_ord},        // 5 Scan Order Selector
    {A4, 0U, 0, 0, 0, 0U, switch_mode},       // 6 Scan Mode & Calibration
    {A5, 0U, 0, 0, 0, 0U, scan},              // 7 Scan
    {8U, 0U, 0, 1, 1, 0U, handle_glass_down}, // 8 Glass down Sensor
};

// send scan command according to scan mode
void send_scan_cmd()
{
  switch (stateButtons[5].counter)
  {
  case RTL:
    Keyboard.press('R');
    Keyboard.release('R');
    delay(200);
    Keyboard.press('L');
    Keyboard.release('L');
    break;

  case LTR:
    Keyboard.press('L');
    Keyboard.release('L');
    delay(200);
    Keyboard.press('R');
    Keyboard.release('R');
    break;

  case R:
    Keyboard.press('R');
    Keyboard.release('R');
    break;

  case L:
    Keyboard.press('L');
    Keyboard.release('L');
    break;
  }
}

void scan(StateButton *self)
{
  if (stateButtons[7].counter % 2 == 1)
    return;

  if (stateButtons[6].counter == PANEL || stateButtons[6].counter == PEDAL)
  {
    send_scan_cmd();
  }
}

void handle_glass_down(StateButton *self)
{
  if (stateButtons[8].counter % 2 == 0)
    return;

  delay(200);

  if (stateButtons[6].counter == AUTO)
  {
    send_scan_cmd();
  }
}

// buzzer

void setup()
{

  // Button PinModes
  for (int i = 0; i < sizeof(stateButtons) / sizeof(struct StateButton); i++)
  {
    pinMode(stateButtons[i].PIN, INPUT);
  }

  pinMode(17, INPUT_PULLUP);

  pinMode(CRADLE_OPEN_SENSOR, INPUT);
  pinMode(CRADLE_CLOSE_SENSOR, INPUT);
  pinMode(CRADLE_OPEN_MOTOR, OUTPUT);
  pinMode(CRADLE_CLOSE_MOTOR, OUTPUT);
  pinMode(GLASS_DOWN_SENSOR, INPUT);
  pinMode(GLASS_UP_SENSOR, INPUT);
  pinMode(GLASS_MOTOR_UP, OUTPUT);
  pinMode(GLASS_MOTOR_DOWN, OUTPUT);

  Keyboard.begin();

  lcd.init();
  lcd.backlight();

  // lcd.setCursor(1, 0);
  // lcd.print("Theia Pro Mega");
  // lcd.setCursor(2, 1);
  // lcd.print("Book Scanner");

  // delay(200); // bootup delay

  printSettings();
}

void printSettings()
{
  // lcd.clear();

  //|ORD  LED   MODE |
  //|L>R  side  PEDAL|
  // ^^^^^^^^^^^^^^^^

  // R>L, L>R, R, L
  // side, back, both
  // auto, panel, pedal

  lcd.setCursor(0, 0);
  lcd.print("SCN:");
  lcd.setCursor(0, 1);

  switch (stateButtons[5].counter)
  {
  case 0:
  case 1:
    lcd.print("R>L");
    break;
  case 2:
  case 3:
    lcd.print("L>R");
    break;
  case 4:
  case 5:
    lcd.print("<R>");
    break;
  case 6:
  case 7:
    lcd.print("<L>");
    break;
  }

  lcd.setCursor(5, 0);
  lcd.print("LED:");
  lcd.setCursor(5, 1);

  switch (stateButtons[1].counter)
  {
  case 0:
  case 1:
    lcd.print("Side");
    break;
  case 2:
  case 3:
    lcd.print("Back");
    break;
  case 4:
  case 5:
    lcd.print("Both");
    break;
  }

  lcd.setCursor(11, 0);
  lcd.print("MODE:");
  lcd.setCursor(11, 1);

  switch (stateButtons[6].counter)
  {
  case 0:
  case 1:
    lcd.print("Auto ");
    break;
  case 2:
  case 3:
    lcd.print("Panel");
    break;
  case 4:
  case 5:
    lcd.print("Pedal");
    break;
  case 6:
  case 7:
    lcd.print("Calib");
    break;
  }
}

// put this in the loop function
void cycleStateButtons()
{
  for (int i = 0; i < sizeof(stateButtons) / sizeof(struct StateButton); i++)
  {
    state_button_check(&stateButtons[i]);
  }
};

void loop()
{
  cycleStateButtons();

  // char str[4];
  // sprintf(str, "%d", digitalRead(A0));
  // lcd.print(str);
}
