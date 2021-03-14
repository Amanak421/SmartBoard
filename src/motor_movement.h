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

    // směr rotace při pozitivní hodnotě
    const int X_POSITIVE = 1;
    const int Y_POSITIVE = 1;
    const int X_NEGATIVE = 0;
    const int Y_NEGATIVE = 0;

    // pocet policek pro posun
    int move_row = 0;
    int move_column = 0;

    //aktualni pozice
    int act_position = 0;

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

    //struktura pro definici pohybu motorů
    struct moveDec{
        motorPick motor;
        int cells;
        int dir;
    };

    //deklarace storage pro vector moves
    moveDec storage[10];

    //deklarace vectoru moves
    Vector<moveDec> moves;
    

public:

    MotorMovement();    //constructor

    void computeCellMovement(int _startCell, int _endCell); //spočítá cestu z levého horního rohu políčka na levý horní roh políčka
    cellPos decodePos(int _value);  //dekéduje id políček -> rozloží je na řádky a sloupce
    
    void clearMoves(); //vyčistí pole pohybů
    void addMove(moveDec _move);  //přídá pohyb(definovaný pomocí moveDec)

    void printMoves();  //vypíše všechny pohyby do Serial Monitor


};



#endif