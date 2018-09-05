#include "path.h"

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
  WITHDRAW_AND_GO,
  LOOP
} programProgress;

static int box;
static int row;
static int scrollCur;
static int scrollAmt;
static int pathPos;

static step_t old_man_path[] = {
  /*{ ACTION_UP,     60 },
  { ACTION_NONE,   60 },
  { ACTION_LEFT,   60 },
  { ACTION_NONE,   60 },
  { ACTION_DOWN,   60 },
  { ACTION_NONE,   60 },
  { ACTION_LEFT,   60 }*/
  { ACTION_A,      20 },
  { ACTION_NONE,   60 },
  { ACTION_A,      20 }
};

static step_t man_to_pc_path[] = {
};

static step_t box_scrolling_path[] = {
};

static step_t deposit_path[] = {
};

static step_t next_box_path[] = {
};

static step_t row_scrolling_path[] = {
};

static step_t press_a_path[] = {
};

static step_t rev_row_scrolling_path[] = {
};

static step_t swap_with_party_path[] = {
};

static step_t withdraw_path[] = {
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
  withdraw_path
};

static uint16_t pathLens[] = {
  sizeof old_man_path,
  sizeof man_to_pc_path,
  sizeof box_scrolling_path,
  sizeof deposit_path,
  sizeof next_box_path,
  sizeof row_scrolling_path,
  sizeof press_a_path,
  sizeof rev_row_scrolling_path,
  sizeof swap_with_party_path,
  sizeof box_scrolling_path,
  sizeof withdraw_path
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
            programProgress == SCROLLING_MOVE)
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

            if (row == 5)
            {
              row = 0;
              box++;

              if (box == 5)
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
          } else if (programProgress == DOWN_TO_ROW)
          {
            scrollAmt = row;
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
