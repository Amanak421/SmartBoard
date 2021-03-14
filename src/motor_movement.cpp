#include "motor_movement.h"

MotorMovement::MotorMovement(){
    //přiřazení storage pro vector array
    moves.setStorage(storage);
}

void MotorMovement::computeCellMovement(int _startCell, int _endCell){

    // vytvoření počátečního a koncového políčka a rozložení hodnot na x-sloupce a y-řádky
    cellPos startCell = decodePos(_startCell);
    cellPos endCell = decodePos(_endCell);

    //výpočet početu políček, o které je nutné se posunout
    move_column = startCell.x - endCell.x;
    move_row = startCell.y - endCell.y;

    #ifdef DEBUG    // debug
    Serial.print("Motor move column/row: ");
    Serial.print(move_column);
    Serial.println(move_row);
    #endif

    // přizazení odpovídajícího pohybu pro x
    if(move_column > 0){
        moveDec cur_move = {X_MOTOR, move_column, X_POSITIVE};
        addMove(cur_move);
    }else{
        moveDec cur_move = {X_MOTOR, abs(move_column), X_NEGATIVE};
        addMove(cur_move);
    }

    // přizazení odpovídajícího pohybu pro y
    if(move_row > 0){
        moveDec cur_move = {Y_MOTOR, move_row, Y_POSITIVE};
        addMove(cur_move);
    }else{
        moveDec cur_move = {Y_MOTOR, abs(move_row), Y_NEGATIVE};
        addMove(cur_move);
    }

}

MotorMovement::cellPos MotorMovement::decodePos(int _value){
    cellPos cell;

    // rozložení čísla na sloupce - x a řádky - y
    cell.y = _value % 10;
    cell.x = (_value - (_value % 10)) / 10;

    return cell;
}

void MotorMovement::addMove(moveDec _move){
    moves.push_back(_move); //přidání pohybu do vektoru pohybů
}

void MotorMovement::clearMoves(){
    moves.clear();  //vyčistí pole
}

void MotorMovement::printMoves(){
    //vypsání všech pohybů z vektoru pohybů
    Serial.print("Počet pohybu: ");
    Serial.println(moves.size());
    Serial.println("Pohyby motoru");

    for(int i = 0; i < moves.size(); i++){
        Serial.print("Motor: ");
        Serial.print(moves[i].motor);
        Serial.print(" Pocet policek: ");
        Serial.print(moves[i].cells);
        Serial.print(" Smer: ");
        Serial.println(moves[i].dir);
    }
}