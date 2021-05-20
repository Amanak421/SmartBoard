#ifndef __SERVERCOM_H__
#define __DERVERVOM_H__

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


class ServerCom{


private:
    String ssid = "";
    String password = "";

    
    String last_move;
    String spec_move;

    String last_on_move;

    String player_color = "b";

    int game_id;

    String act_piece_pos[8][8];

    char fen_board[8][8];

    bool reverse = false;

    void strToFen();

    int on_move = 1;

    bool w_cast_k = true;
    bool w_cast_q = true;
    bool b_cast_k = true;
    bool b_cast_q = true;

    int last_from = -1;
    int last_to = -1;

    


public:
    std::vector<String> ssids;
    String chessstring;
    
    ServerCom();

    void begin(const char* _ssid, const char* _pass);

    String httpGetChessboard();

    String httpGetLastOnMove();

    void decodeChessstring(String _chessstring);
    String encodeChessstring();

    void decodeLastMove(String _last_move);

    void encodeJson(String _json);

    String retLastMove();
    String retSpecMove();

    void doMove(int _from, int _to);
    void doSpecialMove(int _from, int _to, String _spec);

    void httpSend(String _chessstring, String _domove, String _lastmove, String _specmove);

    void printBoard();

    void setGameId(int _id);

    void setReverse(bool _reverse);
    void setPlayerColor(String _color);

    int motor_move_board[8][8];
    void decodeChessBoard();

    void scan();

    String encodeFEN();

    void updateLastMove(int _from, int _to);
};


#endif