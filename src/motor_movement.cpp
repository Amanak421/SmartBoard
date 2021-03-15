#include "motor_movement.h"

//#define DEBUG

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

    //uloží koncovou pozici jako aktuální pozici motorů
    act_cell_pos = endCell;
    /*Serial.print(endCell.x);
    Serial.println(endCell.y);*/

    #ifdef DEBUG    // debug
    Serial.print("Motor move column/row: ");
    Serial.print(move_column);
    Serial.println(move_row);
    #endif

    command move_type = selectMoveType(startCell, endCell);

    // přizazení odpovídajícího pohybu pro x
    if(move_column > 0){
        moveDec cur_move = {X_MOTOR, move_column, X_POSITIVE, move_type};
        addMove(cur_move);
    }else{
        moveDec cur_move = {X_MOTOR, abs(move_column), X_NEGATIVE, move_type};
        addMove(cur_move);
    }

    // přizazení odpovídajícího pohybu pro y
    if(move_row > 0){
        moveDec cur_move = {Y_MOTOR, move_row, Y_POSITIVE, move_type};
        addMove(cur_move);
    }else{
        moveDec cur_move = {Y_MOTOR, abs(move_row), Y_NEGATIVE, move_type};
        addMove(cur_move);
    }

}

void MotorMovement::computeCellMovement(cellPos _startCell, int _endCell){

    // vytvoření koncového políčka a rozložení hodnot na x-sloupce a y-řádky
    cellPos endCell = decodePos(_endCell);

    //výpočet početu políček, o které je nutné se posunout
    move_column = _startCell.x - endCell.x;
    move_row = _startCell.y - endCell.y;

    //uloží koncovou pozici jako aktuální pozici motorů
    act_cell_pos = endCell;

    #ifdef DEBUG    // debug
    Serial.print("Motor move column/row: ");
    Serial.print(move_column);
    Serial.println(move_row);
    #endif

    // přizazení odpovídajícího pohybu pro x
    if(move_column > 0){
        moveDec cur_move = {X_MOTOR, move_column, X_POSITIVE, LIN_MOVE};
        addMove(cur_move);
    }else{
        moveDec cur_move = {X_MOTOR, abs(move_column), X_NEGATIVE, LIN_MOVE};
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
    cell.x = _value % 10;
    cell.y = (_value - (_value % 10)) / 10;

    return cell;
}

bool MotorMovement::checkPath(cellPos _start, cellPos _end, pieceMoves _move){
    
    switch (_move){
    case STRAIGHT:
        if(_start.x == _end.x){
            
            int start_cell = min(_start.y, _end.y); //start kontroly bude na menším políčku
            int end_cell = max(_start.y, _end.y);   //konec kontroly bude na větším políčku

            #ifdef DEBUG
            Serial.print("Stejné x - hodnota: ");
            Serial.println(_start.x);
            Serial.print("Start pozice y: ");
            Serial.println(start_cell);
            #endif

            for(int i = start_cell + 1; i <= end_cell; i++){
                if(board[i][_start.x - 1] != 0){
                    return false;
                }
            }
        }else{
            
            int start_cell = min(_start.x - 1, _end.x - 1); //start kontroly bude na menším políčku
            int end_cell = max(_start.x - 1, _end.x - 1);   //konec kontroly bude na větším políčku

            #ifdef DEBUG
            Serial.print("Stejné y - hodnota: ");
            Serial.println(_start.y);
            Serial.print("Start pozice x: ");
            Serial.println(start_cell);
            #endif

            for(int i = start_cell + 1; i <= end_cell; i++){
                if(board[_start.y][i] != 0){
                    return false;
                }
            }
        }
        break;

    }

    return true;
}

MotorMovement::command MotorMovement::selectMoveType(cellPos _start, cellPos _end){

    /*Serial.print("x souradnice");
    Serial.print(_start.x);
    Serial.println(_end.x);*/
    
    switch (pieceValue(_start))
    {
    case 1:
        if(_start.x == _end.x){
            /*Serial.print("Volná cesta: ");
            Serial.println(checkPath(_start, _end, STRAIGHT));*/
            if(checkPath(_start, _end, STRAIGHT)){
                return CENTER_MOVE;
            }
        }
        break;
    case 2:
        if(checkPath(_start, _end, STRAIGHT)){
            return CENTER_MOVE;
        }
        break;
   
    }

    return LIN_MOVE;
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
        Serial.print(moves[i].dir);
        Serial.print(" Specialni kod: ");
        Serial.println(moves[i].special_command);
    }
}

void MotorMovement::returnToHome(){
    //vyčistí vektor pohybu
    clearMoves();
    //vypočítej cestu do výchozí pozice
    computeCellMovement(act_cell_pos, HOME_POSITION);

    /* SAMOTNÉ PROVEDENÍ POHYBU - ToDo  */

}

void MotorMovement::setUp(){

}

void MotorMovement::doMove(int _startCell, int _endCell){

    #ifdef DEBUG
    Serial.print("Aktuální pozice: ");
    Serial.print(act_cell_pos.x);
    Serial.println(act_cell_pos.y);
    #endif

    computeCellMovement(act_cell_pos, _startCell);  //vypočítá cestu cestu z aktuální pozice na startovní políčko
    computeCellMovement(_startCell, _endCell);  //vypočítá pohyb z políčka na políčko

    /*  SAMOTNÉ PROVEDENÍ TAHU  - ToDo */
}

int MotorMovement::pieceValue(cellPos _position){
    return board[_position.y][_position.x];
}

void MotorMovement::setBoard(int _board[8][8]){
    for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
            board[i][k] = _board[i][k];
        }
    }
}

void MotorMovement::printBoard(){
    for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
            Serial.print(board[i][k]);
            Serial.print(" ");
        }
        Serial.println();
    }
}