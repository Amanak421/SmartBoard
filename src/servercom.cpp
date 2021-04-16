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

String ServerCom::httpGet(String _url){

    HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(_url);
  
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
