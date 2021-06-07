/*==============================================================================
** tty_snake.c -- a snake game in tty.
**
** MODIFY HISTORY:
**
** 2011-09-29 wdf Create.
==============================================================================*/

#include <stdio.h>
#include <string.h>  /* strcmp() */
#include <fcntl.h>   /* open() read() write() close() */
#include <ctype.h>   /* isprint() */
#include "msg.h"

/*==============================================================================
  show config
==============================================================================*/
#define serial_printf   printf
#define serial_putc     putchar
#define serial_read     read
/*==============================================================================
  show config
==============================================================================*/
#define NODE_WIDTH          2    /* char too thin, so a node occupy two char */
#define NODE_ELEMENT        "__" /* node's chars */
#define CURSOR_COL_START    2    /* the fisrt node cursor column */
#define CURSOR_ROW_START    4    /* the fisrt node cursor row */
#define NODE_LR_NUM         30   /* left -- right node number */
#define NODE_TB_NUM         15   /* top -- bottom node number */
/*==============================================================================
  VT100+ Color codes
==============================================================================*/
#define COLOR_FG_BLACK  "\033[30m"             /*  ºÚ   ×Ö                    */
#define COLOR_FG_RED    "\033[31m"             /*  ºì   ×Ö                    */
#define COLOR_FG_GREEN  "\033[32m"             /*  ÂÌ   ×Ö                    */
#define COLOR_FG_YELLOW "\033[33m"             /*  »Æ   ×Ö                    */
#define COLOR_FG_BLUE   "\033[34m"             /*  À¶   ×Ö                    */
#define COLOR_FG_MAGEN  "\033[35m"             /*  Æ·ºì ×Ö                    */
#define COLOR_FG_CYAN   "\033[36m"             /*  Çà   ×Ö                    */
#define COLOR_FG_WHITE  "\033[37m"             /*  °×   ×Ö                    */
#define COLOR_FG_DEF    "\033[0m"              /*  Ä¬ÈÏÑÕÉ«                   */

#define COLOR_BG_BLACK  "\033[40m"             /*  ºÚ   µ×                    */
#define COLOR_BG_RED    "\033[41m"             /*  ºì   µ×                    */
#define COLOR_BG_GREEN  "\033[42m"             /*  ÂÌ   µ×                    */
#define COLOR_BG_YELLOW "\033[43m"             /*  »Æ   µ×                    */
#define COLOR_BG_BLUE   "\033[44m"             /*  À¶   µ×                    */
#define COLOR_BG_MAGEN  "\033[45m"             /*  Æ·ºì µ×                    */
#define COLOR_BG_CYAN   "\033[46m"             /*  Çà   µ×                    */
#define COLOR_BG_WHITE  "\033[47m"             /*  °×   µ×                    */

#define CLEAR_SCREEN    "\033[H\033[0J"        /*  ÇåÆÁ                       */
#define HIDE_CURSOR     "\033[?25l"            /*  Òþ²Ø¹â±ê                   */
#define SHOW_CURSOR     "\033[?25h"            /*  ÏÔÊ¾¹â±ê                   */
/*===============================================================================
  KeyBoard Strings
===============================================================================*/
#define KEY_ESC         "\033"
#define KEY_UP          "\033[A"
#define KEY_DOWN        "\033[B"
#define KEY_RIGHT       "\033[C"
#define KEY_LEFT        "\033[D"
#define KEY_SPACE       " "         /* fix block at bottom */
#define KEY_HOLD        "h"         /* freeze snake */
/*===============================================================================
  the Tetris core support only API function
===============================================================================*/
void snake_run (int column_num, int row_num,
                void (*show_node)(int x, int y, int show),
                void (*show_msg)(int type, int value),
                int  (*input_msg)(void));
/*===============================================================================
  global variables
===============================================================================*/
static int      _G_score;

/*==============================================================================
 * - _draw_frame()
 *
 * - the frame looks like this:
 *   +------------------------+
 *   |                        |
 *   +------------------------+
 *   |                        |
 *   |                        |
 *   |                        |
 *   |                        |
 *   |                        |
 *   +------------------------+
 */
static void _draw_frame (void)
{
    int i;

    /*
     * draw the hat, this area is for show score
     */
    
    /* move cursor to hat left top  */
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START - 3, CURSOR_COL_START - 1);
    /* +--------------+ */
    /* |              | */
    serial_putc ('+');
    for (i = 0; i < NODE_LR_NUM * NODE_WIDTH; i++) serial_putc ('-');
    serial_putc ('+');

    serial_printf ("\033[%d;%dH", CURSOR_ROW_START - 2, CURSOR_COL_START - 1);
    serial_putc ('|');
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START - 2, CURSOR_COL_START + NODE_LR_NUM * NODE_WIDTH);
    serial_putc ('|');
    printf("\r\n");

    /*
     * draw the body, this area is for game
     */

    /* move cursor to left top */
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START - 1, CURSOR_COL_START - 1);
    /* +--------------+ */
    serial_putc ('+');
    for (i = 0; i < NODE_LR_NUM * NODE_WIDTH; i++) serial_putc ('-');
    serial_putc ('+');
    printf("\r\n");

    /* draw left & right wall */
    for (i = 0; i < NODE_TB_NUM; i++) {
        serial_printf ("\033[%d;%dH", CURSOR_ROW_START+i, CURSOR_COL_START - 1);
        serial_putc ('|');
        serial_printf ("\033[%d;%dH", CURSOR_ROW_START+i, CURSOR_COL_START + NODE_LR_NUM * NODE_WIDTH);
        serial_putc ('|');
    }
    printf("\r\n");

    /* move cursor to left bottom */
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START + NODE_TB_NUM, CURSOR_COL_START - 1);
    /* +--------------+ */
    serial_putc ('+');
    for (i = 0; i < NODE_LR_NUM * NODE_WIDTH; i++) serial_putc ('-');
    serial_putc ('+');
    printf("\r\n");
}

/*==============================================================================
 * - _draw_node()
 *
 * - draw the (x, y) position node, it looks like this:
 *   __
 */
static void _draw_node (int x, int y, int color)
{
    /* move cursor */
    serial_printf ("\033[%d;%dH", CURSOR_ROW_START + y, CURSOR_COL_START + (x * NODE_WIDTH));

    /* set foreground color as BLACK */
    serial_printf (COLOR_FG_BLACK);

    /* set background color */
    serial_printf ("\033[%dm", 40 + color);

    /* show or erase node */
    serial_printf (NODE_ELEMENT);
    printf("\r\n");
}

/*==============================================================================
 * - _draw_msg()
 *
 * - draw the snake message, it looks like this:
 */
static void _draw_msg (int type, int value)
{
    int start_col;
    int start_row;

    /* set background & foreground color */
    serial_printf (COLOR_BG_BLACK);
    serial_printf (COLOR_FG_YELLOW);

    switch (type) {
    case MSG_SHOW_SCORE:
        start_col = CURSOR_COL_START;
        start_row = CURSOR_ROW_START - 2;

        /* move cursor */
        serial_printf ("\033[%d;%dH", start_row, start_col);
        serial_printf ("Score: %d", value);

        _G_score = value;
        break;

    case MSG_SHOW_COUNT:
        start_col = CURSOR_COL_START + 12;
        start_row = CURSOR_ROW_START - 2;

        /* move cursor */
        serial_printf ("\033[%d;%dH", start_row, start_col);
        serial_printf ("No.:%d", value);
        break;

    case MSG_SHOW_SPEED:
        start_col = CURSOR_COL_START + 21;
        start_row = CURSOR_ROW_START - 2;

        /* move cursor */
        serial_printf ("\033[%d;%dH", start_row, start_col);
        serial_printf ("Speed: %d", value);
        break;

    case MSG_SHOW_TIME:
        start_col = CURSOR_COL_START + (NODE_LR_NUM * NODE_WIDTH) - 16;
        start_row = CURSOR_ROW_START - 2;

        /* move cursor */
        serial_printf ("\033[%d;%dH", start_row, start_col);
        serial_printf ("Time: %02d:%02d:%02d", value/3600, value%3600/60, value%60);
        break;

    default:
        break;
    }

    printf("\r\n");
}

/*==============================================================================
 * - _input_msg()
 *
 * - get user input key.
 */
static int _input_msg (void)
{
    int  ret;
    char key[4];
    int  nread;

    nread = serial_read (STD_IN, key, 3);
    key[nread] = '\0';

    if (strcmp (key, KEY_ESC) == 0) {           /* Esc: quit */
        ret = MSG_JOB_QUIT;
    } else if (strcmp (key, KEY_UP) == 0) {     /* Up */
        ret = MSG_JOB_UP;
    } else if (strcmp (key, KEY_DOWN) == 0) {   /* Down */
        ret = MSG_JOB_DOWN;
    } else if (strcmp (key, KEY_RIGHT) == 0) {  /* Right */
        ret = MSG_JOB_RIGHT;
    } else if (strcmp (key, KEY_LEFT) == 0) {   /* Left */
        ret = MSG_JOB_LEFT;
    } else if (strcmp (key, KEY_SPACE) == 0) {  /* Space */
        ret = MSG_JOB_AHEAD;
    } else if (strcmp (key, KEY_HOLD) == 0) {   /* 'h': hold on */
        ret = MSG_JOB_HOLD;
    } else {
        ret = MSG_JOB_NOP;
    }

    return ret;
}

/*==============================================================================
 * - snake_main()
 *
 * - draw snake frame and start snake.
 */
void snake_main (int argc, char **argv)
{
    INT  iInOldOption = OPT_TERMINAL;
    INT  iOutOldOption = OPT_TERMINAL;

    ioctl(STD_IN, FIOGETOPTIONS, &iInOldOption);
    ioctl(STD_IN, FIOSETOPTIONS, OPT_RAW);

    ioctl(STD_OUT, FIOGETOPTIONS, &iOutOldOption);
    ioctl(STD_OUT, FIOSETOPTIONS, OPT_RAW);

    /* clear screen and hide cursor */
    serial_printf (CLEAR_SCREEN);
    serial_printf (HIDE_CURSOR);

    /* draw frame */
    _draw_frame ();

    //_print_rank();

    /* start snake */
    snake_run (NODE_LR_NUM, NODE_TB_NUM, _draw_node, _draw_msg, _input_msg);

    //_write_rank();

    /* clear screen, show cursor and set cmd mode */
    serial_printf (COLOR_FG_WHITE);
    serial_printf (COLOR_BG_BLACK);
    serial_printf (CLEAR_SCREEN);
    serial_printf (SHOW_CURSOR);

    ioctl(STD_IN, FIOSETOPTIONS, iInOldOption);
    ioctl(STD_OUT, FIOSETOPTIONS, iOutOldOption);
}

/*==============================================================================
** FILE END
==============================================================================*/

