#include "path.h"

#define ROWS_PER_BOX 5
#define NUM_BOXES 5

static bool programRunning = false;

static enum {
  OLD_MAN,
  // Increment box,row, determine if halt
  PATH_TO_PC,
  SCROLLING_DEPOSIT, // [box]
  PC_MOVE_POKEMON,
  NEXT_BOX, // skipped unless row == 0
  DOWN_TO_ROW, // [row]
  PRESS_A,
  UP_TO_PARTY, // [row+2]
  SWAP_WITH_PARTY,
  SCROLLING_MOVE, // [box]
  PC_WITHDRAW_POKEMON,
  DOWN_TO_ROW_2, // [row]
  WITHDRAW_AND_GO,
  LOOP
} programProgress;

static int box;
static int row;
static int scrollCur;
static int scrollAmt;
static int pathPos;

static step_t old_man_path[] = {
  { ACTION_A,      20 },
  { ACTION_NONE,   60 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 }
};

static step_t man_to_pc_path[] = {
  { ACTION_DOWN,   80 },
  { ACTION_LEFT,   55 },
  { ACTION_DOWN,  190 },
  { ACTION_LEFT,  170 },
  { ACTION_UP,    400 },
  { ACTION_RIGHT,  40 },
  { ACTION_UP,     40 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_DOWN,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   70 },
  { ACTION_SELECT, 20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   40 }
};

static step_t box_scrolling_path[] = {
  { ACTION_RIGHT,  20 },
  { ACTION_NONE,    1 }
};

static step_t deposit_path[] = {
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_B,      20 },
  { ACTION_NONE,   20 },
  { ACTION_B,      20 },
  { ACTION_NONE,   70 },
  { ACTION_DOWN,   20 },
  { ACTION_NONE,   10 },
  { ACTION_A,      20 },
  { ACTION_NONE,   70 },
  { ACTION_SELECT, 20 },
  { ACTION_NONE,   30 }
};

static step_t next_box_path[] = {
  { ACTION_R,      20 },
  { ACTION_NONE,   60 }
};

static step_t row_scrolling_path[] = {
  { ACTION_DOWN,   20 },
  { ACTION_NONE,   20 }
};

static step_t press_a_path[] = {
  { ACTION_A,      20 },
  { ACTION_NONE,   40 }
};

static step_t rev_row_scrolling_path[] = {
  { ACTION_UP,     20 },
  { ACTION_NONE,   20 }
};

static step_t swap_with_party_path[] = {
  { ACTION_A,      20 },
  { ACTION_NONE,   30 },
  { ACTION_LEFT,   10 },
  { ACTION_NONE,   20 },
  { ACTION_A,      20 },
  { ACTION_NONE,   20 },
  { ACTION_SELECT, 10 },
  { ACTION_NONE,   10 },
  { ACTION_A,      10 },
  { ACTION_NONE,   20 },
  { ACTION_DOWN,   10 },
  { ACTION_NONE,   10 },
  { ACTION_DOWN,   10 },
  { ACTION_NONE,   10 },
  { ACTION_A,      10 },
  { ACTION_NONE,   40 }
};

static step_t finish_move_path[] = {
  { ACTION_A,      10 },
  { ACTION_NONE,   10 },
  { ACTION_B,      20 },
  { ACTION_NONE,   20 },
  { ACTION_B,      10 },
  { ACTION_NONE,   10 },
  { ACTION_B,      20 },
  { ACTION_NONE,   70 },
  { ACTION_UP,     10 },
  { ACTION_NONE,   10 },
  { ACTION_UP,     10 },
  { ACTION_NONE,   10 },
  { ACTION_A,      20 },
  { ACTION_NONE,   70 },
  { ACTION_SELECT, 10 },
  { ACTION_NONE,   30 }
};

static step_t withdraw_path[] = {
  { ACTION_RIGHT,  10 },
  { ACTION_NONE,   10 },
  { ACTION_A,      20 },
  { ACTION_NONE,   90 },
  { ACTION_RIGHT,  10 },
  { ACTION_NONE,   10 },
  { ACTION_A,      20 },
  { ACTION_NONE,   90 },
  { ACTION_RIGHT,  10 },
  { ACTION_NONE,   10 },
  { ACTION_A,      20 },
  { ACTION_NONE,   90 },
  { ACTION_RIGHT,  10 },
  { ACTION_NONE,   10 },
  { ACTION_A,      20 },
  { ACTION_NONE,   90 },
  { ACTION_RIGHT,  10 },
  { ACTION_NONE,   10 },
  { ACTION_A,      20 },
  { ACTION_NONE,   90 },
  // Done withdrawing
  { ACTION_B,      10 },
  { ACTION_NONE,   10 },
  { ACTION_B,      20 },
  { ACTION_NONE,   70 },
  { ACTION_B,      20 },
  { ACTION_NONE,   20 },
  { ACTION_B,      20 },
  { ACTION_NONE,   20 },
  { ACTION_DOWN,   40 },
  { ACTION_LEFT,   50 },
  { ACTION_DOWN,  420 },
  { ACTION_UP,     15 },
  { ACTION_RIGHT, 170 },
  { ACTION_UP,    235 },
  { ACTION_RIGHT,  55 },
  { ACTION_UP,     30 },
  { ACTION_RIGHT,  20 },
  { ACTION_NONE,   40 }
};

static step_t* paths[] = {
  old_man_path,
  man_to_pc_path,
  box_scrolling_path,
  deposit_path,
  next_box_path,
  row_scrolling_path,
  press_a_path,
  rev_row_scrolling_path,
  swap_with_party_path,
  box_scrolling_path,
  finish_move_path,
  row_scrolling_path,
  withdraw_path
};

static uint16_t pathLens[] = {
  4,
  25,
  2,
  28,
  2,
  2,
  2,
  2,
  16,
  2,
  16,
  2,
  38
};

void startProgram()
{
  box = 0;
  row = 0;
  scrollCur = 0;
  scrollAmt = 0;
  programProgress = OLD_MAN;
  programRunning = true;
  pathPos = 0;
}

bool isProgramRunning()
{
  return programRunning;
}

const step_t* doProgram(unsigned int vblanks)
{
  if (programRunning)
  {
    if (vblanks >= paths[programProgress][pathPos].vcounts)
    {
      pathPos++;

      if (pathPos == pathLens[programProgress])
      {
        bool moveOn = true;
        pathPos = 0;

        if (programProgress == SCROLLING_DEPOSIT ||
            programProgress == DOWN_TO_ROW       ||
            programProgress == UP_TO_PARTY       ||
            programProgress == SCROLLING_MOVE    ||
            programProgress == DOWN_TO_ROW_2)
        {
          scrollCur++;

          if (scrollCur < scrollAmt)
          {
            moveOn = false;
          }
        }

        if (moveOn)
        {
          scrollCur = 0;

          if (programProgress == OLD_MAN)
          {
            row++;

            if (row == ROWS_PER_BOX)
            {
              row = 0;
              box++;

              if (box == NUM_BOXES)
              {
                programRunning = false;

                return 0;
              }
            }
          }

          programProgress++;
          if (programProgress == LOOP)
          {
            programProgress = OLD_MAN;
          }

          if (programProgress == NEXT_BOX && row != 0)
          {
            programProgress++;
          }

          if (programProgress == SCROLLING_DEPOSIT ||
              programProgress == SCROLLING_MOVE)
          {
            scrollAmt = box;

            if (row == 0)
            {
              scrollAmt--;
            }

            if (scrollAmt == 0)
            {
              programProgress++;
            }
          } else if (programProgress == DOWN_TO_ROW ||
                     programProgress == DOWN_TO_ROW_2)
          {
            scrollAmt = row;

            if (scrollAmt == 0)
            {
              programProgress++;
            }
          } else if (programProgress == UP_TO_PARTY)
          {
            scrollAmt = row + 2;
          }
        }
      }
    }

    if (programRunning)
    {
      return &paths[programProgress][pathPos];
    }
  }

  return 0;
}
