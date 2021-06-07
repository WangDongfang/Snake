/*==============================================================================
** core.c -- Snake core.
**
** MODIFY HISTORY:
**
** 2021-06-06 wdf Create.
==============================================================================*/

#include <stdio.h>         /* prinf() */
#include <stdlib.h>        /* srand(), rand() */
#include <string.h>        /* memcpy() */

#include "core.h"

/*======================================================================
  debug config
======================================================================*/
#define SNAKE_DEBUG      printf

/*======================================================================
  forward function declare
======================================================================*/
static void new_food (SNAKE_CTRL *Snake_ID);
static void eat_food (SNAKE_CTRL *Snake_ID);

/*==============================================================================
 * - is_occupied()
 *
 * - on screen whether this coordinate is occupied
 *
 * - return value
 *   0: not occupied
 *  !0: occupied
 */
static int is_occupied (const SNAKE_CTRL *Snake_ID, const COOR *coor)
{
    int i;

    if (((coor->x >= 0) && (coor->x < Snake_ID->col_num)) &&
        ((coor->y >= 0) && (coor->y < Snake_ID->row_num))) {
        for (i = 0; i < Snake_ID->length; i++) {
            if ((Snake_ID->body[i].x == coor->x) &&
                (Snake_ID->body[i].y == coor->y)) {
                return 1;
            }
        }
    } else {
        return 1;
    }

    return 0;
}

/*==============================================================================
 * - update_speed()
 *
 * - update speed value
 */
static void update_speed (SNAKE_CTRL *Snake_ID)
{
    if (Snake_ID->count % 7 == 0) {
        Snake_ID->speed++;
        Snake_ID->show_msg (MSG_SHOW_SPEED, Snake_ID->speed);
    }
}

/*==============================================================================
 * - show_food()
 *
 * - erase of show the moving block
 */
static void show_food (SNAKE_CTRL *Snake_ID)
{
    int color;

    color = 1 + rand() % 7;
    Snake_ID->show_node (Snake_ID->food.x, Snake_ID->food.y, color);
}

/*==============================================================================
 * - new_food()
 *
 * - create a new food rand on screen
 */
static void new_food (SNAKE_CTRL *Snake_ID)
{
    /* create and show the new food */
    do {
        Snake_ID->food.x = rand() % Snake_ID->col_num;
        Snake_ID->food.y = rand() % Snake_ID->row_num;

    } while (is_occupied(Snake_ID, &(Snake_ID->food)));

    show_food (Snake_ID);

    /* show count message */
    Snake_ID->count++;
    Snake_ID->show_msg (MSG_SHOW_COUNT, Snake_ID->count);
    update_speed (Snake_ID);
}

/*==============================================================================
 * - dump_snake()
 *
 * - dump snake body node coordinate
 */
static void dump_snake (char *prefix, SNAKE_CTRL *Snake_ID)
{
#if 0
    static int row = 20;
    static int color = 31;
    int i;

    SNAKE_DEBUG("\033[40m");
    SNAKE_DEBUG("\033[%dm", color);

    SNAKE_DEBUG("\033[%d;%dH", row, 0);
    SNAKE_DEBUG("%s Len = %d: ", prefix, Snake_ID->length);
    for (i = 0; i < Snake_ID->length - 1; i++) {
        SNAKE_DEBUG("[%d,%d], ", (int)Snake_ID->body[i].x, (int)Snake_ID->body[i].y);
    }
    SNAKE_DEBUG("[%d,%d]\r\n", (int)Snake_ID->body[i].x, (int)Snake_ID->body[i].y);

    row++;
    if (row > 30) {
        row = 20;
        color = (color == 37) ? 31 : (color + 1);
    }
#endif
}
/*==============================================================================
 * - eat_food()
 *
 * - grown up
 */
static void eat_food (SNAKE_CTRL *Snake_ID)
{
    int i;

    for (i = Snake_ID->length; i > 0; i--) {
        Snake_ID->body[i] = Snake_ID->body[i - 1];
    }
    SNAKE_HEAD = Snake_ID->food;

    Snake_ID->length++;
    Snake_ID->score++;
    Snake_ID->show_msg (MSG_SHOW_SCORE, Snake_ID->score);;

    new_food(Snake_ID);
}

/*==============================================================================
 * - goahead()
 *
 * - try to move snake ahead
 *
 * - return value
 *   MOTION_STATUS_OK:        ok
 *   MOTION_STATUS_ERROR:     can't down 
 *   MOTION_STATUS_GAME_OVER: game over
 */
static int goahead (SNAKE_CTRL *Snake_ID)
{
    int i;
    COOR coor;

    switch (Snake_ID->dir) {
    case DIR_UP:
        coor.x = SNAKE_HEAD.x;
        coor.y = (SNAKE_HEAD.y == 0) ? (Snake_ID->row_num - 1) : (SNAKE_HEAD.y - 1);
        break;

    case DIR_DOWN:
        coor.x = SNAKE_HEAD.x;
        coor.y = (SNAKE_HEAD.y + 1) % Snake_ID->row_num;
        break;

    case DIR_LEFT:
        coor.x = (SNAKE_HEAD.x == 0) ? (Snake_ID->col_num - 1) : (SNAKE_HEAD.x - 1);
        coor.y = SNAKE_HEAD.y;
        break;

    case DIR_RIGHT:
        coor.x = (SNAKE_HEAD.x + 1) % Snake_ID->col_num;
        coor.y = SNAKE_HEAD.y;
        break;
        
    default:
        return MOTION_STATUS_ERROR;
    }

    if (is_occupied(Snake_ID, &coor) == 0) {
        if ((coor.x == Snake_ID->food.x) && (coor.y == Snake_ID->food.y)) {
            dump_snake("before eat food.", Snake_ID);

            eat_food (Snake_ID);

            dump_snake(" after eat food.", Snake_ID);
        } else {
            dump_snake("before goahead.", Snake_ID);

            Snake_ID->show_node (SNAKE_TAIL.x, SNAKE_TAIL.y, 0);
            for (i = Snake_ID->length - 1; i > 0; i--) {
                Snake_ID->body[i] = Snake_ID->body[i - 1];
            }
            SNAKE_HEAD = coor;
            Snake_ID->show_node (SNAKE_HEAD.x, SNAKE_HEAD.y, 1);

            dump_snake(" after goahead.", Snake_ID);
        }

    } else {
        return MOTION_STATUS_GAME_OVER;
    }

    return MOTION_STATUS_OK;
}


/*==============================================================================
 * - sk_send_msg()
 *
 * - send a message to job queue
 *
 * -  0: send to the message queue success
 *   -1: send to the message queue failed
 */
static int sk_send_msg (SNAKE_CTRL *Snake_ID, MSG_JOB msg)
{
    int os_send_ret;

    os_send_ret = os_msgQ_send (Snake_ID->job_queue, (char *)&msg, sizeof (MSG_JOB));

    return os_send_ret;
}

/*==============================================================================
 * - sk_recv_msg()
 *
 * - receive message from job queue. Only used by job_task
 */
static MSG_JOB sk_recv_msg (SNAKE_CTRL *Snake_ID)
{
    MSG_JOB msg_job;

    os_msgQ_receive (Snake_ID->job_queue, (char *)&msg_job, sizeof (MSG_JOB));

    return msg_job;
}

/*==============================================================================
 * - mov_task()
 *
 * - a task that send MSG_JOB_AHEAD regularly
 */
static void mov_task (SNAKE_CTRL *Snake_ID)
{
    SNAKE_FOREVER {
        if (Snake_ID->quit == 1) {
            break;
        }
        sk_send_msg (Snake_ID, MSG_JOB_AHEAD);
        os_task_delay (1000 / Snake_ID->speed);
    }
    Snake_ID->quit = 2;
}

/*==============================================================================
 * - job_task()
 *
 * - a task that receive MSG_JOB and process
 */
static void job_task (SNAKE_CTRL *Snake_ID)
{
    MSG_JOB msg_job;
    int hold_on = 0;
    int job_exit = 0;
    int turn_time = 0;

    SNAKE_FOREVER {
        msg_job = sk_recv_msg (Snake_ID);

        /* show score, number, speed, time */
        Snake_ID->time = (os_time_get() - Snake_ID->start) / 1000;
        Snake_ID->show_msg (MSG_SHOW_TIME, Snake_ID->time);

        switch (msg_job) {
            case MSG_JOB_AHEAD:
                if (hold_on) { break; }
                if ((os_time_get() - turn_time) < (500 / Snake_ID->speed)) {
                    break;
                }
                if (goahead (Snake_ID) == MOTION_STATUS_GAME_OVER) {
                    job_exit = 1;
                }
                break;
            case MSG_JOB_LEFT:
                if (hold_on) { break; }
                if (Snake_ID->dir == DIR_UP || Snake_ID->dir == DIR_DOWN) {
                    Snake_ID->dir = DIR_LEFT;
                    turn_time = os_time_get();
                    if (goahead (Snake_ID) == MOTION_STATUS_GAME_OVER) {
                        job_exit = 1;
                    }
                }
                break;
            case MSG_JOB_RIGHT:
                if (hold_on) { break; }
                if (Snake_ID->dir == DIR_UP || Snake_ID->dir == DIR_DOWN) {
                    Snake_ID->dir = DIR_RIGHT;
                    turn_time = os_time_get();
                    if (goahead (Snake_ID) == MOTION_STATUS_GAME_OVER) {
                        job_exit = 1;
                    }
                }
                break;
            case MSG_JOB_UP:
                if (hold_on) { break; }
                if (Snake_ID->dir == DIR_LEFT || Snake_ID->dir == DIR_RIGHT) {
                    Snake_ID->dir = DIR_UP;
                    turn_time = os_time_get();
                    if (goahead (Snake_ID) == MOTION_STATUS_GAME_OVER) {
                        job_exit = 1;
                    }
                }
                break;
            case MSG_JOB_DOWN:
                if (hold_on) { break; }
                if (Snake_ID->dir == DIR_LEFT || Snake_ID->dir == DIR_RIGHT) {
                    Snake_ID->dir = DIR_DOWN;
                    turn_time = os_time_get();
                    if (goahead (Snake_ID) == MOTION_STATUS_GAME_OVER) {
                        job_exit = 1;
                    }
                }
                break;
            case MSG_JOB_HOLD:
                hold_on = 1 - hold_on;
                break;
            case MSG_JOB_QUIT:
                job_exit = 1;
                break;
            default:
                break;
        }

        if (job_exit == 1) {
            break;
        }
    }

    Snake_ID->quit = 1;
}

/*==============================================================================
 * - sk_init()
 *
 * - create job queue and start job_task & down_task
 */
static void sk_init (SNAKE_CTRL *Snake_ID)
{
    /* create msgQ */
    Snake_ID->job_queue = os_msgQ_create (JOB_QUEUE_MAX, sizeof (MSG_JOB));

    srand (os_time_get());

    Snake_ID->body[0].x = 0;
    Snake_ID->body[0].y = Snake_ID->row_num / 3;
    Snake_ID->length    = 1;

    /* update and show moving block */
    new_food (Snake_ID);

    /* create do job task */
    os_task_create ("tSk_job", 40960, JOB_TASK_PRI, (void *)job_task, (int)Snake_ID, 2);
    os_task_create ("tSk_mov", 10240, MOV_TASK_PRI, (void *)mov_task, (int)Snake_ID, 2);

    /* show score, number, speed, time */
    Snake_ID->show_msg (MSG_SHOW_SCORE, Snake_ID->score);
    Snake_ID->show_msg (MSG_SHOW_COUNT, Snake_ID->count);
    Snake_ID->show_msg (MSG_SHOW_SPEED, Snake_ID->speed);
    Snake_ID->show_msg (MSG_SHOW_TIME,  Snake_ID->time);
}

/*==============================================================================
 * - sk_ctrl()
 *
 * - read and send user input message.
 */
static void sk_ctrl (SNAKE_CTRL *Snake_ID)
{
    int user_input_msg;

    SNAKE_FOREVER {
        user_input_msg = Snake_ID->input_msg();

        if (Snake_ID->quit != 0) {
            break;
        }

        if (user_input_msg >= MSG_JOB_QUIT && user_input_msg <= MSG_JOB_HOLD) {
            sk_send_msg (Snake_ID, user_input_msg);

            if (user_input_msg == MSG_JOB_QUIT) {
                break;
            }
        }
    }
}

/*==============================================================================
 * - sk_quit()
 *
 * - game over, delete job queue
 */
static void sk_quit (SNAKE_CTRL *Snake_ID)
{
    /* wait down_task quit */
    if (Snake_ID->quit != 2) {
        os_task_delay (100);
    }

    /* delete job_queue */
    os_msgQ_delete (Snake_ID->job_queue);
}

/*==============================================================================
 * - sanke_run()
 *
 * - run the game and proccess user input.
 */
void snake_run (int col_num, int row_num,
                void (*show_node)(int x, int y, int show),
                void (*show_msg)(int type, int value),
                int  (*input_msg)(void))
{
    SNAKE_CTRL *Snake_ID = NULL;

    /* alloc Snake_ID */
    Snake_ID = (SNAKE_CTRL *)malloc (sizeof (SNAKE_CTRL));
    
    if (Snake_ID == NULL) {
        SNAKE_DEBUG ("There is no memory for <Snake_ID>!\n");
        return;
    }

    /* init Snake_ID */
    Snake_ID->show_node = show_node;
    Snake_ID->show_msg  = show_msg;
    Snake_ID->input_msg = input_msg;
    Snake_ID->col_num   = col_num;
    Snake_ID->row_num   = row_num;
    Snake_ID->dir       = DIR_RIGHT;
    Snake_ID->score     = 0;
    Snake_ID->count     = 0;
    Snake_ID->speed     = 1;
    Snake_ID->start     = os_time_get();
    Snake_ID->time      = 0;
    Snake_ID->quit      = 0;

    sk_init(Snake_ID);
    sk_ctrl(Snake_ID);
    sk_quit(Snake_ID);

    /* free Snake module control struct */
    /* SNAKE_DEBUG ("Snake_ID = %p\n", Snake_ID); */
    free (Snake_ID);
}

/*==============================================================================
** FILE END
==============================================================================*/
