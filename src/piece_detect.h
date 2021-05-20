#ifndef __PIECE_DETECT_H__
#define __PIECE_DETECT_H__

#include "Arduino.h"

#include <Wire.h>
#include <PCF8574.h>


class PieceDetect {

private:

    #define DATA_IN 4  //data z MUX
    const int DATA_PINS[8] = {P0, P1, P2, P3, P4, P5, P6, P7};
    const int ROW_PINS[8] = {15, 2, 0, 4, 16, 17, 18, 19};

    #define READ_DELAY 150  //pauza mezi čtením šachovnice

    const int ROWS = 3;     //velikost šachovnice sloupky * řádky
    const int COLUMNS = 8;

    const int ROW_ID[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

    const int DEC_PINS[3] = {15, 2, 0};   //připojení 3-bitového dekoderu
    const int MUX_PINS[3] = {16, 18, 19};   //připojení MUX

    int last_board[8][8];       //pole pro uložení poslední hodnoty šachovnice v 0 a 1

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

    int start_board[8][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0}
    };

    int temp_board[8][8];

    bool move_completed = false;
    bool move_procc = false;

    int on_move = 0;
    int player_color = 0;

    bool w_cast_k = true;
    bool w_cast_q = true;
    bool b_cast_k = true;
    bool b_cast_q = true;
    bool cast_k_flag = false;
    bool cast_q_flag = false;
    int cast_from = -1;
    int cast_to = -1;

    const int confirm_scans = 3;
    int act_confirm_scan = 0;

    bool reverse = false;   //slouží k obrácení šachovnice při updatování šachovnice

    int last_piece_out = -1;

    int readMUX(int channel);       //přečte hodnotu z určitého kanálu MUX
    void selectRow(int _id);    //nastaví, který řádek chceme číst

    bool checkBoard();      //pokud se právě přečtená šachovnice shoduje s poslední šachovnicí vrátí true
    void findLastMove();
    int readValue(int _column);
    void pickRow(int _row);

    void scanBoard();
    int checkDiff();

    bool isSame();


public:

    PieceDetect();

    int last_from = -1;     //uložení posledního provedeného pohybu
    int last_to = -1;

    void printPieceBoard();
    bool checkMove();

    bool moveCompleted();
    void finishMove();

    void updateBoard(String _fen);

    bool startPoss();

    bool backTurn();

    void makeMoveFromServer(String _move);

    void setReverse(bool _rev);
    void setPlayerColor(int _color);

};


#endif