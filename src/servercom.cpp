#include "servercom.h"

ServerCom::ServerCom(){

}

void ServerCom::begin(const char* _ssid, const char* _pass){
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

    checkForEnnPass(last_move);

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
        act_piece_pos[from_row][from_column - 2] = "n_0";
        act_piece_pos[to_row][to_column - 1] = act_piece_pos[from_row][from_column - 1];
        act_piece_pos[from_row][from_column - 1] = "n_0";
      }else{      //pohneme na pravou stranu
        act_piece_pos[from_row][from_column] = "n_0";
        act_piece_pos[to_row][to_column - 1] = act_piece_pos[from_row][from_column - 1];
        act_piece_pos[from_row][from_column - 1] = "n_0";
      }

      Serial.println("Ennpass...");

  }else if(_spec == "exchd"){
      act_piece_pos[from_row][from_column - 1] = "n_0";
      act_piece_pos[to_row][to_column] = player_color + "_5";
      Serial.println("Vymena dama");
  }else if(_spec == "exchs"){
      act_piece_pos[from_row][from_column - 1] = "n_0";
      act_piece_pos[to_row][to_column] = player_color + "_4";
      Serial.println("Vymena strelec");
  }else if(_spec == "exchk"){
      act_piece_pos[from_row][from_column - 1] = "n_0";
      act_piece_pos[to_row][to_column] = player_color + "_3";
      Serial.println("Vymena kun");
  }else if(_spec == "exchv"){
      act_piece_pos[from_row][from_column - 1] = "n_0";
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

void ServerCom::scan() {
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(WiFi.SSID(i));
      delay(10);
	  ssids.push_back(WiFi.SSID(i));
    }
  }
  Serial.println("");

}

String ServerCom::encodeFEN(){

    strToFen();

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

    /*Serial.println(last_from);
    Serial.println(last_to);

    Serial.println(from_column);
    Serial.println(from_row);
    Serial.println(to_column);
    Serial.println(to_row);

    Serial.println(fen_board[to_row][to_column]);
    Serial.println(fen_board[to_row][to_column]);*/

    if((from_row == 1 || from_row == 6) && (fen_board[to_row][to_column] == 'p' || fen_board[to_row][to_column] == 'P') && (to_row == 3 || to_row == 4)){
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

    Serial.print("FEN ZE SERVERU: ");
    Serial.println(fen);

    return fen;

}

void ServerCom::strToFen(){   //převede chesstring na fen board
  for(int i = 0; i < 8; i++){
    for(int k = 0; k < 8; k++){
      if(act_piece_pos[i][k] == "w_1"){
        fen_board[i][k] = 'P';
      }else if(act_piece_pos[i][k] == "w_2"){
        fen_board[i][k] = 'R';
      }else if(act_piece_pos[i][k] == "w_3"){
        fen_board[i][k] = 'N';
      }else if(act_piece_pos[i][k] == "w_4"){
        fen_board[i][k] = 'B';
      }else if(act_piece_pos[i][k] == "w_5"){
        fen_board[i][k] = 'Q';
      }else if(act_piece_pos[i][k] == "w_6"){
        fen_board[i][k] = 'K';
      }else if(act_piece_pos[i][k] == "b_1"){
        fen_board[i][k] = 'p';
      }else if(act_piece_pos[i][k] == "b_2"){
        fen_board[i][k] = 'r';
      }else if(act_piece_pos[i][k] == "b_3"){
        fen_board[i][k] = 'n';
      }else if(act_piece_pos[i][k] == "b_4"){
        fen_board[i][k] = 'b';
      }else if(act_piece_pos[i][k] == "b_5"){
        fen_board[i][k] = 'q';
      }else if(act_piece_pos[i][k] == "b_6"){
        fen_board[i][k] = 'k';
      }else{
        fen_board[i][k] = ' ';
      }
    }
  }

  if(reverse){

      char helper_board[8][8];


      for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
          char _help = fen_board[i][k];
          int _curr_index = i*10 + k;
          int _new_index = (70 + k*2) - _curr_index;
          //Serial.print("Novy index: ");
          //Serial.println(_new_index);
          int _new_k = _new_index % 10;
          int _new_i = (_new_index - (_new_index % 10)) / 10;

          if(isLowerCase(_help)){
            _help = toUpperCase(_help);
          }else{
            _help = toLowerCase(_help);
          }

          /*Serial.print("| ");
          Serial.print(_new_i);
          Serial.print(_new_k);*/
          helper_board[_new_i][_new_k] = _help;
        }
      }
      
      Serial.println("PREVRACENA SACHOVNICE SERVER: ");
      for(int i = 0; i < 8; i++){
        for(int k = 0; k < 8; k++){
          fen_board[i][k] = helper_board[i][k];
          Serial.print(helper_board[i][k]);
        }
        Serial.println();
      }

  }
}

void ServerCom::updateLastMove(int _from, int _to){
    last_from = _from;
    last_to = _to;
}

void ServerCom::checkForEnnPass(String _last_move){
  String from_str = _last_move.substring(0, 2);
  int from = from_str.toInt();
  String to_str = _last_move.substring(3);
  int to = to_str.toInt();

  /*if(reverse){

        int from_x = from % 10;
        int to_x = to % 10;

        from = (70 + from_x*2) - from;
        to = (70 + to_x*2) - to;

  }*/

  int from_column = from % 10;
  int from_row = (from - from_column) / 10;

  int to_column = to % 10;
  int to_row = (to - to_column) / 10;

  Serial.println("CHECKING FOR ENNPASS");

  Serial.print("FROM | TO -> ");
  Serial.print(from);
  Serial.print(" | ");
  Serial.println(to);

  Serial.print("STRING: ");
  Serial.println(act_piece_pos[from_row][from_column]);

  if((act_piece_pos[from_row][from_column] == "b_1" && from_row == 1 && abs(to_row - from_row) == 2) || (act_piece_pos[from_row][from_column] == "w_1" && from_row == 6 && abs(to_row - from_row) == 2)){
    Serial.print("POVOLUJI ENN PASS NA POLICKO: ");
    String color = act_piece_pos[from_row][from_column].substring(0, 1);
    if(color == "w"){
      possible_enn_pass = (to_row + 1) * 10 + to_column;
      if(reverse){
        possible_enn_pass = (70 + (possible_enn_pass % 10 ) * 2) - possible_enn_pass;
      }
    }else if(color == "b"){
      possible_enn_pass = (to_row - 1) * 10 + to_column;
      if(reverse){
        possible_enn_pass = (70 + (possible_enn_pass % 10 ) * 2) - possible_enn_pass;
      }
    }else{
      Serial.println("BEZ BARVY");
    }
    Serial.println(possible_enn_pass);
  }else{
    possible_enn_pass = -1;
  }


}

int ServerCom::retEnnPassCell(){
  return possible_enn_pass;
}

void ServerCom::setCastle(char _color, char _site, bool _state){
  if(_color == 'w'){
    if(_site == 'k'){
      w_cast_k = _state;
    }else{
      w_cast_q = _state;
    }
  }else{
    if(_site == 'k'){
      b_cast_k = _state;
    }else{
      b_cast_q = _state;
    }
  }
}