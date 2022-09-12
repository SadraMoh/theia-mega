#ifndef STATE_BUTTON_H
#define STATE_BUTTON_H

const unsigned long DEBOUNCE = 24; // global debounce delay

typedef void (*buttonCallback)(struct StateButton *p);

struct StateButton
{
  unsigned int PIN;
  unsigned long DEB;
  int reading;
  int last_reading;
  int state;
  unsigned int counter;
  buttonCallback method; // Called
};

void state_button_check(StateButton *self)
{
  self->reading = digitalRead(self->PIN);

  if (self->reading != self->last_reading)
  {
    self->DEB = millis();
  }

  if (millis() - self->DEB > DEBOUNCE)
  {
    // reading is now the legitimate button state

    if (self->reading != self->state)
    {
      self->counter = self->counter + 1;

      self->method(self);

      Serial.print(F("[EVENT] "));
      Serial.print(self->PIN);
      Serial.println();
    }

    self->state = self->reading;
  }

  self->last_reading = self->reading;
}

#endif