#include "motor_movement.h"

//#define DEBUG

MotorMovement::MotorMovement(){
    //přiřazení storage pro vector array
    moves.setStorage(move_storage);

    //nastavení políček pro vyhozené figurky
    for(int i = 0; i < 8; i++){
        int _y = i;
        pieceOutCells[i].y = _y;
        for(int k = 0; k < 4; k++){
            pieceOutCells[i].isPostFull[k] = 0;
        }
    }

}

void MotorMovement::computeCellMovement(int _startCell, int _endCell){

    // vytvoření počátečního a koncového políčka a rozložení hodnot na x-sloupce a y-řádky
    cellPos startCell = decodePos(_startCell);
    cellPos endCell = decodePos(_endCell);

    if(isPieceOutActive(endCell)){      //zkontroluje a popř. vyhodí figurku na cílové pozici
        Serial.println("Vyhazuji figurku...");
        //board[endCell.y][endCell.x - 1] = 0;
        addPieceOutMove(endCell, 0);
    }
    Serial.println("Provádím klasický tah...");

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
    Serial.print("Vybraný typ pohybu: ");
    Serial.println(move_type);

    //při různých typech pohybu správně nastavuje pohybuvý vektor
    if(move_type == DIA_MOVE){
        
        if(move_row > 0 && move_column > 0){
            moveDec cur_move_x = {X_MOTOR, move_row, X_POSITIVE, DIA_MOVE};
            addMove(cur_move_x);
            moveDec cur_move_y = {Y_MOTOR, move_row, Y_POSITIVE, DIA_MOVE};
            addMove(cur_move_y);
        }else if(move_row < 0 && move_column > 0){
            moveDec cur_move_x = {X_MOTOR, abs(move_row), X_POSITIVE, DIA_MOVE};
            addMove(cur_move_x);
            moveDec cur_move_y = {Y_MOTOR, abs(move_row), Y_NEGATIVE, DIA_MOVE};
            addMove(cur_move_y);
        }else if(move_row > 0 && move_column < 0){
            moveDec cur_move_x = {X_MOTOR, abs(move_row), X_NEGATIVE, DIA_MOVE};
            addMove(cur_move_x);
            moveDec cur_move_y = {Y_MOTOR, abs(move_row), Y_POSITIVE, DIA_MOVE};
            addMove(cur_move_y);
        }else{
            moveDec cur_move_x = {X_MOTOR, abs(move_row), X_NEGATIVE, DIA_MOVE};
            addMove(cur_move_x);
            moveDec cur_move_y = {Y_MOTOR, abs(move_row), Y_NEGATIVE, DIA_MOVE};
            addMove(cur_move_y);
        }

        return;
        
    }else if(move_type == CENTER_MOVE){

        if(en_horse_move == true && horse_move != -1){  //pokud je aktivní pohyb koněm
            if(horse_move == 0){
                if(abs(move_column) == 2){      // v případě pohybu po řádku
                    addHorseMove(move_column, move_row, X_MOTOR, Y_MOTOR, X_POSITIVE, Y_POSITIVE, X_NEGATIVE, Y_NEGATIVE);
                }else{      //v případě pohybu po sloupci
                    addHorseMove(move_row, move_column, Y_MOTOR, X_MOTOR, Y_POSITIVE, X_POSITIVE, Y_NEGATIVE, X_NEGATIVE);
                }
            }else if(horse_move == 1){
                if(abs(move_column) == 2){  // v případě pohybu po řádku
                    addHorseMove(move_row, move_column, Y_MOTOR, X_MOTOR, Y_POSITIVE, X_POSITIVE, Y_NEGATIVE, X_NEGATIVE);
                }else{      //v případě pohybu po sloupci
                    addHorseMove(move_column, move_row, X_MOTOR, Y_MOTOR, Y_POSITIVE, X_POSITIVE, Y_NEGATIVE, X_NEGATIVE);
                }
            }

            //zpět nastaví hodnoty pohybu koně na neaktivní
            horse_move = -1;
            en_horse_move = false;
            return; // ukončí fci
        }

        
    }

    Serial.println("Dostal jsem se sem...");

    // přizazení odpovídajícího pohybu pro x - při pohybu po okrajích
    if(move_column > 0){
        moveDec cur_move = {X_MOTOR, move_column, X_POSITIVE, move_type};
        addMove(cur_move);
    }else{
        moveDec cur_move = {X_MOTOR, abs(move_column), X_NEGATIVE, move_type};
        addMove(cur_move);
    }

    // přizazení odpovídajícího pohybu pro y - při pohybu po okrajích
    if(move_row > 0){
        moveDec cur_move = {Y_MOTOR, move_row, Y_POSITIVE, move_type};
        addMove(cur_move);
    }else{
        moveDec cur_move = {Y_MOTOR, abs(move_row), Y_NEGATIVE, move_type};
        addMove(cur_move);
    }

}

void MotorMovement::computeHomeToCellMovement(cellPos _startCell, cellPos _endCell){


    //výpočet početu políček, o které je nutné se posunout
    move_column = _startCell.x - _endCell.x;
    move_row = _startCell.y - _endCell.y;

    //uloží koncovou pozici jako aktuální pozici motorů
    act_cell_pos = _endCell;

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

void MotorMovement::addHorseMove(int _moveCell_a, int _moveCell_b, motorPick _motor1, motorPick _motor2, int _a_positive, int _b_positive, int _a_negative, int _b_negative){
    if(_moveCell_a > 0){
        moveDec cur_move = {_motor1, _moveCell_a, _a_positive, CENTER_MOVE};
        addMove(cur_move);
    }else{
        moveDec cur_move = {_motor1, abs(_moveCell_a), _a_negative, CENTER_MOVE};
        addMove(cur_move);
    }
    if(_moveCell_b > 0){
        moveDec cur_move = {_motor2, _moveCell_b, _b_positive, CENTER_MOVE};
        addMove(cur_move);
    }else{
        moveDec cur_move = {_motor2, abs(_moveCell_b), _b_negative, CENTER_MOVE};
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

            for(int i = start_cell; i <= end_cell; i++){
                if((_start.y > _end.y) && i == end_cell){       //pokud se trasa zkoumá od posledního políčka k prvnímu -> vynechej poslední políčko
                    continue;
                }else if((_start.y < _end.y) && i == start_cell){       //pokud se trasa zkoumá od prvního políčka k poslednímu -> vynechej první políčko
                    continue;
                }
                Serial.println("testuji...");
                if(board[i][_start.x - 1] != 0){
                    Serial.println("Chyba!!!");
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

            for(int i = start_cell; i <= end_cell; i++){
                if((_start.x > _end.x) && i == end_cell){       //pokud se trasa zkoumá zprava doleva -> vynechej poslední políčko
                    continue;
                }else if((_start.x < _end.x) && i == start_cell){       //pokud se trasa zkoumá zleva doprava -> vynechej první políčko
                    continue;
                }
                if(board[_start.y][i] != 0){
                    return false;
                }
            }
        }
        break;

    case DIAGONAL: {

        int start_cell = min(_start.y, _end.y);
        int end_cell = max(_start.y, _end.y);

        #ifdef DEBUG
            Serial.print("Startovní / konečné políčko: ");
            Serial.print(start_cell);
            Serial.print(" / ")
            Serial.println(end_cell);
        #endif

        for(int i = 1; i <= (end_cell - start_cell); i++){
            if((_start.y > _end.y) && i == (end_cell - start_cell)){
                continue;
            }else if((_start.y < _end.y) && i == 1){
                continue;
            }
            if(board[_start.y + i][_start.x - 1 + i]){
                return false;
            }
        }
        break;
        }

    case HORSE:

        int start_x = min(_start.x-1, _end.x-1);    //nastavení pozice, ze které začínáme zkoumat x
        int end_x = max(_start.x-1, _end.x-1);

        int start_y = min(_start.y, _end.y);        //nastavení pozice, ze které začínáme zkoumat y
        int end_y = max(_start.y, _end.y);

        #ifdef DEBUG
        Serial.print("Start / end - x: ");
        Serial.print(start_x);
        Serial.println(end_x);
        Serial.print("Start / end - y: ");
        Serial.print(start_y);
        Serial.println(end_y);
        #endif

        if(end_x - start_x == 2){   //zkoumá pohyb koně pokud je po ose x

            for(int i = start_x; i <= end_x; i++){      //zkoumání v řádku startovního políčka
                if(_start.x < _end.x && i == start_x){
                    continue;
                }

                if(_start.x > _end.x && i == end_x){
                    horse_move = 0;
                    return true;
                }

                if(board[_start.y][i] != 0){
                    break;
                }else if(board[_start.y][i] == 0 && _start.x < _end.x && i == end_x){
                    horse_move = 0;
                    return true;
                }

            }

            for(int i = start_x; i <= end_x; i++){      //zkoumání v posunutém řádku
                if(board[_start.y - (_start.y - _end.y)][i] != 0){
                    return false;
                }
            }

            horse_move = 1;
        }

        if(end_y - start_y == 2){       //zkoumá pohyb koně pokud je po ose y

            for(int i = start_y; i <= end_y; i++){
                if(_start.y < _end.y && i == start_y){
                    continue;
                }

                if(_start.y > _end.y && i == end_y){
                    horse_move = 0;
                    return true;
                }
                
                if(board[i][_start.x - 1] != 0){
                    break;
                }else if(board[i][_start.x - 1] == 0 && _start.y < _end.y && i == end_y){
                    horse_move = 0;
                    return true;
                }

            }

            for(int i = start_y; i <= end_y; i++){
                if(board[i][_start.x - 1 - (_start.x - _end.x)] != 0){
                    return false;
                }
            }

            horse_move = 1;

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

    case 3:
        if(checkPath(_start, _end, HORSE)){
            en_horse_move = true;
            return CENTER_MOVE;
        }
        break;

    case 4:
        if(checkPath(_start, _end, DIAGONAL)){
            return DIA_MOVE;
        }
        break;
    
    case 5:
        if(_start.x == _end.x || _start.y == _end.y){
            if(checkPath(_start, _end, STRAIGHT)){
                return CENTER_MOVE;
            }
        }else{
            if(checkPath(_start, _end, DIAGONAL)){
                return DIA_MOVE;
            }
        }
        break;

    case 6:
        if(_start.x == _end.x || _start.y == _end.y){
            if(checkPath(_start, _end, STRAIGHT)){
                return CENTER_MOVE;
            }
        }else{
            if(checkPath(_start, _end, DIAGONAL)){
                return DIA_MOVE;
            }
        }
        break;
    }

    return LIN_MOVE;
}

void MotorMovement::addMove(moveDec _move){
    Serial.println("Přidávám pohyb...");
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
        Serial.print(moves[i].special_command);
        if(moves[i].special_command == PIECE_OUT){
            Serial.print(" Vyhození figurky s pozicí: ");
            Serial.print(sel_piece_post);
        }
        Serial.println();
    }
}


int MotorMovement::pieceValue(cellPos _position){
    return board[_position.y][_position.x-1];
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

void MotorMovement::returnToHome(){
    //vyčistí vektor pohybu
    clearMoves();
    //vypočítej cestu do výchozí pozice
    

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

    //vypočítá cestu cestu z aktuální pozice na startovní políčko
    computeCellMovement(_startCell, _endCell);  //vypočítá pohyb z políčka na políčko

    /*  SAMOTNÉ PROVEDENÍ TAHU  - ToDo */
}

bool MotorMovement::isPieceOutActive(cellPos _cell){
    if(board[_cell.y][_cell.x - 1] != 0){
        return true;
    }
    return false;
}

void MotorMovement::addPieceOutMove(cellPos _cell, int _pieceColor){
    int selectedPieceOutCell = 0;
    int selectedCellPost = 0;

    //cyklus pro výběr místa a pozice pro figurku
    int _startCell;
    if(_pieceColor == 0){
        _startCell = 0;
    }else{
        _startCell = 4;
    }
    for(int i = _startCell; i < _startCell + 4; i ++){
        if(!pieceOutCells[i].isFull){
            for(int k = 0; k < 4; k++){
                if(!pieceOutCells[i].isPostFull[k]){
                    selectedPieceOutCell = i;
                    selectedCellPost = k;
                    break;
                }
            }
        }
    }

    //uložení vybraného místa pro figurku
    sel_piece_post = selectedCellPost;

    //výpočet početu políček, o které je nutné se posunout
    int out_move_row = _cell.y - pieceOutCells[selectedPieceOutCell].y; 
    int out_move_column = _cell.x - pieceOutCells[selectedPieceOutCell].x;

    pieceOutMoveAdder(out_move_row, out_move_column, PIECE_OUT);    //přidá pohyb motorů

    board[_cell.y][_cell.x - 1] = 0;    //nastaví pozici, na které se nachází vyhozená figurka na 0 - bez figurky

    //výpočet početu políček, o které je nutné se vrátit
    int ret_move_row = pieceOutCells[selectedPieceOutCell].y  - _cell.y;
    int ret_move_column = pieceOutCells[selectedPieceOutCell].x - _cell.x;

    pieceOutMoveAdder(ret_move_row, ret_move_column, RET_PIECE_OUT);    //přidá pohyb motorů

    //obsazení místa na políčku
    pieceOutCells[selectedPieceOutCell].isPostFull[selectedCellPost] = true;
    //pokud jsou obsazena všechna místa -> políčko je plné
    if(selectedCellPost == 3){
        pieceOutCells[selectedPieceOutCell].isFull = true;
    }

}

void MotorMovement::pieceOutMoveAdder(int _move_row, int _move_column, command _command){
    // přizazení odpovídajícího pohybu pro x - při pohybu po okrajích
    if(_move_column > 0){
        moveDec cur_move = {X_MOTOR, _move_column, X_POSITIVE, _command};
        addMove(cur_move);
    }else{
        moveDec cur_move = {X_MOTOR, abs(_move_column), X_NEGATIVE, _command};
        addMove(cur_move);
    }

    // přizazení odpovídajícího pohybu pro y - při pohybu po okrajích
    if(_move_row > 0){
        moveDec cur_move = {Y_MOTOR, _move_row, Y_POSITIVE, _command};
        addMove(cur_move);
    }else{
        moveDec cur_move = {Y_MOTOR, abs(_move_row), Y_NEGATIVE, _command};
        addMove(cur_move);
    }
}