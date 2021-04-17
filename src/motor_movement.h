#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <Arduino.h>
#include "Vector.h"


//definice pinů endstopů
#define X_MAX 32
#define Y_MAX 33
#define X_MIN 34
#define Y_MIN 35

//definice pinu elektromagnetu
#define ELL_PIN 13

//konstanta pro kalibraci motorů
#define ACC_MOTOR_MOVE 1



class MotorMovement{

private:

    #define MOTOR_X 0x00
    #define MOTOR_Y 0x01

    #define ON 0x02
    #define OFF 0x03

    const int STEPS_PER_MILIMETER = 5;
    const double ANGLE_PER_STEP = 1.8;
    const int ANGLE_PER_MILIMETER = 9;

    // rozměry políčka šachovnice
    int cell_x = 50;
    int cell_y = 50;

    //rozměry políčka pro odklad figurek
    int rest_cell_x = 70;
    int rest_cell_y = 50;

    // směr rotace při pozitivní hodnotě
    const int X_POSITIVE = 0;
    const int Y_POSITIVE = 1;
    const int X_NEGATIVE = 1;
    const int Y_NEGATIVE = 0;

    //výchozí pozice pro motor
    const int HOME_POSITION = 01;

    // pocet policek pro posun
    int move_row = 0;
    int move_column = 0;

    //flag pro braní mimochodem
    bool enPassantFlag = false;

    //kdo je na tahu, 0 - bílá, 1 - černá
    int actual_turn = 0;


    //pozice motoru 0 - roh / 1 - střed políčka
    bool motor_cc_position = false;

    //matice pro ulozeni pozice figurek na sachvnici
    int board[8][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0}
    };

    //struktura pro políčko (x, y)
    struct cellPos{
        int x;
        int y;
    };

    //slouží pro výběr, který motor se má pohybovat
    enum motorPick{
        X_MOTOR,
        Y_MOTOR,
        X_Y_MOTOR
    };

    //speciální příkazy pro pohyb motoru
    enum command{
        LIN_MOVE,
        DIA_MOVE,
        CENTER_MOVE,
        PIECE_OUT,
        RET_PIECE_OUT,
        BOTH
    };

    //různé možnosti posunu figurky
    enum pieceMoves{
        STRAIGHT,
        DIAGONAL,
        HORSE
    };

    //struktura pro definici pohybu motorů
    struct moveDec{
        motorPick motor;
        int cells;
        int dir;
        command special_command;
    };

    //struktura políčeka pro odkládání figurek
    struct pieceOutCell{
        int x = 0;
        int y = 0;
        bool isFull = false;
        bool isPostFull[4] = {false, false, false, false};
    };

    //pole pro políčka vyhozených figurek
    pieceOutCell pieceOutCells[8];
    int sel_piece_post = -1;    // vybraná pozice pro vyřazenou figurku; default = -1 - není aktivní
    int piece_out_cor = -1;

    //proměnná k ukládání cety koně - pokud se zpracovává cesta pro koně; 0 - provedení v řádku/sloupci počátečního políčka; 1 - provedení v řádku/sloupci o 1 vyšší/nižší; -1 - výchozí
    bool en_horse_move = false;
    int horse_move = -1;

    //aktualni pozice
    cellPos act_position = {0, 0};  //mm
    cellPos act_cell_pos = {1, 0};  //políčka

    //deklarace storage pro vector moves
    moveDec move_storage[20];

    //deklarace vectoru moves
    Vector<moveDec> moves;
    
    void addHorseMove(int _moveCellColumn, int _moveCellRow, motorPick _motor1, motorPick _motor2, int _a_positive, int _b_positive, int _a_negative, int _b_negative);
    void pieceMoveAdder(int _move_row, int _move_column, command _command);  //slouží jako pomocná fce, přidává pohyby motorů do vektoru

    

public:

    MotorMovement();    //konstructor

    void computeCellMovement(int _startCell, int _endCell); //spočítá cestu z levého horního rohu políčka na levý horní roh políčka

    void computeHomeToCellMovement(cellPos _startCell, cellPos _endCell); //spočítá cestu z levého horního rohu políčka na levý horní roh políčka
    cellPos decodePos(int _value);  //dekéduje id políček -> rozloží je na řádky a sloupce
    
    command selectMoveType(cellPos _start, cellPos _end);
    bool checkPath(cellPos _start, cellPos _end, pieceMoves _move); //vrátí true pokud je cesta pro figurku prázdná; fce spoléhá na prázné koncové políčko

    void setBoard(int _board[8][8]);    //nastaví na šachovnici aktuální rozpoložení figurek
    void printBoard();  //vytiskne šachovnici
    int pieceValue(cellPos _positoin);  //vrátí hodnotu figurky na určitém políčko

    void clearMoves(); //vyčistí pole pohybů
    void addMove(moveDec _move);  //přídá pohyb(definovaný pomocí moveDec)
    void printMoves();  //vypíše všechny pohyby do Serial Monitor

    void setUp();   //zkalibruje motory a dojede na výchozí pozici výchozí pozici
    void doMove(int _startCell, int _endCell);  //nejprve vypočítá a poté provede pohyb

    bool isPieceOutActive(cellPos _cell);   //zkontroluje zda není potřeba vyhodit figurku
    void addPieceOutMove(cellPos _cell, int _pieceColor);

    /* SPECIÁLNÍ POHYBY */ 
    void addCastlingMove(cellPos _start, cellPos _end);
    void addEnPassantMove(cellPos _start, cellPos _end, int _pieceOutColor);

    double calculateAngle(int _milimeters); //vypočítá úhel posunu motorů

    void returnToHome();

    void moveToEndstop(int _XY, int _direction);

    void setMagnetState(int _state);

    void moveCorToCen(int _dir);    //pohyb z rohu do středu a naopak -> 1 - do středu, -1 - do rohu
    void moveMotorEStop(double _angle, int _dir, int _motor);   //rotovat s motory a zaragovat na případný endstop
    void doMotorMove(); // vykoná kroky z vektoru pohybů
    void doDiagonalMove(double _angle_x, double _angle_y, int _dir_x, int _dir_y);  //provede diagonální pohyb
    void doPieceOutPos();

    void doMoveFromServer(String _move);
    void doMoveWithoutMotors(int _from, int _to);

    String last_special_move = "none";

};

#endif