
// struct Task
// {
//   unsigned long start;
//   unsigned long delay;
//   callback func;
// };

// unsigned long waitcall void waitcall(callback func, unsigned long delay)
// {
//   struct Task *task = malloc(sizeof(Task));
//   task->delay = delay;
//   task->func = func;
//   task->start = millis();
// }