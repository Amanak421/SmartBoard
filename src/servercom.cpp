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
    int from_column = _from % 10;
    int from_row = (_from - (_from % 10)) / 10;

    int to_column = _to % 10;
    int to_row = (_to - (_to % 10)) / 10;

    act_piece_pos[to_row][to_column - 1] = act_piece_pos[from_row][from_column - 1];
    act_piece_pos[from_row][from_column - 1] = "n_0";
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

    return last_on_move;

}

void ServerCom::setGameId(int _id){
  game_id = _id;
}