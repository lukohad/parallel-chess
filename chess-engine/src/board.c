#include "board.h"

#include <stdio.h>
#include <string.h>

//piece symbols indexed by the Piece enum, uppercase is white, lowercase is black, '.' is empty
static const char SYMBOLS[] = ".PNBRQKpnbrqk";

//initialize the board to the starting position
void board_init(Board *board)
{
    *board = (Board) {
        .squares = {
            WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK,
            WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN,
            EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
            EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
            EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
            EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
            BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN,
            BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK
        },
        .side_to_move = WHITE,
        .castling_rights = CASTLE_WHITE_KINGSIDE | CASTLE_WHITE_QUEENSIDE |
                           CASTLE_BLACK_KINGSIDE | CASTLE_BLACK_QUEENSIDE,
        .en_passant = -1,
        .halfmove_clock = 0,
        .fullmove_number = 1
    };
}

void board_print(const Board *board)
{
    //print the board from rank 8 to rank 1
    for (int rank = 7; rank >= 0; --rank) {
        printf("%d  ", rank + 1);
        //print each file in the rank
        for (int file = 0; file < 8; ++file) {
            printf("%c ", SYMBOLS[board->squares[rank * 8 + file]]);
        }
        putchar('\n');
    }
    //print the file labels and side to move
    printf("\n   a b c d e f g h\n");
    //print the side to move
    printf("Side to move: %s\n", board->side_to_move == WHITE ? "White" : "Black");
}

// load a position from FEN notation, false if the string is malformed
bool board_from_fen(Board *board, const char *fen)
{
    *board = (Board){.en_passant = -1, .fullmove_number = 1};
    int sq = 56; // FEN lists rank 8 first, starting at a8
    for (; *fen != '\0' && *fen != ' '; ++fen) {
        const char *symbol = strchr(SYMBOLS + 1, *fen);
        if (*fen == '/') sq -= 16;
        else if (*fen >= '1' && *fen <= '8') sq += *fen - '0';
        else if (symbol != NULL && sq >= 0 && sq < 64) board->squares[sq++] = symbol - SYMBOLS;
        else return false;
    }
    if (sq != 8) return false; // a complete board ends just past h1

    // remaining fields are side to move, castling rights, en passant square and the two clocks, like "w KQkq e3 0 1"
    char side, castling[5], ep[3];
    int used = 0;
    if (sscanf(fen, " %c %4s %2s%n", &side, castling, ep, &used) != 3) return false;
    // the clocks are optional in FEN, if absent the defaults (0 and 1) stay
    sscanf(fen + used, "%d %d", &board->halfmove_clock, &board->fullmove_number);
    board->side_to_move = side == 'b' ? BLACK : WHITE;
    if (strchr(castling, 'K')) board->castling_rights |= CASTLE_WHITE_KINGSIDE;
    if (strchr(castling, 'Q')) board->castling_rights |= CASTLE_WHITE_QUEENSIDE;
    if (strchr(castling, 'k')) board->castling_rights |= CASTLE_BLACK_KINGSIDE;
    if (strchr(castling, 'q')) board->castling_rights |= CASTLE_BLACK_QUEENSIDE;
    if (ep[0] != '-') board->en_passant = (ep[1] - '1') * 8 + (ep[0] - 'a');
    return true;
}
