#pragma region definitions

#define OSX 0
#define WINDOWS 1
#define UBUNTU 2

int platform = UBUNTU;

#include "Keyboard.h"
#include <LiquidCrystal_I2C.h>

const LiquidCrystal_I2C lcd(0x27, 16, 4);

#include "state_button.h"
#include "queue.h"

#define MOTOR_SPEED 120

// to prevent damage to the engine
#define MOTOR_SAFETY_DELAY 200

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

unsigned short BUZZER = 4U;
unsigned short SCAN_LED = 13U;

unsigned short CRADLE_OPEN_SENSOR = 12;  // BLUE
unsigned short CRADLE_CLOSE_SENSOR = 11; // GREEN
unsigned short CRADLE_OPEN_MOTOR = 10;   // PINK
unsigned short CRADLE_CLOSE_MOTOR = 9;   // WHITE

unsigned short GLASS_UP_SENSOR = 7;
unsigned short GLASS_DOWN_SENSOR = 8;
unsigned short GLASS_MOTOR_UP = 6;   // (RED LED)
unsigned short GLASS_MOTOR_DOWN = 5; // (GREEN LED)

const unsigned long CALIBRATION_DURATION = 30 * 1000U;

bool BottomTouchdownFlag = false; // Used in auto mode, has the pedal touched the bottom sensor.

bool IsCalibrating = false; // Is the calibration in action

void nothing(){};

void test(StateButton *self);

void switch_ord(StateButton *self);
void switch_led_side(StateButton *self);
void switch_mode(StateButton *self);

void handle_scan_button(StateButton *self);

void handle_glass_down_sensor(StateButton *self);
void handle_glass_up_sensor(StateButton *self);

void open_cradle(StateButton *self);

void close_cradle(StateButton *self);
void handle_cradle_open_sensor(StateButton *self);
void handle_cradle_close_sensor(StateButton *self);

void handle_pedal(StateButton *self);

void printSettings();

// attempts to spin the glass up
// does nothing if the up sensor won't allow it
bool spin_glass_up()
{
  if (digitalRead(GLASS_UP_SENSOR) == HIGH)
  {
    analogWrite(GLASS_MOTOR_UP, MOTOR_SPEED);
    analogWrite(GLASS_MOTOR_DOWN, 0);
    return true;
  }

  return false;
}

// attempts to spin the glass down
// does nothing if the down sensor won't allow it
bool spin_glass_down()
{
  if (digitalRead(GLASS_DOWN_SENSOR) == HIGH)
  {
    analogWrite(GLASS_MOTOR_DOWN, MOTOR_SPEED);
    analogWrite(GLASS_MOTOR_UP, 0);
    return true;
  }

  return false;
}

struct StateButton stateButtons[] = {
    {4U, 0U, 0, 0, 0, 0U, nothing},                     // 0  Buzzer
    {13U, 0U, 0, 0, 0, 0U, switch_led_side},            // 1  LED Mode
    {A0, 0U, 0, 0, 0, 0U, open_cradle},                 // 2  Cradle Open Button
    {A1, 0U, 0, 0, 0, 0U, close_cradle},                // 3  Cradle Close Button
    {A2, 0U, 0, 0, 0, 0U, handle_pedal},                // 4  Pedal
    {A3, 0U, 0, 0, 0, 0U, switch_ord},                  // 5  Scan Order Selector
    {A4, 0U, 0, 0, 0, 0U, switch_mode},                 // 6  Scan Mode & Calibration
    {A5, 0U, 0, 0, 0, 0U, handle_scan_button},          // 7  Scan
    {8U, 0U, 0, 1, 1, 0U, handle_glass_down_sensor},    // 8  Glass Down Sensor
    {7U, 0U, 0, 1, 1, 0U, handle_glass_up_sensor},      // 9  Glass Up Sensor
    {12U, 0U, 0, 1, 1, 0U, handle_cradle_open_sensor},  // 10 Cradle Open Sensor  (BLUE)
    {11U, 0U, 0, 1, 1, 0U, handle_cradle_close_sensor}, // 11 Cradle Clsoe Sensor (GREEN)
};

#pragma endregion definitions

// utility

void test(StateButton *self)
{
  lcd.clear();
  char str[4];
  sprintf(str, "%d", self->counter);
  lcd.print(str);
}

int count = 0;

void countup()
{
  lcd.clear();

  char str[4];
  sprintf(str, "%d", count++);
  lcd.setCursor(0, 0);
  lcd.print(str);
}

// alert

const int MESSAGE_DELAY = 3000;

void alert_lcd(char msg[32])
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg);
  waitcall(printSettings, MESSAGE_DELAY);
}

// order

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

// progress

unsigned short progress_state = 16;

char *progress_message;

unsigned long progress_duration = 0;

// calibrating...
// ================ <

// RECURSIVE
//
void progress_rev()
{

  // check for abortion
  if (IsCalibrating == false)
  {
    progress_state = 16;
    progress_duration = 0;
    return;
  }

  lcd.clear();

  // end recursion when state ends
  if (progress_state-- == 0)
  {
    progress_state = 16;
    printSettings();
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print(progress_message);

  lcd.setCursor(0, 1);
  for (unsigned short i = 0; i <= progress_state; i++)
    lcd.print(">");

  waitcall(progress_rev, floor(progress_duration / 16));
}

void progress_start(char msg[16], unsigned long duration)
{
  progress_state = 16;
  progress_message = msg;
  progress_duration = duration;

  lcd.clear();

  progress_rev();
}

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

void handle_scan_button(StateButton *self)
{
  if (stateButtons[7].counter % 2 == 1)
    return;

  switch (stateButtons[6].counter)
  {
  case PEDAL:
    send_scan_cmd();
    break;

  case PANEL:
    if (digitalRead(GLASS_DOWN_SENSOR) == LOW)
      send_scan_cmd();

    break;

  case AUTO:
    // disabled
    break;
  }
}

// glass

void handle_glass_down_sensor(StateButton *self)
{
  if (stateButtons[8].counter % 2 == 0)
    return;

  analogWrite(GLASS_MOTOR_DOWN, 0);

  if (stateButtons[6].counter == AUTO)
  {
    BottomTouchdownFlag = true;
    delay(200);
    send_scan_cmd();
    delay(1000);
    BottomTouchdownFlag = false;

    spin_glass_up();
  }
}

void handle_glass_up_sensor(StateButton *self)
{
  if (stateButtons[7].counter % 2 == 1)
    return;

  analogWrite(GLASS_MOTOR_UP, 0);
}

// cradle

void open_cradle(StateButton *self)
{
  if (stateButtons[2].counter % 2 == 0)
  {
    // release

    analogWrite(CRADLE_OPEN_MOTOR, 0);
  }
  else
  {
    // push

    if (digitalRead(CRADLE_OPEN_SENSOR) == HIGH)
      analogWrite(CRADLE_OPEN_MOTOR, MOTOR_SPEED);
  }
}

void close_cradle(StateButton *self)
{

  if (stateButtons[3].counter % 2 == 0)
  {
    // release

    analogWrite(CRADLE_CLOSE_MOTOR, 0);
  }
  else
  {
    // push

    if (digitalRead(CRADLE_CLOSE_SENSOR) == HIGH)
      analogWrite(CRADLE_CLOSE_MOTOR, MOTOR_SPEED);
  }
}

void handle_cradle_open_sensor(StateButton *self)
{
  if (stateButtons[10].counter % 2 == 0)
    return;

  analogWrite(CRADLE_OPEN_MOTOR, 0);
}

void handle_cradle_close_sensor(StateButton *self)
{
  if (stateButtons[11].counter % 2 == 0)
    return;

  analogWrite(CRADLE_CLOSE_MOTOR, 0);
}

void handle_pedal(StateButton *self)
{

  if (stateButtons[4].counter % 2 == 0)
  {
    // release

    switch (stateButtons[6].counter)
    {
    case PEDAL:
      send_scan_cmd();
      break;
    case PANEL:
      analogWrite(GLASS_MOTOR_DOWN, 0);

      // to avoid jitter clicking and getting the pedal stuck
      // only return panel when
      if (digitalRead(GLASS_UP_SENSOR) == HIGH)
      {
        delay(MOTOR_SAFETY_DELAY);
        spin_glass_up();
      }

      break;
    case AUTO:
      // return glass only if it hasn't come all the way down
      if (BottomTouchdownFlag == false)
      {
        analogWrite(GLASS_MOTOR_DOWN, 0);
        delay(MOTOR_SAFETY_DELAY);
        spin_glass_up();
      }
      break;
    }
  }
  else
  {
    // push

    switch (stateButtons[6].counter)
    {
    case PEDAL:
      send_scan_cmd();
      break;

    case PANEL:
    case AUTO:
      analogWrite(GLASS_MOTOR_UP, 0);
      delay(MOTOR_SAFETY_DELAY);
      spin_glass_down();
      break;

    case CALIB:

      if (!IsCalibrating)
      {

        if (!spin_glass_down())
          break;

        // start calibration
        IsCalibrating = true;
        progress_start("Calibration Mode", CALIBRATION_DURATION);
        waitcall([]()
                 {
                   digitalWrite(BUZZER, HIGH);

                   waitcall([]()
                            { 
                              // end calibration
                              digitalWrite(BUZZER, LOW);

                              spin_glass_up();
                             },
                            5640U); },
                 CALIBRATION_DURATION - 5000);
      }
      else
      {
        // abort calibration
        IsCalibrating = false;

        digitalWrite(BUZZER, LOW);
        clear_queue(); // !

        spin_glass_up();
        printSettings();
      }

      break;
    }
  }
}

// buzzer

void blink(){};

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
  lcd.print("SCN: ");
  lcd.setCursor(0, 1);

  switch (stateButtons[5].counter)
  {
  case 0:
  case 1:
    lcd.print("R>L  ");
    break;
  case 2:
  case 3:
    lcd.print("L>R  ");
    break;
  case 4:
  case 5:
    lcd.print("<R>  ");
    break;
  case 6:
  case 7:
    lcd.print("<L>  ");
    break;
  }

  lcd.setCursor(5, 0);
  lcd.print("LED:  ");
  lcd.setCursor(5, 1);

  switch (stateButtons[1].counter)
  {
  case 0:
  case 1:
    lcd.print("Side  ");
    break;
  case 2:
  case 3:
    lcd.print("Back  ");
    break;
  case 4:
  case 5:
    lcd.print("Both  ");
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
};

// put this in the loop function
void cycleStateButtons()
{
  for (int i = 0; i < sizeof(stateButtons) / sizeof(struct StateButton); i++)
  {
    state_button_check(&stateButtons[i]);
  }
};

void setup()
{

  // Button PinModes
  for (int i = 0; i < sizeof(stateButtons) / sizeof(struct StateButton); i++)
  {
    pinMode(stateButtons[i].PIN, INPUT);
  }

  pinMode(13, OUTPUT);

  pinMode(CRADLE_OPEN_SENSOR, INPUT);
  pinMode(CRADLE_CLOSE_SENSOR, INPUT);
  pinMode(CRADLE_OPEN_MOTOR, OUTPUT);
  pinMode(CRADLE_CLOSE_MOTOR, OUTPUT);

  pinMode(12, INPUT);
  pinMode(11, INPUT);

  pinMode(GLASS_MOTOR_UP, OUTPUT);
  pinMode(GLASS_MOTOR_DOWN, OUTPUT);

  Keyboard.begin();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(1, 0);
  lcd.print("Theia Pro Mega");
  lcd.setCursor(2, 1);
  lcd.print("Book Scanner");

  delay(2000); // bootup delay

  printSettings();

  Serial.begin(9600);
}

void loop()
{

  cycleStateButtons();

  // char str[24];
  // sprintf(str, "%lu", millis());
  // lcd.setCursor(0, 0);
  // lcd.print(str);

  doChores();

  // char str[4];
  // sprintf(str, "%d", digitalRead(A2));
  // lcd.print(str);
}
