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

    int game_id;

    String act_piece_pos[8][8];


public:
    String chessstring;
    ServerCom();

    void begin(char* _ssid, char* _pass);

    String httpGetChessboard();

    String httpGetLastOnMove();

    void decodeChessstring(String _chessstring);
    String encodeChessstring();

    void decodeLastMove(String _last_move);

    void encodeJson(String _json);

    String retLastMove();

    void doMove(int _from, int _to);

    void httpSend(String _chessstring, String _domove, String _lastmove, String _specmove);

    void printBoard();

    void setGameId(int _id);







};


#endif