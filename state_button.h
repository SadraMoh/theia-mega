#ifndef STATE_BUTTON_H
#define STATE_BUTTON_H

const unsigned long DEBOUNCE = 50; // global debounce delay

typedef void (*callback)(struct StateButton *p);

struct StateButton
{
  unsigned int PIN;
  unsigned long DEB;
  int reading;
  int last_reading;
  int state;
  unsigned int counter;
  callback method; // Called
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
    }

    self->state = self->reading;
  }

  self->last_reading = self->reading;
}

#endif