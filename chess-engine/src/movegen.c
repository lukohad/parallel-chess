#include "movegen.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// {file, rank} steps
static const int ROOK_DIRS[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
static const int BISHOP_DIRS[4][2] = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
static const int ALL_DIRS[8][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
static const int KNIGHT_DIRS[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};

// color of a non-empty piece
static Color color_of(Piece p) { return p >= BLACK_PAWN ? BLACK : WHITE; }

// the other side
static Color opponent(Color c) { return c == WHITE ? BLACK : WHITE; }

// white version of a piece, so WHITE_* values double as piece types
static Piece type_of(Piece p) { return p >= BLACK_PAWN ? p - 6 : p; }

// the piece of this type in color c
static Piece colored(Piece type, Color c) { return c == BLACK ? type + 6 : type; }

// true if nothing is on sq
static bool is_empty(const Board *b, int sq) { return b->squares[sq] == EMPTY; }

// true if sq holds a piece of the side not to move
static bool is_enemy(const Board *b, int sq) { return !is_empty(b, sq) && color_of(b->squares[sq]) != b->side_to_move; }

// true if sq is on the board and holds piece p
static bool holds(const Board *b, int sq, Piece p) { return sq >= 0 && b->squares[sq] == p; }

// square reached by moving (df, dr) from sq, or -1 if it falls off the board
static int step_by(int sq, int df, int dr)
{
    int file = sq % 8 + df, rank = sq / 8 + dr;
    return file >= 0 && file < 8 && rank >= 0 && rank < 8 ? rank * 8 + file : -1;
}

// step_by with the offset given as a {file, rank} pair
static int step(int sq, const int dir[2]) { return step_by(sq, dir[0], dir[1]); }

// first piece met walking from sq along dir, or EMPTY if the ray leaves the board
static Piece first_piece(const Board *b, int sq, const int dir[2])
{
    int t = step(sq, dir);
    while (t >= 0 && is_empty(b, t)) t = step(t, dir);
    return t >= 0 ? b->squares[t] : EMPTY;
}

// true if any piece of color by attacks sq
static bool is_attacked(const Board *b, int sq, Color by)
{
    for (int i = 0; i < 8; ++i) {
        if (holds(b, step(sq, KNIGHT_DIRS[i]), colored(WHITE_KNIGHT, by))) return true;
        if (holds(b, step(sq, ALL_DIRS[i]), colored(WHITE_KING, by))) return true;
    }
    Piece queen = colored(WHITE_QUEEN, by);
    for (int i = 0; i < 4; ++i) {
        Piece hit = first_piece(b, sq, ROOK_DIRS[i]);
        if (hit == queen || hit == colored(WHITE_ROOK, by)) return true;
        hit = first_piece(b, sq, BISHOP_DIRS[i]);
        if (hit == queen || hit == colored(WHITE_BISHOP, by)) return true;
    }
    // an attacking pawn sits one rank behind sq from its own side's view
    int behind = by == WHITE ? -1 : 1;
    Piece pawn = colored(WHITE_PAWN, by);
    return holds(b, step_by(sq, -1, behind), pawn) || holds(b, step_by(sq, 1, behind), pawn);
}

// square of the king of color c
static int king_square(const Board *b, Color c)
{
    for (int sq = 0; sq < 64; ++sq)
        if (b->squares[sq] == colored(WHITE_KING, c)) return sq;
    fputs("king_square: position has no king\n", stderr);
    abort();
}

// append a move to the list
static void add(MoveList *list, int from, int to, Piece promotion, uint8_t flags)
{
    list->moves[list->count++] = (Move){from, to, promotion, flags};
}

// append a pawn move, as four promotions if it reaches the last rank
static void add_pawn_move(const Board *b, MoveList *list, int from, int to, uint8_t flags)
{
    if (to / 8 != 0 && to / 8 != 7) {
        add(list, from, to, EMPTY, flags);
        return;
    }
    for (Piece t = WHITE_KNIGHT; t <= WHITE_QUEEN; ++t)
        add(list, from, to, colored(t, b->side_to_move), flags | MOVE_PROMOTION);
}

// pushes, double pushes, captures and en passant for the pawn on from
static void pawn_moves(const Board *b, MoveList *list, int from)
{
    int forward = b->side_to_move == WHITE ? 1 : -1;
    int one = from + 8 * forward, two = from + 16 * forward;
    bool on_start_rank = from / 8 == (b->side_to_move == WHITE ? 1 : 6);
    if (is_empty(b, one)) {
        add_pawn_move(b, list, from, one, MOVE_NONE);
        if (on_start_rank && is_empty(b, two)) add(list, from, two, EMPTY, MOVE_DOUBLE_PAWN);
    }
    for (int df = -1; df <= 1; df += 2) {
        int target = step_by(from, df, forward);
        if (target < 0) continue;
        if (target == b->en_passant) add(list, from, target, EMPTY, MOVE_CAPTURE | MOVE_EN_PASSANT);
        else if (is_enemy(b, target)) add_pawn_move(b, list, from, target, MOVE_CAPTURE);
    }
}

// moves along each direction, one step for knights and kings, until blocked for sliders
static void piece_moves(const Board *b, MoveList *list, int from, const int dirs[][2], int count, bool slides)
{
    for (int i = 0; i < count; ++i) {
        for (int to = step(from, dirs[i]); to >= 0; to = step(to, dirs[i])) {
            Piece victim = b->squares[to];
            if (victim != EMPTY && color_of(victim) == b->side_to_move) break;
            add(list, from, to, EMPTY, victim == EMPTY ? MOVE_NONE : MOVE_CAPTURE);
            if (victim != EMPTY || !slides) break;
        }
    }
}

// castling moves, the king may not start in or pass through check (landing in check is caught later)
static void castle_moves(const Board *b, MoveList *list)
{
    Color us = b->side_to_move, them = opponent(us);
    int king = us == WHITE ? 4 : 60;
    uint8_t kingside = us == WHITE ? CASTLE_WHITE_KINGSIDE : CASTLE_BLACK_KINGSIDE;
    uint8_t queenside = us == WHITE ? CASTLE_WHITE_QUEENSIDE : CASTLE_BLACK_QUEENSIDE;
    if (!(b->castling_rights & (kingside | queenside)) || is_attacked(b, king, them)) return;
    bool kingside_clear = is_empty(b, king + 1) && is_empty(b, king + 2);
    bool queenside_clear = is_empty(b, king - 1) && is_empty(b, king - 2) && is_empty(b, king - 3);
    if ((b->castling_rights & kingside) && kingside_clear && !is_attacked(b, king + 1, them))
        add(list, king, king + 2, EMPTY, MOVE_CASTLING);
    if ((b->castling_rights & queenside) && queenside_clear && !is_attacked(b, king - 1, them))
        add(list, king, king - 2, EMPTY, MOVE_CASTLING);
}

// all legal moves for the side to move
void generate_legal_moves(const Board *board, MoveList *list)
{
    list->count = 0;
    for (int from = 0; from < 64; ++from) {
        Piece p = board->squares[from];
        if (p == EMPTY || color_of(p) != board->side_to_move) continue;
        switch (type_of(p)) {
        case WHITE_PAWN:   pawn_moves(board, list, from); break;
        case WHITE_KNIGHT: piece_moves(board, list, from, KNIGHT_DIRS, 8, false); break;
        case WHITE_BISHOP: piece_moves(board, list, from, BISHOP_DIRS, 4, true); break;
        case WHITE_ROOK:   piece_moves(board, list, from, ROOK_DIRS, 4, true); break;
        case WHITE_QUEEN:  piece_moves(board, list, from, ALL_DIRS, 8, true); break;
        default: // king
            piece_moves(board, list, from, ALL_DIRS, 8, false);
            castle_moves(board, list);
            break;
        }
    }
    // keep only moves that do not leave our own king attacked
    int legal = 0;
    for (int i = 0; i < list->count; ++i) {
        Board next;
        make_move(board, list->moves[i], &next);
        if (!is_attacked(&next, king_square(&next, board->side_to_move), next.side_to_move))
            list->moves[legal++] = list->moves[i];
    }
    list->count = legal;
}

// play move into next, leaving board unchanged
void make_move(const Board *board, Move move, Board *next)
{
    *next = *board;
    Piece piece = board->squares[move.from];
    next->squares[move.to] = move.promotion != EMPTY ? move.promotion : piece;
    next->squares[move.from] = EMPTY;
    if (move.flags & MOVE_EN_PASSANT) // the captured pawn is beside the mover, not on the target square
        next->squares[move.to + (board->side_to_move == WHITE ? -8 : 8)] = EMPTY;
    if (move.flags & MOVE_CASTLING) { // rook lands on the square the king passed over
        int rook_from = move.to > move.from ? move.from + 3 : move.from - 4;
        next->squares[(move.from + move.to) / 2] = board->squares[rook_from];
        next->squares[rook_from] = EMPTY;
    }
    // a castling right is gone for good once its king or rook is off its home square
    if (next->squares[4] != WHITE_KING) next->castling_rights &= ~(CASTLE_WHITE_KINGSIDE | CASTLE_WHITE_QUEENSIDE);
    if (next->squares[0] != WHITE_ROOK) next->castling_rights &= ~CASTLE_WHITE_QUEENSIDE;
    if (next->squares[7] != WHITE_ROOK) next->castling_rights &= ~CASTLE_WHITE_KINGSIDE;
    if (next->squares[60] != BLACK_KING) next->castling_rights &= ~(CASTLE_BLACK_KINGSIDE | CASTLE_BLACK_QUEENSIDE);
    if (next->squares[56] != BLACK_ROOK) next->castling_rights &= ~CASTLE_BLACK_QUEENSIDE;
    if (next->squares[63] != BLACK_ROOK) next->castling_rights &= ~CASTLE_BLACK_KINGSIDE;
    next->en_passant = move.flags & MOVE_DOUBLE_PAWN ? (move.from + move.to) / 2 : -1;
    bool resets_clock = type_of(piece) == WHITE_PAWN || (move.flags & MOVE_CAPTURE);
    next->halfmove_clock = resets_clock ? 0 : board->halfmove_clock + 1;
    if (board->side_to_move == BLACK) next->fullmove_number++;
    next->side_to_move = opponent(board->side_to_move);
}
