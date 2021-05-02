#ifndef __SERVERCOM_H__
#define __DERVERVOM_H__

#include <Arduino.h>
#include <vector>


class ValidateMove {

private:

    const int Directions[8] = {8, -8, -1, 1, 9, -9, 7, -7};     //možné směry posunu na šachovnici
    const int KnightDirections[8] = {6, 15, 17, 10, -6, -15, -17, -10};     //možnosti tahu s koněm
    const int PawnDirections[6] = {8, 9, 7, -8, -9, -7};
    int CellToEnd[64][8];

    struct move {
        int startCell;
        int endCell;
    };

    class move_test{

        public:
            int startCell;
            int endCell;
    };

    enum Piece {
        pawn,
        rock,
        bishop,
        knight,
        queen,
        king
    };

    enum Site {
        kings,
        queens,
    };

    char on_move_color = 'w';

    char player_color = 'w';
    char oponent_color = 'b';

    int myKingSquare();
    int opponent_king_square = 60;

    int possibleEnnPassCell = -1;

    bool w_castk = true;
    bool w_castq = true;
    bool b_castk = true;
    bool b_castq = true;

    char board[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
    };


    std::vector<ValidateMove::move_test> _moves;    //uložení vektoru tahů z posledního vygenerování

    std::vector<ValidateMove::move_test> legal_moves;       //tahy, které je možné provést
    std::vector<ValidateMove::move_test> pseudo_legal_moves;    //slouží pro vnitřní potřeby fce generateLegalMoves

    char last_piece_out = ' ';

    void generatePseudLegalMoves();

    void calculateCellToEnd();

    bool isSameColor(char _piece, char _color);
    bool isLongRangePiece(char _piece);
    bool isKnight(char _piece);
    bool isKing(char _piece);
    bool isPawn(char _piece);

    bool isPieceType(char _piece, Piece _piece_type);


    void generateMoves();
    void generateLegalMoves();

    void generateLongRangeMoves(int _start, char _piece);
    void generateKnightMoves(int _start);
    void generatePawnMoves(int _start, char _piece);
    void generateKingMoves(int _start);

    bool canCastle(char _color, Site _site);

    char pieceID(char _color, Piece _piece);

    void makeMove(move_test _move);
    void unmakeMove(move_test _move);


public:

    ValidateMove();

    void setPlayerColor();

    void showPossibleMoves(int _cell);      //ukáže možnosti posunu pro figurku na určitém políčku, id políčka od 0 - 63

    void MakeMoveTest(int _start, int _end);
    void UnMakeMove(int _start, int _end);

    bool validateMove(int _start, int _end); //id políček jsou v klasickém 00 - 77

    void updateBoard(String _fen);


};


#endif