#ifndef PATH_H_07951612
#define PATH_H_07951612

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  enum {
    ACTION_NONE,
    ACTION_A,
    ACTION_B,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_START,
    ACTION_SELECT,
    ACTION_L,
    ACTION_R
  } action;
  uint16_t vcounts;
} step_t;

void startProgram();

bool isProgramRunning();

const step_t* doProgram(unsigned int vblanks);

#endif /* end of include guard: PATH_H_07951612 */
