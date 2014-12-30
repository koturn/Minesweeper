#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <getopt.h>
#include <termutil.h>
#if defined(_MSC_VER) && defined(_DEBUG)
#  include <msvcdbg.h>
#endif


#define BUF_SIZE  256
#define TRUE  1
#define FALSE 0
#define N_ALPHABET    26
#define MAX_ROW_SIZE  52
#define MAX_COL_SIZE  52
#define MIN_ROW_SIZE  4
#define MIN_COL_SIZE  4
#define DEFAULT_ROW   9
#define DEFAULT_COL   9
#define DEFAULT_N_MINE  10

#define Y_OFFSET  3
#define X_OFFSET  2

#define CTRL_C  0x03

#define UNCHECKED_MASK  ((unsigned char) 0x80)
#define FLAG_MASK       ((unsigned char) 0x40)
#define MINE            ((unsigned char) 0x3f)

#define SWAP(type, a, b) \
  do { \
    type __tmp_swap_var__ = *(a); \
    *(a) = *(b); \
    *(b) = __tmp_swap_var__; \
  } while (0)

enum {
  EASY,
  NORMAL,
  HARD
} Level;

typedef enum {
  CMD_NONE,
  CMD_OPEN,
  CMD_FLAG
} Command;

typedef enum {
  CURSOR,
  PROMPT
} Mode;

typedef struct {
  int n_row;
  int n_col;
  int n_mine;
  Mode mode;
} Param;

typedef struct {
  unsigned char *data;
  int n_row;
  int n_col;
  int _n_col;
  int n_mine;
} Board;


static void
parse_arg(Param *param, int argc, char *argv[]);

static void
show_usage(const char *progname);

static void
init_board(Board *board);

static void
run_interactive(Board *board);

static void
run_cursor_mode(Board *board);

static void
init_term(void);

static void
print_board(const Board *board);

static void
print_col_label(int n_col);

static void
print_separator(int n_col);

static int
read_coodinate(int *y, int *x, const char *str);

static void
open_panel(Board *board, int y, int x);

static void
flag_panel(Board *board, int y, int x);

static int
check_sweeped(const Board *board);


/*!
 * @brief Entry point of this program
 * @param [in] argc  The number of given command-line arguments
 * @param [in] argv  The command line argument vector
 * @return Exit-status
 */
int
main(int argc, char *argv[])
{
  Param param = {DEFAULT_ROW, DEFAULT_COL, DEFAULT_N_MINE, CURSOR};
  Board board;

  parse_arg(&param, argc, argv);
  board.n_row = param.n_row;
  board.n_col = param.n_col;
  board._n_col = param.n_col + 2;
  board.n_mine = param.n_mine;
  board.data = calloc(
      (size_t) ((board.n_row + 2) * (board.n_col + 2)),
      sizeof(unsigned char));
  if (board.data == NULL) {
    fputs("Cannot allocate memory\n", stderr);
    return EXIT_FAILURE;
  }

  srand((unsigned int) time(NULL));
  printf("Start Minesweeper\n");

  if (param.mode == CURSOR) {
    run_cursor_mode(&board);
  } else if (param.mode == PROMPT) {
    run_interactive(&board);
  }

  free(board.data);
  return EXIT_SUCCESS;
}


/*!
 * @brief Parse argument
 * @param [out] board  Game board information of minesweeper
 * @param [out] argc   The number of given command-line arguments
 * @param [out] argv   The command line argument array
 */
static void
parse_arg(Param *param, int argc, char *argv[])
{
  static const struct option OPTIONS[] = {
    {"column", required_argument, NULL, 'c'},
    {"help",   no_argument,       NULL, 'h'},
    {"level",  required_argument, NULL, 'l'},
    {"mode",   required_argument, NULL, 'm'},
    {"n-mine", required_argument, NULL, 'n'},
    {"row",    required_argument, NULL, 'r'},
    {0, 0, 0, 0}  /* must be filled with zero */
  };
  int ret, optidx = 0;

  while ((ret = getopt_long(argc, argv, "c:hl:m:n:r:", OPTIONS, &optidx)) != -1) {
    switch (ret) {
      case 'c':  /* -c, --column */
        param->n_col = atoi(optarg);
        break;
      case 'h':  /* -h, --help */
        show_usage(argv[0]);
        exit(EXIT_SUCCESS);
      case 'l':  /* -l, --level */
        if (!strcmp(optarg, "easy")) {
          param->n_row = 10;
          param->n_col = 10;
          param->n_mine = 10;
        } else if (!strcmp(optarg, "normal")) {
          param->n_row = 16;
          param->n_col = 16;
          param->n_mine = 40;
        } else if (!strcmp(optarg, "hard")) {
          param->n_row = 16;
          param->n_col = 30;
          param->n_mine = 99;
        } else {
          fputs("Unknown level is specified\n", stderr);
          exit(EXIT_FAILURE);
        }
        break;
      case 'm':  /* -m, --mode */
        if (!strcmp(optarg, "cursor")) {
          param->mode = CURSOR;
        } else if (!strcmp(optarg, "prompt")) {
          param->mode = PROMPT;
        } else {
          fputs("Unknown mode is specified\n", stderr);
          exit(EXIT_FAILURE);
        }
        break;
      case 'n':  /* -n, --n-mine */
        param->n_mine = atoi(optarg);
        break;
      case 'r':  /* -r, --row */
        param->n_row = atoi(optarg);
        break;
      case '?':  /* unknown option */
        show_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (param->n_row > MAX_ROW_SIZE) {
    fprintf(stderr, "Row size must be at most %d\n", MAX_ROW_SIZE);
    exit(EXIT_FAILURE);
  } else if (param->n_row < MIN_ROW_SIZE) {
    fprintf(stderr, "Row size must be at least %d\n", MIN_ROW_SIZE);
    exit(EXIT_FAILURE);
  }
  if (param->n_col > MAX_COL_SIZE) {
    fprintf(stderr, "Column size must be at most %d\n", MAX_COL_SIZE);
    exit(EXIT_FAILURE);
  } else if (param->n_col < MIN_COL_SIZE) {
    fprintf(stderr, "Column size must be at least %d\n", MIN_COL_SIZE);
    exit(EXIT_FAILURE);
  }
  if (param->n_mine >= param->n_row * param->n_col) {
    fputs("Too many mines!\n", stderr);
    exit(EXIT_FAILURE);
  }
}


/*!
 * @brief Show usage of this program
 * @param [in] progname  Name of this program
 */
static void
show_usage(const char *progname)
{
  printf(
      "[Usage]\n"
      "  $ %s [options]\n\n", progname);
  printf(
      "[Options]\n"
      "  -c COLUMN_SIZE, --column=COLUMN_SIZE\n"
      "    Specify column size\n"
      "  -h, --help\n"
      "    Show help and exit\n"
      "  -l LEVEL, --level=LEVEL\n"
      "    Specify level of minesweeper\n"
      "    [Levels]\n"
      "      easy:\n"
      "        row size is 9,  column size is 9  and number of mine is 10\n"
      "      normal:\n"
      "        row size is 16, column size is 16 and number of mine is 40\n"
      "      hard:\n"
      "        row size is 16, column size is 30 and number of mine is 99\n");
  printf(
      "  -m MODE, --mode=MODE\n"
      "    Specify mode\n"
      "    [Modes]\n"
      "      cursor:\n"
      "        You can move cursor\n"
      "          'h': Move cursor left\n"
      "          'j': Move cursor down\n"
      "          'k': Move cursor up\n"
      "          'l': Move cursor right\n"
      "          'o': Open a panel under the cursor\n"
      "          'f': Flag a panel under the cursor\n"
      "      prompt:\n"
      "        Show prompt and you have to input command and coordinate\n"
      "        [Command]\n"
      "          open: Open a panel\n"
      "          flag: Flag a panel\n");
  printf(
      "  -n N_MINE, --n-mines=N_MINE\n"
      "    Specify the number of mines\n"
      "  -r ROW_SIZE, --row=ROW_SIZE\n"
      "    Specify row size\n");
}


/*!
 * @brief Play minesweeper interactively
 * @param [in,out] board  Game board information of minesweeper
 */
static void
run_interactive(Board *board)
{
  static char buf[BUF_SIZE];
  int x, y, is_sweeped;

  for (;;) {
    init_board(board);
    is_sweeped = FALSE;

    do {
      Command cmd = CMD_NONE;
      do {
        print_board(board);
        printf("command? [open|flag] > ");
        fgets(buf, sizeof(buf), stdin);
        if (!strcmp(buf, "open\n")) {
          cmd = CMD_OPEN;
        } else if (!strcmp(buf, "flag\n")) {
          cmd = CMD_FLAG;
        }
      } while (cmd == CMD_NONE);

      do {
        printf("where? > ");
        fgets(buf, sizeof(buf), stdin);
        if (!strcmp(buf, "quit\n")) {
          return;
        }
      } while (!read_coodinate(&y, &x, buf));

      if (cmd == CMD_OPEN) {
        open_panel(board, y, x);
      } else {
        flag_panel(board, y, x);
      }
      if (board->data[y * board->_n_col + x] == MINE) {
        break;
      }
    } while (!(is_sweeped = check_sweeped(board)));

    print_board(board);
    if (is_sweeped) {
      printf("Good-Job!!!  You've sweeped all Mines in success.\n\n");
    } else {
      printf("Oops!!! You've hit a Mine...\n\n");
    }
    printf("Try again? [Y/N]");
    fgets(buf, sizeof(buf), stdin);
    if (!strcmp(buf, "n\n") || !strcmp(buf, "N\n")) {
      break;
    }
  }
}


/*!
 * @brief Play minesweeper in cursor mode
 * @param [in,out] board  Game board information of minesweeper
 */
static void
run_cursor_mode(Board *board)
{
  int x = 1, y = 1, is_sweeped, ch;

  for (;;) {
    init_board(board);
    init_term();
    is_sweeped = FALSE;

    do {
      tu_move(0, 0);
      tu_clear();
      print_board(board);
      tu_move(y + Y_OFFSET, x + X_OFFSET);
      switch (tu_getch()) {
        case 'h':
          if (x > 1) x--;
          tu_move(y + Y_OFFSET, x + X_OFFSET);
          break;
        case 'j':
          if (y < board->n_row) y++;
          tu_move(y + Y_OFFSET, x + X_OFFSET);
          break;
        case 'k':
          if (y > 1) y--;
          tu_move(y + Y_OFFSET, x + X_OFFSET);
          break;
        case 'l':
          if (x < board->n_col) x++;
          tu_move(y + Y_OFFSET, x + X_OFFSET);
          break;
        case 'o':
          open_panel(board, y, x);
          break;
        case 'f':
          flag_panel(board, y, x);
          break;
        case CTRL_C:
          exit(EXIT_SUCCESS);
          break;
      }
      if (board->data[y * board->_n_col + x] == MINE) {
        break;
      }
    } while (!(is_sweeped = check_sweeped(board)));

    tu_move(0, 0);
    print_board(board);
    if (is_sweeped) {
      tu_addstr("Good-Job!!!  You've sweeped all Mines in success.\n\n");
    } else {
      tu_addstr("Oops!!! You've hit a Mine...\n\n");
    }
    tu_addstr("Try again? [Y/N]");
    ch = tu_getch();
    if (ch == 'n' || ch == 'N') {
      break;
    }
  }
}


/*!
 * @brief Initialize terminal
 */
static void
init_term(void)
{
  tu_init();
  tu_clear();
  tu_cbreak();
  tu_noecho();
}


/*!
 * @brief Initialize board
 * @param [out] board  Game board information of minesweeper
 */
static void
init_board(Board *board)
{
  int i, j, x, y;

  memset(board->data, 0, (size_t) ((board->n_row + 2) * (board->n_col + 2)));
  /* Set mines */
  for (i = 0; i < board->n_mine; i++) {
    board->data[i] = MINE;
  }
  /* Fishe- Yates shuffle */
  for (i = board->n_row * board->n_col - 1; i > 0; i--) {
    j = rand() % (i + 1);
    SWAP(unsigned char, &board->data[i], &board->data[j]);
  }
  /* Move */
  for (i = board->n_row * board->n_col - 1; i >= 0; i--) {
    int offset = board->n_col + 3 + 2 * (i / board->n_col);
    SWAP(unsigned char, &board->data[i + offset], &board->data[i]);
  }
  /* Set labels which indicate number of bombs */
  for (y = 1; y <= board->n_row; y++) {
    for (x = 1; x <= board->n_col; x++) {
      if (board->data[y * board->_n_col + x] == MINE) {
        if (board->data[(y - 1) * board->_n_col + (x - 1)] != MINE)
          board->data[(y - 1) * board->_n_col + (x - 1)]++;
        if (board->data[(y - 1) * board->_n_col + x] != MINE)
          board->data[(y - 1) * board->_n_col + x]++;
        if (board->data[(y - 1) * board->_n_col + (x + 1)] != MINE)
          board->data[(y - 1) * board->_n_col + (x + 1)]++;

        if (board->data[y * board->_n_col + (x - 1)] != MINE)
          board->data[y * board->_n_col + (x - 1)]++;
        if (board->data[y * board->_n_col + (x + 1)] != MINE)
          board->data[y * board->_n_col + (x + 1)]++;

        if (board->data[(y + 1) * board->_n_col + (x - 1)] != MINE)
          board->data[(y + 1) * board->_n_col + (x - 1)]++;
        if (board->data[(y + 1) * board->_n_col + x] != MINE)
          board->data[(y + 1) * board->_n_col + x]++;
        if (board->data[(y + 1) * board->_n_col + (x + 1)] != MINE)
          board->data[(y + 1) * board->_n_col + (x + 1)]++;
      }
    }
  }
  /* Hide panels */
  for (y = 0; y < board->n_row + 2; y++) {
    for (x = 0; x < board->n_col + 2; x++) {
      board->data[y * board->_n_col + x] |= UNCHECKED_MASK;
    }
  }
}


/*!
 * @brief Print board
 * @param [in] board  Game board information of minesweeper
 */
static void
print_board(const Board *board)
{
  int x, y;

  tu_addstr("\n\n");
  print_col_label(board->n_col);
  print_separator(board->n_col);
  for (y = 1; y <= board->n_row; y++) {
    tu_printw("%2d|", y);
    for (x = 1; x <= board->n_col; x++) {
      if (board->data[y * board->_n_col + x] & FLAG_MASK) {
        tu_addch('*');
      } else if (board->data[y * board->_n_col + x] & UNCHECKED_MASK) {
        tu_addch('o');
      } else {
        switch (board->data[y * board->_n_col + x]) {
          case 0:
            tu_addch('.');
            break;
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
          case 8:
            tu_printw("%d", (int) board->data[y * board->_n_col + x]);
            break;
          case MINE:
            tu_addch('@');
            break;
        }
      }
    }
    tu_addch('\n');
  }
  tu_addch('\n');
}


/*!
 * @brief Print column label
 * @param [in] n_col  The number of columns
 */
static void
print_col_label(int n_col)
{
  int i, last;

  tu_addstr("  |");
  if (n_col < N_ALPHABET) {
    for (i = 0; i < n_col; i++) {
      tu_addch('a' + i);
    }
  } else {
    for (i = 0; i < N_ALPHABET; i++) {
      tu_addch('a' + i);
    }
    for (i = 0, last = n_col - N_ALPHABET; i < last; i++) {
      tu_addch('A' + i);
    }
  }
  tu_addch('\n');
}


/*!
 * @brief Print column label separator
 * @param [in] n_col  The number of columns
 */
static void
print_separator(int n_col)
{
  int i;

  tu_addstr("--+");
  for (i = 0; i < n_col; i++) {
    tu_addch('-');
  }
  tu_addch('\n');
}


/*!
 * @brief Read coordinate from inputted string
 * @param [out] y    Pointer to variable which store the Y-coordinate
 * @param [out] x    Pointer to variable which store the X-coordinate
 * @param [in]  str  Inputted coordinate string
 * @return  Return 0 if invalid string is inputted, otherwise return 0
 */
static int
read_coodinate(int *y, int *x, const char *str)
{
  size_t len = strlen(str);

  if (len < 3) return FALSE;
  if (islower(str[0])) {
    *x = str[0] - 'a' + 1;
  } else if (isupper(str[0])) {
    *x = str[0] - 'A' + N_ALPHABET + 1;
  } else {
    return FALSE;
  }
  if (len == 3 && isdigit(str[1])) {
    *y = str[1] - '0';
  } else if (len == 4 && isdigit(str[1]) && isdigit(str[2])) {
    *y = (str[1] - '0') * 10 + (str[2] - '0');
  } else {
    return FALSE;
  }
  return TRUE;
}


/*!
 * @brief Open specified panel
 *
 * If there is no bombs around specified panel, open eight neighborhood panels
 * @param [out] board  Game board information of minesweeper
 * @param [in]  y      Y-coordinate
 * @param [in]  x      X-coordinate
 */
static void
open_panel(Board *board, int y, int x)
{
  int offset = y * board->_n_col + x;

  if (y < 1 || y > board->n_row || x < 1 || x > board->n_col) {
    return;
  }
  /* Already opened */
  if (!(board->data[offset] & UNCHECKED_MASK)) {
    return;
  }
  /* Already flagged */
  if (board->data[offset] & FLAG_MASK) {
    return;
  }
  board->data[offset] &= (unsigned char) (~UNCHECKED_MASK);
  if (board->data[offset] == 0) {
    open_panel(board, y - 1, x - 1);
    open_panel(board, y - 1, x);
    open_panel(board, y - 1, x + 1);
    open_panel(board, y, x - 1);
    open_panel(board, y, x + 1);
    open_panel(board, y + 1, x - 1);
    open_panel(board, y + 1, x);
    open_panel(board, y + 1, x + 1);
  }
}


/*!
 * @brief Toggle flag on specified panel
 * @param [out] board  Game board information of minesweeper
 * @param [in]  y      Y-coordinate
 * @param [in]  x      X-coordinate
 */
static void
flag_panel(Board *board, int y, int x)
{
  int offset = y * board->_n_col + x;

  if (y < 1 || y > board->n_row || x < 1 || x > board->n_col) {
    return;
  }
  /* Already opened */
  if (!(board->data[offset] & UNCHECKED_MASK)) {
    return;
  }
  board->data[offset] ^= FLAG_MASK;
}


/*!
 * @brief Check whether all non-bomb panel is opened or not.
 * @param [in] board  Game board information of minesweeper
 * @return If all non-bomb panel is opened, return 1, otherwise return 0.
 */
static int
check_sweeped(const Board *board)
{
  int x, y;

  for (y = 1; y <= board->n_row; y++) {
    for (x = 1; x <= board->n_col; x++) {
      /* Specified panel is not opened though the panel is not bomb */
      if ((board->data[y * board->_n_col + x] & ~(UNCHECKED_MASK | FLAG_MASK)) != MINE &&
          (board->data[y * board->_n_col + x] & (UNCHECKED_MASK | FLAG_MASK))) {
        return FALSE;
      }
    }
  }
  return TRUE;
}
