#include "servercom.h"

ServerCom::ServerCom(){

}

void ServerCom::begin(char* _ssid, char* _pass){
        ssid = _ssid;
    password = _pass;

    WiFi.begin(_ssid, _pass);
    Serial.println("Connecting");
     while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

String ServerCom::httpGetChessboard(){

  String url = "http://smartboard.mygamesonline.org/game-proceed/?com=get_last&id=" + String(game_id, DEC);

  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(url);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    Serial.println("Opakuji pozadavek...");
    httpGetChessboard();
  }
  // Free resources
  http.end();

  return payload;


}

void ServerCom::decodeChessstring(String _chessstring){
    for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
            act_piece_pos[i][k] = _chessstring.substring((k*3 + i*24), (k*3 + i*24)+3);
            Serial.print(act_piece_pos[i][k]);
        }
        Serial.println();
    }
}

String ServerCom::encodeChessstring(){
  String _chessstring = "";

    for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
            _chessstring += act_piece_pos[i][k];
        }
    }

    return _chessstring;
}

void ServerCom::encodeJson(String _json){

    DynamicJsonDocument doc(1024);

    deserializeJson(doc, _json);
    JsonObject obj = doc.as<JsonObject>();

    String _chessstring = obj["chessstring"];
    chessstring = _chessstring;
    String _last_move = obj["lastmove"];
    last_move = _last_move;
    String _spec_move = obj["specmove"];
    spec_move = _spec_move;

    Serial.print("Chessstring: ");
    Serial.println(chessstring);
    Serial.print("Last_move: ");
    Serial.println(last_move);
    Serial.print("SPEC_MOVE: ");
    Serial.println(spec_move);

}

String ServerCom::retLastMove(){
  return last_move;
}


void ServerCom::doMove(int _from, int _to){

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

  act_piece_pos[to_row][to_column - 1] = act_piece_pos[from_row][from_column - 1];
  act_piece_pos[from_row][from_column - 1] = "n_0";
}

void ServerCom::doSpecialMove(int _from, int _to, String _spec){

  _spec.trim();

  if(reverse){

    int _from_x = _from % 10;
    int _to_x = _to % 10;

    _from = (70 + _from_x*2) - _from;
    _to = (70 + _to_x*2) - _to;
  }

  Serial.print("FROM: ");
  Serial.println(_from);
  Serial.print("TO: ");
  Serial.println(_to);

  int from_column = _from % 10;
  int from_row = (_from - (_from % 10)) / 10;

  int to_column = _to % 10;
  int to_row = (_to - (_to % 10)) / 10;

  Serial.print("FROM_COLUMN ");
  Serial.println(from_column);
  Serial.print("FROM_ROW ");
  Serial.println(from_row);
  Serial.print("TO_COLUMN ");
  Serial.println(to_column);
  Serial.print("TO_ROW ");
  Serial.println(to_row);

  Serial.println(_spec);

  if(_spec == "castk"){
      act_piece_pos[to_row][to_column - 1] = player_color + "_6";
      act_piece_pos[to_row][5] = player_color + "_2";
      act_piece_pos[to_row][7] = "n_0";
      act_piece_pos[to_row][from_column - 1] = "n_0";
      Serial.println("Rosada na stranu krale...");
  }else if(_spec == "castq"){
      act_piece_pos[to_row][to_column - 1] = player_color + "_6";
      act_piece_pos[to_row][3] = player_color + "_2";
      act_piece_pos[to_row][0] = "n_0";
      act_piece_pos[from_row][from_column - 1] = "n_0";
      Serial.println("Rosada na stranu damy...");
  }else if(_spec == "ennpass"){
      int move_cell = from_column - to_column;

      if(move_cell > 0){  //pohneme na levou stranu
        act_piece_pos[from_row][from_column - 1] = "n_0";
        act_piece_pos[to_row][to_column] = act_piece_pos[from_row][from_column];
        act_piece_pos[from_row][from_column] = "n_0";
      }else{      //pohneme na pravou stranu
        act_piece_pos[from_row][from_column + 1] = "n_0";
        act_piece_pos[to_row][to_column] = act_piece_pos[from_row][from_column];
        act_piece_pos[from_row][from_column] = "n_0";
      }

      Serial.println("Ennpass...");

  }else if(_spec == "exchd"){
      act_piece_pos[from_row][from_column] = "n_0";
      act_piece_pos[to_row][to_column] = player_color + "_5";
      Serial.println("Vymena dama");
  }else if(_spec == "exchs"){
      act_piece_pos[from_row][from_column] = "n_0";
      act_piece_pos[to_row][to_column] = player_color + "_4";
      Serial.println("Vymena strelec");
  }else if(_spec == "exchk"){
      act_piece_pos[from_row][from_column] = "n_0";
      act_piece_pos[to_row][to_column] = player_color + "_3";
      Serial.println("Vymena kun");
  }else if(_spec == "exchv"){
      act_piece_pos[from_row][from_column] = "n_0";
      act_piece_pos[to_row][to_column] = player_color + "_2";
      Serial.println("Vymena vez");
  }
}

void ServerCom::setPlayerColor(String _color){
  player_color = _color;
}

void ServerCom::printBoard(){
  for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
            Serial.print(act_piece_pos[i][k]);
        }
        Serial.println();
    }
}

void ServerCom::httpSend(String _chessstring, String _domove, String _lastmove, String _specmove){

  String url = "http://smartboard.mygamesonline.org/game-proceed/?com=add&id=" + String(game_id, DEC) + "&chessstring=" +_chessstring+"&domove="+_domove+"&lastmove="+_lastmove+"&specmove="+_specmove;

  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(url);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    delay(300);
    httpSend(_chessstring, _domove, _lastmove, _specmove);
  }
  // Free resources
  http.end();


}

String ServerCom::httpGetLastOnMove(){
  String url = "http://smartboard.mygamesonline.org/game-proceed/?com=last_play&id=" + String(game_id, DEC);

  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(url);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    Serial.println("Opakuji pozadavek...");
    httpGetLastOnMove();
  }
  // Free resources
  http.end();

  DynamicJsonDocument doc(1024);

  deserializeJson(doc, payload);
  JsonObject obj = doc.as<JsonObject>();

  String _last_on_move = obj["lastplay"];
  last_on_move = _last_on_move;

    Serial.print("Last on move: ");
    Serial.println(last_on_move);

    if(last_on_move == nullptr){
      return "error";
    }

  return last_on_move;

}

void ServerCom::setGameId(int _id){
  game_id = _id;
}

void ServerCom::decodeChessBoard(){

    for(int i = 0; i < 8; i++){
      for(int k = 0; k < 8; k++){
        String _piece = act_piece_pos[i][k].substring(2);
        motor_move_board[i][k] = _piece.toInt();
        Serial.print(motor_move_board[i][k]);
      }
      Serial.println();
    }

    if(reverse){

      int helper_board[8][8] = {{0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0}};


      for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
          int _help = motor_move_board[i][k];
          int _curr_index = i*10 + k;
          int _new_index = (70 + k*2) - _curr_index;
          //Serial.print("Novy index: ");
          //Serial.println(_new_index);
          int _new_k = _new_index % 10;
          int _new_i = (_new_index - (_new_index % 10)) / 10;
          /*Serial.print("| ");
          Serial.print(_new_i);
          Serial.print(_new_k);*/
          helper_board[_new_i][_new_k] = _help;
        }
      }
      

      for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
          motor_move_board[i][k] = helper_board[i][k];
          Serial.print(helper_board[i][k]);
        }
        Serial.println();
      }

    }


}

void ServerCom::setReverse(bool _reverse){
  reverse = _reverse;
}

String ServerCom::retSpecMove(){
  return spec_move;
}