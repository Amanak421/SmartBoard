#include "validate_move.h"

ValidateMove::ValidateMove(){
    calculateCellToEnd();
}

void ValidateMove::calculateCellToEnd(){      //spočítá pro každé políčko kolik políček zbýva na konec šachovnice pro každou stranu
    for(int row = 0; row < 8; row++){
        for(int column = 0; column < 8; column++){

            int top = 7 - row;
            int bottom = row;
            int left = column;
            int right = 7 - column;

            int cell = row * 8 + column;

            CellToEnd[cell][0] = top;
            CellToEnd[cell][1] = bottom;
            CellToEnd[cell][2] = left;
            CellToEnd[cell][3] = right;
            CellToEnd[cell][4] = min(top, right);
            CellToEnd[cell][5] = min(bottom, left);
            CellToEnd[cell][6] = min(top, left);
            CellToEnd[cell][7] = min(bottom, right);

        }
    }
}

void ValidateMove::generateMoves(){

    _moves.clear();
    //Serial.println(_moves.size());
    
    for(int row = 0; row < 8; row++){
        for(int column = 0; column < 8; column++){

            int corr_row = 7 - row;
            //Serial.println("test");

            char piece = board[corr_row][column];
            if(isSameColor(piece, on_move_color)){

                /*Serial.print(row);
                Serial.print(" | ");
                Serial.println(column);*/

                if(isLongRangePiece(piece)){
                    //Serial.println(piece);
                    generateLongRangeMoves((row * 8 + column), piece);
                }
                //Serial.println("test");

                if(isPieceType(piece, knight)){
                    //Serial.println(piece);
                    generateKnightMoves((row * 8 + column));
                }
                //Serial.println("test");

                if(isPieceType(piece, pawn)){
                    //Serial.println(piece);
                    generatePawnMoves((row * 8 + column), piece);
                }
                //Serial.println("test");

                if(isPieceType(piece, king)){
                    //Serial.println(piece);
                    generateKingMoves((row * 8 + column));
                }
                //Serial.println("test");

            }

        }
    }

}

void ValidateMove::generateLongRangeMoves(int _start, char _piece){

    int start_column = _start % 8;
    int start_row = 7 - ((_start - start_column) / 8);

    int start_index = (isPieceType(board[start_row][start_column], bishop)) ? 4 : 0;
    int end_index = (isPieceType(board[start_row][start_column], rock)) ? 4 : 8;

    for(int dir = start_index; dir < end_index; dir++){
        //Serial.println(CellToEnd[_start][dir]);
        for(int k = 0; k < CellToEnd[_start][dir]; k++){

            int endCell = _start + Directions[dir] * (k + 1);       //vypočítá nové políčko kam se může figurka pohnout

            int end_column = endCell % 8;           //převede id políčka na řádek a sloupek
            int end_row =  7 - ((endCell - end_column) / 8);

            char endCellPiece = board[end_row][end_column];      //zjistí figurku na posledním políčku

            if(isSameColor(endCellPiece, player_color)){        //pokud překáží figurka hráče, který hraje ukonči prohledávání v tomto směru
                //Serial.println("figurka hrace");
                break;
            }

            move_test move_add;
            move_add.startCell = _start;      //přidej pohyb do listu pohybů
            move_add.endCell = endCell;

            /*Serial.println(" pridavam pohyb");
            Serial.print(move_add.startCell);
            Serial.print("  ");
            Serial.println(move_add.endCell);*/
            _moves.push_back(move_add);

            if(isSameColor(endCellPiece, oponent_color)){       //pokud je v cestě figurka protihráče může se tm pohnout ale dál ne
                //Serial.println("figurka protihrace");
                break;
            }

        }
    }

}

void ValidateMove::generateKnightMoves(int _start){
    for(int dir = 0; dir < 8; dir++){

        int endCell = _start + KnightDirections[dir];

        if(endCell > 64 || endCell < 0){
            continue;
        }

        int end_column = endCell % 8;
        int start_column = _start % 8;
        if(end_column - start_column > 2){
            continue;
        }

        int _end_row = (endCell - end_column) / 8;
        int start_row = (_start - start_column) / 8;
        if(_end_row - start_row > 2){
            continue;
        }

        int end_row = 7 - _end_row;

        char endCellPiece = board[end_row][end_column];

        if(isSameColor(endCellPiece, player_color)){
            //Serial.println("figurka hrace");
            continue;
        }

        move_test move_add;
        move_add.startCell = _start;      //přidej pohyb do listu pohybů
        move_add.endCell = endCell;

        /*Serial.println(" pridavam pohyb");
        Serial.print(move_add->startCell);
        Serial.print("  ");
        Serial.println(move_add->endCell);*/
        _moves.push_back(move_add);

        if(isSameColor(endCellPiece, oponent_color)){       //pokud je v cestě figurka protihráče může se tm pohnout ale dál ne
            //Serial.println("figurka protihrace");
            continue;
        }

    }
}

void ValidateMove::generatePawnMoves(int _start, char _piece){

    int start_column = _start % 8;
    int start_row = ((_start - start_column) / 8);      // nepřevráceno, proto 1->6 a 6->1

    int start_index = (isSameColor(_piece, player_color)) ? 0 : 3;
    int end_index = (isSameColor(_piece, oponent_color)) ? 6 : 3;

    for(int dir = start_index; dir < end_index; dir++){

        if(((isSameColor(_piece, player_color) && start_row == 1) || (isSameColor(_piece, oponent_color) && start_row == 6)) && (dir == 0 || dir == 3)){
            //Serial.println("prosel jsem prvni podminkou");
            int endCell = _start + PawnDirections[dir] * 2;
            int end_column = endCell % 8;
            int end_row = 7 - ((endCell - end_column) / 8);
            if(board[end_row][end_column] == ' ' && ((isSameColor(_piece, player_color) && board[end_row + 1][end_column] == ' ') || (isSameColor(_piece, oponent_color) && board[end_row - 1][end_column] == ' '))){

            move_test move_add;
            move_add.startCell = _start;      //přidej pohyb do listu pohybů
            move_add.endCell = endCell;

            /*Serial.println(" pridavam pohyb");
            Serial.print(move_add->startCell);
            Serial.print("  ");
            Serial.println(move_add->endCell);*/
            _moves.push_back(move_add);

            }
        }

        int endCell = _start + PawnDirections[dir];
        int end_column = endCell % 8;
        //int _end_row = ((endCell - end_column) / 8);
        int end_row = 7 - ((endCell - end_column) / 8);

        if((dir == 0 || dir == 3) && board[end_row][end_column] != ' '){
            continue;   //sem muze jen kdyz je prázné políčko
        }

        if(abs(end_column - start_column) > 1){
            //Serial.println("Preslap pres radek");
            continue;
        }


        if(abs(end_column - start_column) == 1 && board[end_row][end_column] == ' ' && endCell != possibleEnnPassCell){
            //Serial.println("Sem mizes jen kdyz utocis");
            continue;
        }

        int endCellPiece = board[end_row][end_column];

        if(isSameColor(endCellPiece, player_color)){
            //Serial.println("figurka hrace");
            continue;
        }

        move_test move_add;
        move_add.startCell = _start;      //přidej pohyb do listu pohybů
        move_add.endCell = endCell;

        /*Serial.println(" pridavam pohyb");
        Serial.print(move_add->startCell);
        Serial.print("  ");
        Serial.println(move_add->endCell);*/
        _moves.push_back(move_add);

        if(isSameColor(endCellPiece, oponent_color)){       //pokud je v cestě figurka protihráče může se tm pohnout ale dál ne
            //Serial.println("figurka protihrace");
            continue;
        }

    }

}

void ValidateMove::generateKingMoves(int _start){

    for(int dir = 0; dir < 8; dir++){

        int start_column = _start % 8;
        int start_row = ((_start - start_column) / 8);

        int endCell = _start + Directions[dir];
        int end_column = endCell % 8;
        int _end_row = ((endCell - end_column) / 8);
        int end_row = 7 - ((endCell - end_column) / 8);

        if(endCell < 0 || endCell > 64){
            continue;
        }

        if(canCastle(on_move_color, kings) && dir == 3){
            move_test move_add;
            move_add.startCell = _start;      //přidej pohyb do listu pohybů
            move_add.endCell = endCell + 1;

            /*Serial.println(" pridavam pohyb");
            Serial.print(move_add->startCell);
            Serial.print("  ");
            Serial.println(move_add->endCell);*/
            _moves.push_back(move_add);
        }else if(canCastle(on_move_color, queens)&& dir == 2){
            move_test move_add;
            move_add.startCell = _start;      //přidej pohyb do listu pohybů
            move_add.endCell = endCell - 1;

            /*Serial.println(" pridavam pohyb");
            Serial.print(move_add->startCell);
            Serial.print("  ");
            Serial.println(move_add->endCell);*/
            _moves.push_back(move_add);
        }

        if(_end_row - start_row > 1){
            continue;
        }

        if(end_column - start_column > 2){
            continue;
        }

        int endCellPiece = board[end_row][end_column];

        if(isSameColor(endCellPiece, player_color)){
            //Serial.println("figurka hrace");
            continue;
        }

        move_test move_add;
            move_add.startCell = _start;      //přidej pohyb do listu pohybů
            move_add.endCell = endCell;

            /*Serial.println(" pridavam pohyb");
            Serial.print(move_add->startCell);
            Serial.print("  ");
            Serial.println(move_add->endCell);*/
            _moves.push_back(move_add);

        if(isSameColor(endCellPiece, oponent_color)){       //pokud je v cestě figurka protihráče může se tm pohnout ale dál ne
            //Serial.println("figurka protihrace");
            continue;
        }

    }

}

void ValidateMove::generateLegalMoves(){

    pseudo_legal_moves.clear();

    Serial.println("Generuji pseudolegalni tahy...");

    generatePseudLegalMoves();

    Serial.print("Pseudo legalni tahy: ");
    for(int i = 0; i < pseudo_legal_moves.size(); i++){
        move_test _move = pseudo_legal_moves.at(i);
        Serial.print(_move.startCell);
        Serial.print(" | ");
        Serial.println(_move.endCell);
    }
    

    for(int i = 0; i < pseudo_legal_moves.size(); i++){


        move_test m_move = pseudo_legal_moves[i];

        /*Serial.print("Provadim ");
        Serial.print(i);
        Serial.print(" pseudo tah: ");
        Serial.print(m_move.startCell);
        Serial.print(" | ");
        Serial.println(m_move.endCell);*/
        makeMove(m_move);

        //Serial.println("Generuji odpovedi na tento tah");
 

        generateMoves();

        /*Serial.print("Tahy oponenta: ");
        for(int i = 0; i < _moves.size(); i++){
            move_test _move = _moves.at(i);
            Serial.print(_move.startCell);
            Serial.print(" | ");
            Serial.println(_move.endCell);
        }*/

        int fails = 0;

        //Serial.println("Zkoumam zda tah byl legalni");
        for(int k = 0; k < _moves.size(); k++){
            move_test opp_move = _moves[k];
            if(opp_move.endCell == myKingSquare()){
                //ale jsem v matu -> můj poslední tah musel být nelegání
                fails += 1;
            }
        }

        if(fails == 0){
            //Serial.println("Tah byl pridan");
            legal_moves.push_back(m_move);
        }

        //Serial.println("vracim tah");
        unmakeMove(m_move);

    }

    Serial.print("Pseudo legalni tahy: ");
    for(int i = 0; i < pseudo_legal_moves.size(); i++){
        move_test _move = pseudo_legal_moves.at(i);
        Serial.print(_move.startCell);
        Serial.print(" | ");
        Serial.println(_move.endCell);
    }

}

void ValidateMove::generatePseudLegalMoves(){
    generateMoves();
    pseudo_legal_moves = _moves;
    _moves.clear();
}

void ValidateMove::makeMove(move_test _move){

    /*Serial.print(_move.startCell);
    Serial.print("Pocatecni sloupec / radek");*/

    //int start = _move->startCell;
    int start_column = _move.startCell % 8;
    //Serial.print(start_column);
    int start_row = 7 - ((_move.startCell - start_column) / 8);
    //Serial.print(" / ");
    //Serial.println(start_row);
    char startPiece = board[start_row][start_column];

    /*Serial.print(_move.startCell);
    Serial.print("Koncovy sloupec / radek");*/

    int end = _move.endCell;
    int end_column = _move.endCell % 8;
    //Serial.print(end_column);
    int end_row = 7 - ((_move.endCell - end_column) / 8);
    //Serial.print(" / ");
    //Serial.println(end_row);
    char endPiece = board[end_row][end_column];

    //Serial.println("test");

    //ennpass

    if(isPieceType(startPiece, pawn) && abs(end_column - start_column) == 1 && end == possibleEnnPassCell){
        if(on_move_color == 'w'){
            board[end_row][end_column] = board[start_row][start_column];
            board[start_row][start_column] = ' ';
            board[end_row - 1][end_column] = ' ';

            if(on_move_color == 'w'){
                on_move_color = 'b';
                player_color = 'b';
                oponent_color = 'w';
            }else{
                on_move_color = 'w';
                player_color = 'w';
                oponent_color = 'b';
            }

            return;
        }else{
            board[end_row][end_column] = board[start_row][start_column];
            board[start_row][start_column] = ' ';
            board[end_row + 1][end_column] = ' ';
            
            if(on_move_color == 'w'){
                on_move_color = 'b';
                player_color = 'b';
                oponent_color = 'w';
            }else{
                on_move_color = 'w';
                player_color = 'w';
                oponent_color = 'b';
            }
            
            return;
        }
    }

    //rosada

    //Serial.println("test");

    if(isPieceType(startPiece, king) && abs(end_column - start_column) == 2 && canCastle(on_move_color, kings)){

        board[end_row][end_column] = board[start_row][start_column];
        board[end_row][end_column - 1] = pieceID(on_move_color, rock);
        board[end_row][7] = ' ';
        board[start_row][start_column] = ' ';

        if(on_move_color == 'w'){
            on_move_color = 'b';
            player_color = 'b';
            oponent_color = 'w';
        }else{
            on_move_color = 'w';
            player_color = 'w';
            oponent_color = 'b';
        }

        return;

    }else if(isPieceType(startPiece, king) && abs(end_column - start_column) == 2 && canCastle(on_move_color, queens)){
        board[end_row][end_column] = board[start_row][start_column];
        board[end_row][end_column + 1] = pieceID(on_move_color, rock);
        board[end_row][0] = ' ';
        board[start_row][start_column] = ' ';

        if(on_move_color == 'w'){
            on_move_color = 'b';
            player_color = 'b';
            oponent_color = 'w';
        }else{
            on_move_color = 'w';
            player_color = 'w';
            oponent_color = 'b';
        }

        return;
    }

    if(endPiece != ' '){
        last_piece_out = endPiece;
    }else{
        last_piece_out = ' ';
    }


    //Serial.println("test");
     //provede klasický tah

    board[end_row][end_column] = board[start_row][start_column];
    board[start_row][start_column] = ' ';

    if(on_move_color == 'w'){
        on_move_color = 'b';
        player_color = 'b';
        oponent_color = 'w';
    }else{
        on_move_color = 'w';
        player_color = 'w';
        oponent_color = 'b';
    }

}

void ValidateMove::unmakeMove(move_test _move){

    //int start = _move->startCell;
    int start_column = _move.startCell % 8;
    int start_row = 7 - ((_move.startCell - start_column) / 8);
    int startPiece = board[start_row][start_column];

    int end = _move.endCell;
    int end_column = _move.endCell % 8;
    int end_row = 7 - ((_move.endCell - end_column) / 8);
    //int endPiece = board[end_row][end_column];

    //ennpass

    if(isPieceType(startPiece, pawn) && abs(end_column - start_column) == 1 && end == possibleEnnPassCell){
        if(on_move_color == 'w'){
            board[start_row][start_column] = board[end_row][end_column];
            board[end_row][end_column] = ' ';
            board[end_row - 1][end_column] = pieceID(oponent_color, pawn);

            if(on_move_color == 'w'){
                on_move_color = 'b';
                player_color = 'b';
                oponent_color = 'w';
            }else{
                on_move_color = 'w';
                player_color = 'w';
                oponent_color = 'b';
            }

            return;
        }else{
            board[start_row][start_column] = board[end_row][end_column];
            board[end_row][end_column] = ' ';
            board[end_row - 1][end_column] = pieceID(oponent_color, pawn);

            if(on_move_color == 'w'){
                on_move_color = 'b';
                player_color = 'b';
                oponent_color = 'w';
            }else{
                on_move_color = 'w';
                player_color = 'w';
                oponent_color = 'b';
            }

            return;
        }
    }

    //rosada

    if(isPieceType(startPiece, king) && abs(end_column - start_column) == 2 && canCastle(on_move_color, kings)){

        board[start_row][start_column] = board[end_row][end_column];
        board[end_row][end_column - 1] = ' ';
        board[end_row][7] = pieceID(on_move_color, rock);
        board[end_row][end_column] = ' ';

        if(on_move_color == 'w'){
            on_move_color = 'b';
            player_color = 'b';
            oponent_color = 'w';
        }else{
            on_move_color = 'w';
            player_color = 'w';
            oponent_color = 'b';
        }

        return;

    }else if(isPieceType(startPiece, king) && abs(end_column - start_column) == 2 && canCastle(on_move_color, queens)){
        board[start_row][start_column] = board[end_row][end_column];
        board[end_row][end_column + 1] = ' ';
        board[end_row][0] = pieceID(on_move_color, rock);
        board[end_row][end_column] = ' ';

        if(on_move_color == 'w'){
            on_move_color = 'b';
            player_color = 'b';
            oponent_color = 'w';
        }else{
            on_move_color = 'w';
            player_color = 'w';
            oponent_color = 'b';
        }

        return;
    }


    //vrátí klasický tah

    board[start_row][start_column] = board[end_row][end_column];
    board[end_row][end_column] = ' ';
    if(last_piece_out != ' '){
        board[end_row][end_column] = last_piece_out;
        last_piece_out = ' ';
    }

    if(on_move_color == 'w'){
        on_move_color = 'b';
        player_color = 'b';
        oponent_color = 'w';
    }else{
        on_move_color = 'w';
        player_color = 'w';
        oponent_color = 'b';
    }

}

char ValidateMove::pieceID(char _color, Piece _piece){
    if(_piece == rock){
        if(_color == 'w'){
            return 'R';
        }else{
            return 'r';
        }
    }else if(_piece == pawn){
        if(_color == 'w'){
            return 'P';
        }else{
            return 'p';
        }
    }

    return ' ';
}

bool ValidateMove::isSameColor(char _piece, char _color){
    
    if(_piece == ' '){
        return false;
    }

    if(isUpperCase(_piece) && _color == 'w'){
        return true;
    }else if(!isUpperCase(_piece) && _color == 'b'){
        return true;
    }

    return false;

}

bool ValidateMove::isLongRangePiece(char _piece){

    if(_piece == 'r' || _piece == 'R' || _piece == 'b' || _piece == 'B' || _piece == 'q' || _piece == 'Q'){
        return true;
    }

    return false;

}
bool ValidateMove::isKnight(char _piece){

    if(_piece == 'n' || _piece == 'N'){
        return true;
    }

    return false;

}
bool ValidateMove::isKing(char _piece){

    if(_piece == 'k' || _piece == 'K'){
        return true;
    }

    return false;

}
bool ValidateMove::isPawn(char _piece){

    if(_piece == 'p' || _piece == 'P'){
        return true;
    }

    return false;

}

bool ValidateMove::isPieceType(char _piece, Piece _piece_type){

    char _other_piece = ' ';

    if(_piece_type == pawn){
        _other_piece = 'p';
    }else if(_piece_type == rock){
        _other_piece = 'r';
    }else if(_piece_type == bishop){
        _other_piece = 'b';
    }else if(_piece_type == knight){
        _other_piece = 'n';
    }else if(_piece_type == queen){
        _other_piece = 'q';
    }else if(_piece_type == king){
        _other_piece = 'k';
    }

    _piece = toLowerCase(_piece);

    if(_other_piece == _piece){
        return true;
    }

    return false;

}

void ValidateMove::showPossibleMoves(int _cell){

    generateLegalMoves();

    //generateMoves();

    std::vector<move_test> my_moves;


    Serial.print("_moves: ");
    Serial.println(legal_moves.size());

    for(int i = 0 ; i < legal_moves.size(); i++){

        move_test cur_move;
        cur_move = legal_moves[i];
        Serial.print(cur_move.startCell);
        Serial.print(" | ");
        Serial.println(cur_move.endCell);
        if(cur_move.startCell == _cell){
            Serial.println("pridavam odpovidajici pohyb");
            my_moves.push_back(cur_move);
        }
    }

    String board_f_print[8][8];

    for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
            board_f_print[i][k] = board[i][k];
        }
    }

    Serial.print("my_moves: ");
    Serial.println(my_moves.size());
    for(int i = 0; i < my_moves.size(); i++){
        move_test _move = my_moves[i];
        int end_column  = _move.endCell % 8;
        int end_row =  7 - ((_move.endCell - end_column) / 8);
        if(board_f_print[end_row][end_column] == " "){
            board_f_print[end_row][end_column] = 'o';
        }else{
            board_f_print[end_row][end_column] = '-' + board_f_print[end_row][end_column];
        }
    }

    for(int i = 0; i < 8; i++){
        Serial.print("----");
    }

    Serial.println();

    for(int i = 0; i < 8; i++){
        Serial.print("| ");
        for(int k = 0; k < 8; k++){
            Serial.print(board_f_print[i][k]);
            Serial.print(" | ");
        }
        Serial.println();
        for(int i = 0; i < 8; i++){
            Serial.print("----");
        }
        Serial.println();
    }

}

bool ValidateMove::canCastle(char _color, Site _site){
    if(_color == 'w'){
        if(_site == kings){
            if(board[7][5] == ' ' && board[7][6] == ' ' && w_castk){
                return true;
            }
        }else{
            if(board[7][1] == ' ' && board[7][2] == ' ' && board[7][3] == ' ' && w_castq){
                return true;
            }
        }
    }else{
        if(_site == kings){
            if(board[0][5] == ' ' && board[0][6] == ' ' && b_castk){
                return true;
            }
        }else{
            if(board[0][1] == ' ' && board[0][2] == ' ' && board[0][3] == ' ' && b_castq){
                return true;
            }
        }
    }
    return false;
}

void ValidateMove::MakeMoveTest(int _start, int _end){
    move_test _move;
    _move.startCell = _start;
    _move.endCell = _end;
    makeMove(_move);
}
void ValidateMove::UnMakeMove(int _start, int _end){
    move_test _move;
    _move.startCell = _start;
    _move.endCell = _end;
    unmakeMove(_move);
}

int ValidateMove::myKingSquare(){

    for(int i = 0; i < 8; i++){
        int corr_i = 7 - i;
        for (int k = 0; k < 8; k++){
            if(board[corr_i][k] == 'K'){
                return (i * 8 + k);
            }
        }
        
    }

    return -1;
}

bool ValidateMove::validateMove(int _start, int _end){
    std::vector<move_test> poss_moves;

    generateLegalMoves();

    int start_column = _start % 10 - 1;
    int start_row = 7 - (_start - start_column) / 10;

    int start_cell = start_row * 8 + start_column;

    int end_column = _end % 10 - 1;
    int end_row = 7 - (_end  - end_column) / 10;

    int end_cell = end_row * 8 + end_column;

    Serial.print("sloupek | radek");
    Serial.print(start_cell);
    Serial.print(" | ");
    Serial.println(end_cell);

    for(int i = 0; i < legal_moves.size(); i++){
        move_test _move = legal_moves[i];
        if(_move.startCell == start_cell && _move.endCell == end_cell){
            return true;
        }
    }

    return false;

}

void ValidateMove::updateBoard(String _fen){
    /*w_castk = false;
    w_castq = false;
    b_castq = false;
    b_castk = false;*/

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
                board[row][column] = ' ';
                column++;
            }
        }else if(part == 0 && cur_char != '/' && cur_char != ' '){
            board[row][column] = cur_char;
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
                w_castk = true;
            }else if(cur_char == 'k'){
                b_castk = true;
            }else if(cur_char == 'Q'){
                w_castq = true;
            }else if(cur_char == 'q'){
                b_castq = true;
            }
        }else if(part == 3){
            if(cur_char != '-'){
                ennpas += cur_char;
            }
        }
        
    }

    for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
            Serial.print(board[i][k]);
        }
        Serial.println();
    }

}