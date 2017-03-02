#ifndef LOGGER
#define LOGGER

#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_ALERT -1
#define LOG_WARN -2

typedef struct {
  void (*const init) ();
  void (*const save) (int level, char * body);
  void (*const end) ();
} Log_class;

extern Log_class const Log;

#endif
