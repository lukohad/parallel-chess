#include "board.h"

int main(void)
{
    Board board;
    board_init(&board);
    board_print(&board);
    return 0;
}
