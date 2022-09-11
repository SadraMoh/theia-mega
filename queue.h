typedef void (*taskCallback)(struct Task *p);

struct Task
{
  unsigned long start;
  unsigned long delay;
  taskCallback func;
};

const size_t TASK_POOL_SIZE = 16;

static const struct Task *TaskPool[TASK_POOL_SIZE];

void clear_queue()
{
  // insert task to the task pool
  for (size_t i = 0; i < TASK_POOL_SIZE; i++)
  {

    free(TaskPool[i]);
    TaskPool[i] = NULL;
  }
}

void waitcall(taskCallback func, unsigned long delay)
{
  struct Task *task = malloc(sizeof(*task));
  task->delay = delay;
  task->func = func;
  task->start = millis();

  // insert task to the task pool
  for (size_t i = 0; i < TASK_POOL_SIZE; i++)
    if (TaskPool[i] == NULL || TaskPool[i] == nullptr)
    {
      TaskPool[i] = task;
      return;
    }

  Serial.println(F("NO MORE SPACE IN TASK POOL"));
}

// put this in the loop function
// checks to see what tasks should be done, runs them and cleans them up
void doChores()
{

  for (size_t i = 0; i < TASK_POOL_SIZE; i++)
  {
    if (TaskPool[i] == NULL || TaskPool[i] == nullptr)
      continue;

    if ((TaskPool[i]->start + TaskPool[i]->delay) < millis())
    {
      // run task

      // Serial.print(F("Start: "));
      // Serial.println(TaskPool[i]->start);
      // Serial.print(F("Delay: "));
      // Serial.println(TaskPool[i]->delay);
      // Serial.print(F("Now: "));
      // Serial.println(millis());
      // Serial.print(F("Index: "));
      // Serial.println(i);

      // Serial.println(F("------------RAN-------------"));

      TaskPool[i]->func(0);

      free(TaskPool[i]);
      TaskPool[i] = NULL;
    }
  }
}