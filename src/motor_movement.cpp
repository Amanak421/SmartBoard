#include "motor_movement.h"
#include "A4988.h"
#include "SyncDriver.h"

//nastavení motorů
#define MOTOR_STEPS 200
#define RPM 120
#define MICROSTEPS 4

#define MS1 2
#define MS2 2
#define MS3 2

//motor x
#define DIR_X 14
#define STEP_X 12

A4988 stepper_x(MOTOR_STEPS, DIR_X, STEP_X, MS1, MS2, MS3);


//motor y
#define DIR_Y  25
#define STEP_Y 26

A4988 stepper_y(MOTOR_STEPS, DIR_Y, STEP_Y, MS1, MS2, MS3);

SyncDriver diaMoveControl(stepper_x, stepper_y);

#define DEBUG

MotorMovement::MotorMovement(){
    //přiřazení storage pro vector array
    moves.setStorage(move_storage);

    //nastavení políček pro vyhozené figurky
    for(int i = 0; i < 8; i++){
        int _y = i;
        pieceOutCells[i].y = _y;
        pieceOutCells[i].x = 0;
        pieceOutCells[i].isFull = false;
        for(int k = 0; k < 4; k++){
            pieceOutCells[i].isPostFull[k] = false;
        }
    }

    //připojení endstopů
    pinMode(X_MAX, INPUT);
    pinMode(Y_MAX, INPUT);
    pinMode(X_MIN, INPUT);
    pinMode(Y_MIN, INPUT);

    //připojení elektrmagnetu
    pinMode(ELL_PIN, OUTPUT);
    digitalWrite(ELL_PIN, LOW);

    //aktivování motorů
    stepper_x.begin(RPM, MICROSTEPS);
    stepper_y.begin(RPM, MICROSTEPS);
    stepper_x.enable();
    stepper_y.enable();


    Serial.println("INIT DONE");

}

void MotorMovement::computeCellMovement(int _startCell, int _endCell){

    clearMoves();   //vyčistí vektor s pohyby
    last_special_move = "none";

    // vytvoření počátečního a koncového políčka a rozložení hodnot na x-sloupce a y-řádky
    cellPos startCell = decodePos(_startCell);
    cellPos endCell = decodePos(_endCell);

    


    if(isPieceOutActive(endCell)){      //zkontroluje a popř. vyhodí figurku na cílové pozici
        Serial.println("Vyhazuji figurku...");
        //board[endCell.y][endCell.x - 1] = 0;
        computeHomeToCellMovement(act_cell_pos, endCell);
        addPieceOutMove(endCell, 0);
    }
    Serial.println("Provadim klasicky tah...");

    //výpočet početu políček, o které je nutné se posunout
    move_column = startCell.x - endCell.x;
    move_row = startCell.y - endCell.y;

    //vypočte cestu z aktuální pozice do rohu první buňky
    computeHomeToCellMovement(act_cell_pos, startCell);

    //uloží koncovou pozici jako aktuální pozici motorů
    act_cell_pos = endCell;
    /*Serial.print(endCell.x);
    Serial.println(endCell.y);*/

    #ifdef DEBUG    // debug
    Serial.print("Motor move column/row: ");
    Serial.print(move_column);
    Serial.println(move_row);
    #endif

    if(pieceValue(startCell) == 6 && abs(move_column) > 1){
        addCastlingMove(startCell, endCell);
        last_special_move = "cast";
        return;
    }else if(pieceValue(endCell) == 0 && pieceValue(startCell) == 1 && abs(startCell.x - endCell.x) == 1){
        addEnPassantMove(startCell, endCell, actual_turn);
        last_special_move = "enpas";
        return;
    }else{
        last_special_move = "none";
    }


    command move_type = selectMoveType(startCell, endCell);
    Serial.print("Vybrany typ pohybu: ");
    Serial.println(move_type);

    //posunutí figurky
    board[endCell.y][endCell.x - 1] = board[startCell.y][startCell.x - 1];
    board[startCell.y][startCell.x - 1] = 0;


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
    int m_move_column = _startCell.x - _endCell.x;
    int m_move_row = _startCell.y - _endCell.y;

    //uloží koncovou pozici jako aktuální pozici motorů
    act_cell_pos = _endCell;

    #ifdef DEBUG    // debug
    Serial.print("Motor move column/row: ");
    Serial.print(move_column);
    Serial.println(move_row);
    #endif

    // přizazení odpovídajícího pohybu pro x
    if(m_move_column > 0){
        moveDec cur_move = {X_MOTOR, m_move_column, X_POSITIVE, BOTH};
        addMove(cur_move);
    }else{
        moveDec cur_move = {X_MOTOR, abs(m_move_column), X_NEGATIVE, BOTH};
        addMove(cur_move);
    }

    // přizazení odpovídajícího pohybu pro y
    if(m_move_row > 0){
        moveDec cur_move = {Y_MOTOR, m_move_row, Y_POSITIVE, BOTH};
        addMove(cur_move);
    }else{
        moveDec cur_move = {Y_MOTOR, abs(m_move_row), Y_NEGATIVE, BOTH};
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

        int x_cell_inveter = 1; //bude odečítat pokud se bude pohyb zkoumat po klesajícím x a přičítat pokud po stoupajícím

        if((_start.x < _end.x && _start.y > _end.y) || (_start.x > _end.x && _start.y < _end.y)){   //v těchto případech je potřeba odečítat x
            x_cell_inveter = -1;
        }else{
            x_cell_inveter = 1;
        }

        #ifdef DEBUG
            Serial.print("Startovni / konecne policko: ");
            Serial.print(start_cell);
            Serial.print(" / ");
            Serial.println(end_cell);
        #endif

        for(int i = 1; i <= (end_cell - start_cell); i++){
            Serial.print("Kontrola policek: ");
            Serial.print(_start.y + i);
            Serial.print(" / ");
            Serial.println(_start.x - 1 + i);
            if((_start.y > _end.y) && i == (end_cell - start_cell)){
                continue;
            }else if((_start.y < _end.y) && i == 1){
                continue;
            }
            if(board[start_cell + i][(_start.x - 1) + (i * x_cell_inveter)] != 0){
                Serial.println("Kontrola selhala...");
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
        }else{
            if(checkPath(_start, _end, DIAGONAL)){
                return DIA_MOVE;
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
    Serial.print("Pocet pohybu: ");
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
            Serial.print(" Vyhození figurky s pozici: ");
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

    if(_pieceColor == 0){
 
        bool end_cycle = false;
        for(int i = 0; i < 4; i++){
            if(pieceOutCells[i].isFull == false){
                for(int k = 0; k < 4; k++){
                    if(pieceOutCells[i].isPostFull[k] == false){
                        selectedPieceOutCell = i;
                        selectedCellPost = k;
                        end_cycle = true;
                        break;
                    }
                }
            }
            if(end_cycle){
                break;
            }
        }

    }else{

        bool end_cycle = false;
        for(int i = 7; i > 3; i--){
            if(pieceOutCells[i].isFull == false){
                for(int k = 0; k < 4; k++){
                    if(pieceOutCells[i].isPostFull[k] == false){
                        selectedPieceOutCell = i;
                        selectedCellPost = k;
                        end_cycle = true;
                        break;
                    }
                }
            }
            if(end_cycle){
                break;
            }
        }
    }
    

    Serial.println(selectedPieceOutCell);

    //uložení vybraného místa pro figurku
    sel_piece_post = selectedCellPost;

    int correction = 0; //korekce pro správné fungování vyhazování figurek
    if(_pieceColor == 0){
        correction = 1;
    }else if(_pieceColor == 1){
        correction = 0;
    }

    piece_out_cor = correction;

    //výpočet početu políček, o které je nutné se posunout
    int out_move_row = _cell.y - pieceOutCells[selectedPieceOutCell].y - correction;     //přidá korekci - směr nahoru o 1 min
    int out_move_column = _cell.x - pieceOutCells[selectedPieceOutCell].x;

    pieceMoveAdder(out_move_row, out_move_column, PIECE_OUT);    //přidá pohyb motorů

    board[_cell.y][_cell.x - 1] = 0;    //nastaví pozici, na které se nachází vyhozená figurka na 0 - bez figurky

    //výpočet početu políček, o které je nutné se vrátit
    int ret_move_row = pieceOutCells[selectedPieceOutCell].y  - _cell.y + correction;     //přidá korekci - správný návrat
    int ret_move_column = pieceOutCells[selectedPieceOutCell].x - _cell.x;

    pieceMoveAdder(ret_move_row, ret_move_column, RET_PIECE_OUT);    //přidá pohyb motorů

    //obsazení místa na políčku
    pieceOutCells[selectedPieceOutCell].isPostFull[selectedCellPost] = true;
    //pokud jsou obsazena všechna místa -> políčko je plné
    if(selectedCellPost == 3){
        pieceOutCells[selectedPieceOutCell].isFull = true;
    }

}

void MotorMovement::pieceMoveAdder(int _move_row, int _move_column, command _command){
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

void MotorMovement::addCastlingMove(cellPos _start, cellPos _end){
    if(_start.x == 5 && _end.x == 3){  //rošáda je prováděna králem

        int k_move_row = _start.y - _end.y;
        int k_move_column = _start.x - _end.x;
        pieceMoveAdder(k_move_row, k_move_column, CENTER_MOVE); //tah králem

        pieceMoveAdder(0, _end.x - 1, BOTH); //cesta k věži

        int c_move_row = 0;
        int c_move_column = 4;
        pieceMoveAdder(c_move_row, c_move_column, LIN_MOVE); //tah věží

        act_cell_pos = {4, _start.y};   //zaktualizuje aktuální pozici

    }else if(_start.x == 5 && _end.x == 7){

        int q_move_row = _start.y - _end.y;
        int q_move_column = _start.x - _end.x;
        pieceMoveAdder(q_move_row, q_move_column, CENTER_MOVE);

        pieceMoveAdder(0, _end.x - 8, BOTH); //cesta k věži

        int c_move_row = 0;
        int c_move_column = 2;
        pieceMoveAdder(c_move_row, c_move_column, LIN_MOVE);

        act_cell_pos = {6, _start.y};   //zaktualizuje aktuální pozici

    }
}

void MotorMovement::addEnPassantMove(cellPos _start, cellPos _end, int _pieceOutColor){
    
    pieceMoveAdder(0, (_start.y - _end.y)*(-1), BOTH);  //přesuna motory k pěšci, kterého je potřeba vyhodit

    cellPos out_piece = {_start.x + (_start.y - _end.y), _start.y};   //vyhodí pěšce
    addPieceOutMove(out_piece, _pieceOutColor);

    pieceMoveAdder(0, (_start.y - _end.y), BOTH);    //vrátí se k původnímu pěšci

    int _move_row = _start.y - _end.y;  //přidá tah pěšcem
    int _move_column = _start.x - _end.x;
    pieceMoveAdder(_move_row, _move_column, DIA_MOVE);

    act_cell_pos = _end;    //zaktualizuje aktuální pozici
}

void MotorMovement::returnToHome(){

    moveToEndstop(MOTOR_X, -1);
    delay(100);
    moveToEndstop(MOTOR_X, -1);
    delay(100);
    moveToEndstop(MOTOR_Y, 1);
    delay(100);
    moveToEndstop(MOTOR_Y, 1);
    delay(100);
    stepper_x.rotate(calculateAngle(rest_cell_x) * ACC_MOTOR_MOVE);
    act_cell_pos = {1, 0};

}

void MotorMovement::moveToEndstop(int _XY, int _direction){

    if(_XY == MOTOR_X){   //motor x
        Serial.println("START MOTOR X");
        stepper_x.startMove(100 * _direction * MOTOR_STEPS * MICROSTEPS);
        while(digitalRead(X_MIN) == LOW && digitalRead(X_MAX) == LOW){
            stepper_x.nextAction();
        }
        Serial.print("ENDSTOP REACHED in direction: ");
        Serial.println(_direction);
        stepper_x.stop();
        stepper_x.rotate(90 * ACC_MOTOR_MOVE * _direction * -1);
    }else if(_XY == MOTOR_Y){   //motor y
        Serial.println("START MOTOR Y");
        stepper_y.startMove(100 * _direction * MOTOR_STEPS * MICROSTEPS);
        while(digitalRead(Y_MIN) == LOW && digitalRead(Y_MAX) == LOW){
            stepper_y.nextAction();
        }
        Serial.print("ENDSTOP REACHED in direction: ");
        Serial.println(_direction);
        stepper_y.stop();
        stepper_y.rotate(202.5 * ACC_MOTOR_MOVE * _direction * -1);
    }

}

void MotorMovement::setMagnetState(int _state){
    if(_state == ON){
        digitalWrite(ELL_PIN, HIGH);
    }else if(_state == OFF){
        digitalWrite(ELL_PIN, LOW);
    }

    delay(500);
    
}

double MotorMovement::calculateAngle(int _milimeters){
    int _value = _milimeters * ANGLE_PER_MILIMETER;
    return _value;
}

void MotorMovement::moveCorToCen(int _dir){
    if(_dir == 1 && motor_cc_position == false){
        //provede pohyb na střed políčka
        stepper_x.rotate(calculateAngle(cell_x / 2) * ACC_MOTOR_MOVE);
        stepper_y.rotate(calculateAngle(cell_y / 2) * ACC_MOTOR_MOVE * -1);
        motor_cc_position = true; //zapamatuje si tuto pozici
    }else if(_dir == -1 && motor_cc_position == true){
        //provede pohyb na okraj políčka
        stepper_x.rotate(calculateAngle(cell_x / 2) * ACC_MOTOR_MOVE * -1);
        stepper_y.rotate(calculateAngle(cell_y / 2) * ACC_MOTOR_MOVE);
        motor_cc_position = false; //zapamatuje si totu volbu
    }else{
        Serial.println("Tento pohyb nemuze byt proveden!");
        return;
    }
}
void MotorMovement::moveMotorEStop(double _angle, int _dir, int _motor){
    int dir = 0;

    if(_dir == 0){
        dir = -1;
    }else if(_dir == 1){
        dir = 1;
    }

    if(_motor == X_MOTOR){
        stepper_x.rotate(_angle * dir * ACC_MOTOR_MOVE);
    }else if(_motor == Y_MOTOR){
        stepper_y.rotate(_angle * dir * ACC_MOTOR_MOVE);
    }
    
    Serial.println("Pohyb proveden...");

}

void MotorMovement::doMotorMove(){
    setMagnetState(OFF);
    bool is_piece_at_corner = false;
    bool is_magnet_in_center = false;
    bool dia_skip_move = false;


    for(int i = 0; i < moves.size(); i++){
        if(moves[i].special_command == LIN_MOVE){

            if(is_piece_at_corner == false && (moves[i-1].special_command != moves[i].special_command || i == 0)){
                Serial.println("Beru figurku ze stredu...");
                moveCorToCen(1);
                delay(2000);
                setMagnetState(ON);  //zapne magnet
                delay(250);
                moveCorToCen(-1);
                is_piece_at_corner = true;
            }
            

            Serial.print("Provadim pohyb s motorem: ");
            Serial.print(moves[i].motor);
            Serial.print(" o pocet policek: ");
            Serial.print(moves[i].cells);
            Serial.print(" coz je uhel: ");
            Serial.print(calculateAngle(cell_x * moves[i].cells));
            Serial.print(" ve smeru: ");
            Serial.println(moves[i].dir);

            if(moves[i].cells == 0){
                Serial.println("Pohyb preskocen...");
                continue;
            }

            moveMotorEStop(calculateAngle(cell_x * moves[i].cells), moves[i].dir, moves[i].motor);

            if(is_piece_at_corner != false && (moves[i+1].special_command != moves[i].special_command || i == moves.size() - 1)){
                Serial.println("Vracim figurku do stredu...");
                moveCorToCen(1);
                delay(2000);
                setMagnetState(OFF);  //vypne elektromagnet
                delay(250);
                moveCorToCen(-1);
                is_piece_at_corner = false;

            }

            if(moves.size() - 1 == i){  //pokud to byl polední pohyb ukončí funkci
                    return;
            }

        }else if(moves[i].special_command == DIA_MOVE){

            if(is_magnet_in_center == false && (moves[i-1].special_command != moves[i].special_command || i == 0)){
                moveCorToCen(1);
                setMagnetState(ON);
                delay(250);
                is_magnet_in_center = true;
            }

            if(dia_skip_move){
                continue;
                dia_skip_move = false;
            }

            doDiagonalMove(calculateAngle(cell_x * moves[i].cells), calculateAngle(cell_x * moves[i+1].cells), moves[i].dir, moves[i+1].dir);
            dia_skip_move = true;

            if(is_magnet_in_center == true && (moves[i-1].special_command != moves[i].special_command || i == moves.size() - 1)){
                setMagnetState(OFF);
                delay(250);
                moveCorToCen(-1); 
                is_magnet_in_center = true;
            }



        }else if(moves[i].special_command == CENTER_MOVE){

            if(is_magnet_in_center == false && (moves[i-1].special_command != moves[i].special_command || i == 0)){
                moveCorToCen(1);
                setMagnetState(ON);
                delay(250);
                is_magnet_in_center = true;
            }

            Serial.print("Provadim pohyb s motorem: ");
            Serial.print(moves[i].motor);
            Serial.print(" o pocet policek: ");
            Serial.print(moves[i].cells);
            Serial.print(" coz je uhel: ");
            Serial.print(calculateAngle(cell_x * moves[i].cells));
            Serial.print(" ve smeru: ");
            Serial.println(moves[i].dir);

            if(moves[i].cells == 0){
                Serial.println("Pohyb preskocen...");
                continue;
            }

            
            moveMotorEStop(calculateAngle(cell_x * moves[i].cells), moves[i].dir, moves[i].motor);

            if(is_magnet_in_center == true && (moves[i+1].special_command != moves[i].special_command || i == moves.size() - 1)){
                setMagnetState(OFF);
                delay(250);
                moveCorToCen(-1);
                is_magnet_in_center = false;
            }

        
        }else if(moves[i].special_command == PIECE_OUT){

            if(is_magnet_in_center == false && (moves[i-1].special_command != moves[i].special_command || i == 0)){
                moveCorToCen(1);
                setMagnetState(ON);
                delay(250);
                is_magnet_in_center = false;
                moveCorToCen(-1);
            }

            Serial.print("Provadim pohyb s motorem: ");
            Serial.print(moves[i].motor);
            Serial.print(" o pocet policek: ");
            Serial.print(moves[i].cells);
            Serial.print(" coz je uhel: ");
            Serial.print(calculateAngle((cell_x * (moves[i].cells - 1)) + rest_cell_x));
            Serial.print(" ve smeru: ");
            Serial.println(moves[i].dir);

            if(moves[i].motor == X_MOTOR){
                moveMotorEStop(calculateAngle((cell_x * (moves[i].cells - 1)) + rest_cell_x), moves[i].dir, moves[i].motor);
            }else{
                moveMotorEStop(calculateAngle(cell_x * moves[i].cells), moves[i].dir, moves[i].motor);
            }

            /* POHYB NA PŘÍSLUČNOU POZICI NA VYHAZOVACÍM POLÍČKU    */
            if((moves[i+1].special_command != moves[i].special_command || i == moves.size() - 1)){
                doPieceOutPos();
            }


        }else if(moves[i].special_command == RET_PIECE_OUT){

            Serial.print("Provadim pohyb s motorem: ");
            Serial.print(moves[i].motor);
            Serial.print(" o pocet policek: ");
            Serial.print(moves[i].cells);
            Serial.print(" coz je uhel: ");
            Serial.print(calculateAngle((cell_x * (moves[i].cells - 1)) + rest_cell_x));
            Serial.print(" ve smeru: ");
            Serial.println(moves[i].dir);

            if(moves[i].motor == X_MOTOR){
                moveMotorEStop(calculateAngle((cell_x * (moves[i].cells - 1)) + rest_cell_x), moves[i].dir, moves[i].motor);
            }else{
                moveMotorEStop(calculateAngle(cell_x * moves[i].cells), moves[i].dir, moves[i].motor);
            }

        }else if(moves[i].special_command == BOTH){
            setMagnetState(OFF);  //vypne magnet

            if(moves[i].cells == 0){
                Serial.println("Pohyb preskocen...");
                continue;
            }
            Serial.println("Pohyb k policku...");
            Serial.print("Provadim pohyb s motorem: ");
            Serial.print(moves[i].motor);
            Serial.print(" o pocet policek: ");
            Serial.print(moves[i].cells);
            Serial.print(" coz je uhel: ");
            Serial.print(calculateAngle(cell_x * moves[i].cells));
            Serial.print(" ve smeru: ");
            Serial.println(moves[i].dir);
            moveMotorEStop(calculateAngle(cell_x * moves[i].cells), moves[i].dir, moves[i].motor);
        }

        delay(2000);
    }



}

void MotorMovement::doPieceOutPos(){
    switch (sel_piece_post)
    {
    case 0:
        moveMotorEStop(calculateAngle((3 * rest_cell_x) / 4), 1, X_MOTOR);
        moveMotorEStop(calculateAngle((3 * rest_cell_y) / 4), 1, Y_MOTOR);
        setMagnetState(OFF);
        delay(50);
        Serial.println("Vracim se na pozici");
        moveMotorEStop(calculateAngle((3 * rest_cell_x) / 4), 0, X_MOTOR);
        moveMotorEStop(calculateAngle((3 * rest_cell_y) / 4), 0, Y_MOTOR);
        break;

    case 1:
        moveMotorEStop(calculateAngle(rest_cell_x / 4), 1, X_MOTOR);
        moveMotorEStop(calculateAngle((3 * rest_cell_y) / 4), 1, Y_MOTOR);
        setMagnetState(OFF);
        delay(50);
        Serial.println("Vracim se na pozici");
        moveMotorEStop(calculateAngle(rest_cell_x / 4), 0, X_MOTOR);
        moveMotorEStop(calculateAngle((3 * rest_cell_y) / 4), 0, Y_MOTOR);
        break;

    case 2:
        moveMotorEStop(calculateAngle((3 * rest_cell_x) / 4), 1, X_MOTOR);
        moveMotorEStop(calculateAngle(rest_cell_y / 4), 1, Y_MOTOR);
        setMagnetState(OFF);
        delay(50);
        Serial.println("Vracim se na pozici");
        moveMotorEStop(calculateAngle((3 * rest_cell_x) / 4), 0, X_MOTOR);
        moveMotorEStop(calculateAngle(rest_cell_y / 4), 0, Y_MOTOR);
        break;

    case 3:
        moveMotorEStop(calculateAngle(rest_cell_x / 4), 1, X_MOTOR);
        moveMotorEStop(calculateAngle(rest_cell_y / 4), 1, Y_MOTOR);
        setMagnetState(OFF);
        delay(50);
        Serial.println("Vracim se na pozici");
        moveMotorEStop(calculateAngle(rest_cell_x / 4), 0, X_MOTOR);
        moveMotorEStop(calculateAngle(rest_cell_y / 4), 0, Y_MOTOR);
        break;
    
    default:
        break;
    }

}

void MotorMovement::doDiagonalMove(double _angle_x, double _angle_y, int _dir_x, int _dir_y){

    int dir_x = 0;
    if(_dir_x == 0){
        dir_x = -1;
    }else{
        dir_x = 1;
    }

    int dir_y = 0;
    if(_dir_y == 0){
        dir_y = -1;
    }else{
        dir_y = 1;
    }

    diaMoveControl.rotate(_angle_x * dir_x, _angle_y * dir_y);

}

void MotorMovement::doMoveFromServer(String _move){

    String from_str = _move.substring(0, 2);
    int from = from_str.toInt() + 1;
    String to_str = _move.substring(3);
    int to = to_str.toInt() + 1;

    Serial.print("Z: ");
    Serial.println(from);
    Serial.print("NA: ");
    Serial.println(to);

    computeCellMovement(from, to);
    doMotorMove();
    returnToHome();

}

void MotorMovement::doMoveWithoutMotors(int _from, int _to){

    int from_column = _from % 10;
    int from_row = (_from - (_from % 10)) / 10;

    int to_column = _to % 10;
    int to_row = (_to - (_to % 10)) / 10;

    board[to_row][to_column-1] = board[from_row][from_column-1];
    board[from_row][from_column-1] = 0;

}