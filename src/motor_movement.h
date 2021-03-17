#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <Arduino.h>
#include "Vector.h"

#include "A4988.h"


class MotorMovement{

private:

    // rozměry políčka šachovnice
    int cell_x = 40;
    int cell_y = 40;

    //rozměry políčka pro odklad figurek
    int rest_cell_x = 70;
    int rest_cell_y = 40;

    // směr rotace při pozitivní hodnotě
    const int X_POSITIVE = 1;
    const int Y_POSITIVE = 1;
    const int X_NEGATIVE = 0;
    const int Y_NEGATIVE = 0;

    //výchozí pozice pro motor
    const int HOME_POSITION = 01;

    // pocet policek pro posun
    int move_row = 0;
    int move_column = 0;

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
        PIECE_OUT
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

    //proměnná k ukládání cety koně - pokud se zpracovává cesta pro koně; 0 - provedení v řádku/sloupci počátečního políčka; 1 - provedení v řádku/sloupci o 1 vyšší/nižší; -1 - výchozí
    bool en_horse_move = false;
    int horse_move = -1;

    //aktualni pozice
    cellPos act_position = {0, 0};  //mm
    cellPos act_cell_pos = {1, 0};  //políčka

    //deklarace storage pro vector moves
    moveDec storage[10];

    //deklarace vectoru moves
    Vector<moveDec> moves;
    
    void addHorseMove(int _moveCellColumn, int _moveCellRow, motorPick _motor1, motorPick _motor2, int _a_positive, int _b_positive, int _a_negative, int _b_negative);

public:

    MotorMovement();    //konstructor

    void computeCellMovement(int _startCell, int _endCell); //spočítá cestu z levého horního rohu políčka na levý horní roh políčka
    void computeCellMovement(cellPos _startCell, int _endCell); //spočítá cestu z levého horního rohu políčka na levý horní roh políčka
    cellPos decodePos(int _value);  //dekéduje id políček -> rozloží je na řádky a sloupce
    
    command selectMoveType(cellPos _start, cellPos _end);
    bool checkPath(cellPos _start, cellPos _end, pieceMoves _move); //vrátí true pokud je cesta pro figurku prázdná; fce spoléhá na prázné koncové políčko

    void setBoard(int _board[8][8]);    //nastaví na šachovnici aktuální rozpoložení figurek
    void printBoard();  //vytiskne šachovnici
    int pieceValue(cellPos _positoin);  //vrátí hodnotu figurky na určitém políčko

    void clearMoves(); //vyčistí pole pohybů
    void addMove(moveDec _move);  //přídá pohyb(definovaný pomocí moveDec)
    void printMoves();  //vypíše všechny pohyby do Serial Monitor

    void returnToHome();    //vrátí se na políčko 00
    void setUp();   //zkalibruje motory a dojede na výchozí pozici výchozí pozici
    void doMove(int _startCell, int _endCell);  //nejprve vypočítá a poté provede pohyb

    cellPos test1 = {4, 4};
    cellPos test2 = {6, 3};
    pieceMoves test_move = HORSE;

};

#endif