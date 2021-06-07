/*==============================================================================
** core.h -- Snake Private Structs and Macros.
**
** MODIFY HISTORY:
**
** 2021-06-06 wdf Create.
==============================================================================*/

#ifndef  __SNAKE_CORE_H__
#define  __SNAKE_CORE_H__

#include "os.h"
#include "msg.h"

/*======================================================================
  configs
======================================================================*/
#define JOB_QUEUE_MAX           60

#define MOV_TASK_PRI            100
#define JOB_TASK_PRI            100

/*======================================================================
  characteristic
======================================================================*/
#define SNAKE_BODY_MAX          1024

/*======================================================================
  FOREVER loop
======================================================================*/
#define SNAKE_FOREVER           for (;;)

/*======================================================================
  fast access
======================================================================*/
#define SNAKE_HEAD              (Snake_ID->body[0])
#define SNAKE_TAIL              (Snake_ID->body[Snake_ID->length - 1])

/*======================================================================
  when move a block, may return below vaules
======================================================================*/
typedef enum motion_status {
    MOTION_STATUS_OK        = 0,
    MOTION_STATUS_ERROR     = -1,
    MOTION_STATUS_GAME_OVER = -2
} MOTION_STATUS;

/*======================================================================
  the moving direction of the snake
======================================================================*/
typedef enum direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} DIRECTION;

/*======================================================================
  the coordinate of a node in screen
======================================================================*/
typedef struct coor {
    short x;
    short y;
} COOR;

/*======================================================================
  A Snake Game Instance
======================================================================*/
typedef struct Snake_Ctrl {
    void         (*show_node)(int x, int y, int color);
    void         (*show_msg)(int type, int value);
    int          (*input_msg)(void);

    int           col_num;
    int           row_num;

    COOR          food;
    COOR          body[SNAKE_BODY_MAX];
    int           length;
    DIRECTION     dir;

    int           score;
    int           count; /* total block number */
    int           speed; /* falling speed */
    int           start; /* start second */
    int           time;  /* run second */

    OS_MSG_Q_ID   job_queue;

    int           quit;
} SNAKE_CTRL;

#endif /* __SNAKE_CORE_H__ */

/*==============================================================================
** FILE END
==============================================================================*/

