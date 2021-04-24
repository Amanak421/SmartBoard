#include "chess_board.h"

#include "chess_engine.h"

ChessBoard::ChessBoard(){
}

void ChessBoard::doMove(int _from, int _to){

    int from_column = _from % 10;
    int from_row = (_from - (_from % 10)) / 10;

    int to_column = _to % 10;
    int to_row = (_to - (_to % 10)) / 10;

    if(fen_board[from_row][from_column] == 'K'){
        w_cast_q = false;
    }else if(fen_board[from_row][from_column] == 'k'){
        b_cast_q = false;
    }

    num_board[to_row][to_column-1] = num_board[from_row][from_column-1];
    num_board[from_row][from_column-1] = 0;

    fen_board[to_row][to_column-1] = fen_board[from_row][from_column-1];
    fen_board[from_row][from_column-1] = ' ';

    last_from = from_row * 10 + from_column - 1;
    last_to = to_row * 10 + to_column - 1;
    
}

void ChessBoard::doMove(int _from, int _to, String _spec){
    if(reverse){
        int _from_x = _from % 10;
        int _to_x = _to % 10;

        _from = (70 + _from_x*2) - _from;
        _to = (70 + _to_x*2) - _to;
    }

    int from_column = _from % 10;
    int from_row = (_from - (_from % 10)) / 10;

    int to_column = _to % 10;
    int to_row = (_to - (_to % 10)) / 10;

    if(_spec == "castk"){

        if(fen_board[from_row][from_column] == 'K'){
            w_cast_k = false;
        }else{
            b_cast_k = false;
        }

        num_board[to_row][to_column - 1] = num_board[from_row][from_column];
        num_board[to_row][5] = 2;
        num_board[to_row][7] = 0;
        num_board[from_row][from_column - 1] = 0;

        fen_board[to_row][to_column - 1] = fen_board[from_row][from_column];
        fen_board[to_row][5] = fen_board[to_row][0];
        fen_board[to_row][7] = ' ';
        fen_board[from_row][from_column - 1] = ' ';

    }else if(_spec == "castq"){

        if(fen_board[from_row][from_column] == 'K'){
            w_cast_q = false;
        }else{
            b_cast_q = false;
        }

        num_board[to_row][to_column - 1] = 6;
        num_board[to_row][3] = 2;
        num_board[to_row][0] = 0;
        num_board[from_row][from_column - 1] = 0;

        fen_board[to_row][to_column - 1] = fen_board[from_row][from_column];
        fen_board[to_row][3] = fen_board[to_row][0];
        fen_board[to_row][0] = ' ';
        fen_board[from_row][from_column - 1] = ' ';

    }else if(_spec == "ennpass"){

        int move_cell = from_column - to_column;

        if(move_cell > 0){  //pohneme na levou stranu
        num_board[from_row][from_column - 1] = 0;
        num_board[to_row][to_column] = num_board[from_row][from_column];
        num_board[from_row][from_column] = 0;
        
        fen_board[from_row][from_column - 1] = ' ';
        fen_board[to_row][to_column] = fen_board[from_row][from_column];
        fen_board[from_row][from_column] = ' ';
        }else{      //pohneme na pravou stranu
        num_board[from_row][from_column + 1] = 0;
        num_board[to_row][to_column] = num_board[from_row][from_column];
        num_board[from_row][from_column] = 0;
        
        fen_board[from_row][from_column + 1] = ' ';
        fen_board[to_row][to_column] = fen_board[from_row][from_column];
        fen_board[from_row][from_column] = ' ';
        }

    }else if(_spec == "exchd"){
        num_board[from_row][from_column] = 0;
        num_board[to_row][to_column] = 5;

        fen_board[from_row][from_column] = ' ';
        if(on_move == 0){
            fen_board[to_row][to_column] = 'Q';
        }else{
            fen_board[to_row][to_column] = 'q';
        }
        
    }else if(_spec == "exchs"){
        num_board[from_row][from_column] = 0;
        num_board[to_row][to_column] = 4;

        fen_board[from_row][from_column] = ' ';
        if(on_move == 0){
            fen_board[to_row][to_column] = 'B';
        }else{
            fen_board[to_row][to_column] = 'b';
        }
    }else if(_spec == "exchk"){
        num_board[from_row][from_column] = 0;
        num_board[to_row][to_column] = 3;

        fen_board[from_row][from_column] = ' ';
        if(on_move == 0){
            fen_board[to_row][to_column] = 'N';
        }else{
            fen_board[to_row][to_column] = 'n';
        }
    }else if(_spec == "exchv"){
        num_board[from_row][from_column] = 0;
        num_board[to_row][to_column] = 2;

        fen_board[from_row][from_column] = ' ';
        if(on_move == 0){
            fen_board[to_row][to_column] = 'R';
        }else{
            fen_board[to_row][to_column] = 'r';
        }
    }

    last_from = from_row * 10 + from_column - 1;
    last_to = to_row * 10 + to_column - 1;
}

String ChessBoard::encodeFEN(){
    String fen;

    for(int i = 0; i < 8; i++){
        int blank_spaces = 0;
        for(int k = 0; k < 8; k++){

            if(fen_board[i][k] != ' '){
                fen += fen_board[i][k];
            }else{

                blank_spaces += 1;
                if(fen_board[i][k+1] != ' ' || k+1 == 8){
                    fen += blank_spaces;
                    blank_spaces = 0;
                }

            }

        }

        if(i != 7){
            fen += '/';
        }
    }

    fen += " ";

    if(on_move == 0){
        fen += "b";
    }else{
        fen += "w";
    }

    fen += " ";

    if(w_cast_k){
        fen += "K";
    }

    if(w_cast_q){
        fen += "Q";
    }

    if(b_cast_k){
        fen += "k";
    }

    if(b_cast_k){
        fen += "q";
    }

    if(!w_cast_k && !w_cast_q && !b_cast_k && !b_cast_q){
        fen += "-";
    }

    fen += " ";

    int from_column = last_from % 10;
    int from_row = (last_from - (last_from % 10)) / 10;

    int to_column = last_to % 10;
    int to_row = (last_to - (last_to % 10)) / 10;

    Serial.println(last_from);
    Serial.println(last_to);

    Serial.println(from_column);
    Serial.println(from_row);
    Serial.println(to_column);
    Serial.println(to_row);

    Serial.println(fen_board[to_row][to_column]);
    Serial.println(fen_board[to_row][to_column]);

    if((from_row == 1 || from_row == 6) && (fen_board[to_row][to_column] == 'p' || fen_board[to_row][to_column] == 'P')){
        int enn_column = to_column;
        int enn_row = to_row - 1;

        switch (enn_column)
        {
        case 0:
            fen += "a";
            break;
        case 1:
            fen += "b";
            break;
        case 2:
            fen += "c";
            break;
        case 3:
            fen += "d";
            break;
        case 4:
            fen += "e";
            break;
        case 5:
            fen += "f";
            break;
        case 6:
            fen += "g";
            break;
        case 7:
            fen += "h";
            break;
        
        default:
            break;
        }

        fen += enn_row;
    }else{
        fen += "-";
    }

    fen += " ";
    fen += "0";
    fen += " ";
    fen += "1";

    return fen;

}

void ChessBoard::decodeFEN(String _fen){

    w_cast_k = false;
    w_cast_q = false;
    b_cast_q = false;
    b_cast_k = false;

    int part = 0;
    int row = 0;
    int column = 0;

    char next_move;
    String ennpas = "";
    
    for(int i = 0; i < _fen.length(); i++){

        char cur_char = _fen.charAt(i);
        if(isDigit(cur_char) && part == 0){
            int blank_spaces = cur_char - '0';
            for(int k = 0; k < blank_spaces; k++){
                fen_board[row][column] = ' ';
                column++;
            }
        }else if(part == 0 && cur_char != '/' && cur_char != ' '){
            fen_board[row][column] = cur_char;
            column++;
        }else if(part == 0 && cur_char == '/'){
            column = 0;
            row++;
        }else if(cur_char == ' '){
            part++;
        }else if(part == 1){
            next_move = cur_char;
        }else if(part == 2){
            if(cur_char == 'K'){
                w_cast_k = true;
            }else if(cur_char == 'k'){
                b_cast_k = true;
            }else if(cur_char == 'Q'){
                w_cast_q = true;
            }else if(cur_char == 'q'){
                b_cast_q = true;
            }
        }else if(part == 3){
            if(cur_char != '-'){
                ennpas += cur_char;
            }
        }
        
    }

    for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
            Serial.print(fen_board[i][k]);
        }
        Serial.println();
    }

    /*Serial.println(next_move);
    Serial.println(ennpas);
    Serial.println(w_cast_k);
    Serial.println(b_cast_k);
    Serial.println(w_cast_q);
    Serial.println(b_cast_q);*/

}

void ChessBoard::getNextMove(String _fen){
    

    for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
            last_fen_board[i][k] = fen_board[i][k];
        }
    }

    startTask();

    decodeFEN(solve_my_fen(_fen));

    int _from = 0;
    int _to = 0;

    String str_from = last_best_move.substring(1, 3);
    String str_to = last_best_move.substring(4);

    String str_from_row= str_from.substring(1);
    String str_to_row = str_to.substring(1);

    _from = letterToInt(str_from.substring(0, 1)) + ( 7 - (str_from_row.toInt() - 1)) * 10;
    _to = letterToInt(str_to.substring(0, 1)) + ( 7 - (str_to_row.toInt() - 1)) * 10;

    last_from = _from;
    last_to = _to;

    updateNumBoard();

    Serial.println(str_from);
    Serial.println(str_to);
    Serial.println(_from);
    Serial.println(_to);

    stopTask();

}

int ChessBoard::letterToInt(String _lett){

    if(_lett == "a"){
        return 0;
    }else if(_lett == "b"){
        return 1;
    }else if(_lett == "c"){
        return 2;
    }else if(_lett == "d"){
        return 3;
    }else if(_lett == "e"){
        return 4;
    }else if(_lett == "f"){
        return 5;
    }else if(_lett == "g"){
        return 6;
    }else if(_lett == "h"){
        return 7;
    }

    return 0;

}

String ChessBoard::getLastMove(){
    return last_move;
}

void ChessBoard::updateNumBoard(){

    for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
            if(fen_board[i][k] == 'p' || fen_board[i][k] == 'P'){
                num_board[i][k] = 1;
            }else if(fen_board[i][k] == 'r' || fen_board[i][k] == 'R'){
                num_board[i][k] = 2;
            }else if(fen_board[i][k] == 'n' || fen_board[i][k] == 'N'){
                num_board[i][k] = 3;
            }else if(fen_board[i][k] == 'b' || fen_board[i][k] == 'B'){
                num_board[i][k] = 4;
            }else if(fen_board[i][k] == 'q' || fen_board[i][k] == 'Q'){
                num_board[i][k] = 5;
            }else if(fen_board[i][k] == 'k' || fen_board[i][k] == 'K'){
                num_board[i][k] = 6;
            }else{
                num_board[i][k] = 0;
            }
        }
    }

}

void ChessBoard::stopTask(){
    taskEnd();
}

void ChessBoard::startTask(){
    createTask();
}