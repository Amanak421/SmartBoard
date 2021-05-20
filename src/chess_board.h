#ifndef __CHESS_BOARD_H__
#define __CHESS_BOARD_H__

#include <Arduino.h>

class ChessBoard {
private:

    bool reverse = false;

    int on_move = 0;
    int player_color = 0;

    bool b_cast_k = true;
    bool b_cast_q = true;

    bool w_cast_k = true;
    bool w_cast_q = true;

    

    String last_move = "";

    char last_fen_board[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
    };

    int letterToInt(String _lett);
    void updateNumBoard();

    void stopTask();
    void startTask();


public:

    ChessBoard();

    int last_from = 0;
    int last_to = 0;

    int num_board[8][8] = {
        {2, 3, 4, 5, 6, 4, 3, 2},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {2, 3, 4, 5, 6, 4, 3, 2}
    };

    char fen_board[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
    };

    String encodeFEN();
    void decodeFEN(String _fen);

    void setReverse(bool _state);
    void setPlayerColor(int _color);

    void getNextMove(String _fen);

    void doMove(int _from, int _to);
    void doMove(int _from, int _to, String _spec);

    String getLastMove();

    void makeMoveFromServer(String _move);

    

};


#endif