/*==============================================================================
** msg.h -- Snake share Structs between core and ui.
**
** MODIFY HISTORY:
**
** 2011-06-06 wdf Create.
==============================================================================*/

#ifndef  __SNAKE_MSG_H__
#define  __SNAKE_MSG_H__

/*======================================================================
  Tetris job_task can process these messages
======================================================================*/
typedef enum msg_job {
    MSG_JOB_QUIT,
    MSG_JOB_AHEAD,
    MSG_JOB_UP,
    MSG_JOB_DOWN,
    MSG_JOB_RIGHT,
    MSG_JOB_LEFT,
    MSG_JOB_HOLD,
    MSG_JOB_NOP
} MSG_JOB;

/*======================================================================
  Tetris send to ui show message type
======================================================================*/
typedef enum msg_show {
    MSG_SHOW_SCORE,
    MSG_SHOW_COUNT,
    MSG_SHOW_SPEED,
    MSG_SHOW_TIME
} MSG_SHOW;

#endif /* __SNAKE_MSG_H__ */
/*==============================================================================
** FILE END
==============================================================================*/

