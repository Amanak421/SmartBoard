#include "piece_detect.h"

PCF8574 data(0x20);
//PCF8574 rows(0x24);


PieceDetect::PieceDetect(){
    pinMode(DATA_IN, INPUT);        //nastaví datov pin na vstup

    /*for(int i = 0; i < 3; i++){
        pinMode(MUX_PINS[i], OUTPUT);
    }*/

    for(int i = 0; i < 8; i++){
    data.pinMode(DATA_PINS[i], INPUT_PULLUP);
    }
    
    data.begin();

    for(int i = 0; i < 8; i++){
      pinMode(ROW_PINS[i], OUTPUT);
      digitalWrite(ROW_PINS[i], HIGH);
    }

    for(int i = 0; i < 8; i++){
      for(int k = 0; k < 8; k++){
        last_board[i][k] = start_board[i][k];
      }
    }


    /*for(int i = 0; i < 3; i++){
        pinMode(DEC_PINS[i], OUTPUT);
    }*/

    //selectRow(0);   //vybere 1. řádek
}

void PieceDetect::selectRow(int _id){
  for (int bits = 3; bits > -1; bits--) {
    // Compare bits 7-0 in byte
    if (ROW_ID[_id] & (1 << bits)) {
      digitalWrite(DEC_PINS[bits], HIGH);
    }
    else {
      digitalWrite(DEC_PINS[bits], LOW);
    }
  }
}

int PieceDetect::readMUX(int channel){

  int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6 použití 8 místo 6 -> nefunguje
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  //loop through the 4 sig
  //Serial.println("Channel: ");
  for(int i = 0; i < 3; i ++){
    //Serial.print(muxChannel[channel][i]);
    digitalWrite(MUX_PINS[i], muxChannel[channel][i]);
  }
  //Serial.println();

  //read the value at the SIG pin
  int val = digitalRead(DATA_IN);

  //return the value
  return val;
}

bool PieceDetect::checkBoard(){
  int fails = 0;
  for(int i = 0; i < ROWS; i++){
    pickRow(i);
    delay(50);
    for(int k = 0; k < COLUMNS; k++){
      int value = readValue(k);
      last_board[i][k] = value;
      if((fen_board[i][k] != ' ' && value != 0) || (fen_board[i][k] == ' ' && value != 1)){
        fails += 1;
      }
    }
    delay(5);
  }

  if(fails == 0){
    return true;
  }

  return false;

}

void PieceDetect::findLastMove(){
  for(int i = 0; i < 8; i++){
    for(int k = 0; k < 8; k++){
      if(fen_board[i][k] != ' ' && last_board[i][k] == 1){

          if((isUpperCase(fen_board[i][k]) && player_color != 0) || (isLowerCase(fen_board[i][k]) && player_color != 1)){
            Serial.println("Vyhozeni figurky....");
            /*int _piece_out = i * 10 + k;
            int out_column = _piece_out % 10;
            int our_row = (_piece_out - out_column) / 10;*/
            fen_board[i][k] = ' ';
            last_from = -1;
          }else{
            last_from = i * 10 + k;
            Serial.print("Figurka odebrana z sachovnice: ");
            Serial.print(fen_board[i][k]);
            Serial.print(" ");
            Serial.println(last_from);
          }
        
      }else if(fen_board[i][k] == ' ' && last_board[i][k] == 0 && last_from > -1){
        last_to = i * 10 + k;
        if((last_from == 74 || last_from == 4) && (last_to == 76 || last_to == 6) && !cast_k_flag && w_cast_k){
            cast_k_flag = true;
            cast_from = last_from;
            cast_to = last_to;
            Serial.println("ROSADA::::");
        }else if(cast_k_flag && ((last_from == 77 || last_from == 7) && (last_to == 75 || last_to == 5))){
            last_from = cast_from;
            last_to = cast_to;
            move_completed = true;
            cast_k_flag = false;
            Serial.println("ROSADA::::DOKONCENA");
        }else if((last_from == 74 || last_from == 4) && (last_to == 72 || last_to == 2)&& !cast_q_flag && w_cast_q){
            cast_q_flag = true;
            cast_from = last_from;
            cast_to = last_to;
            Serial.println("ROSADA");
        }else if(cast_q_flag && ((last_from == 70 || last_from == 0) && (last_to == 73 || last_to == 3))){
            last_from = cast_from;
            last_to = cast_to;
            move_completed = true;
            cast_q_flag = false;
            Serial.println("ROSADA::::DOKONCENA");
        }else{
            Serial.print("Figurka dana na sachovnici: ");
            Serial.print(fen_board[i][k]);
            Serial.print(" ");
            Serial.println(last_to);
            move_completed = true;
        }
      }
    }
  }

  if(last_from > -1 && last_to > -1 && move_completed){
    int from_column = last_from % 10;
    int from_row = (last_from - from_column) / 10;

    int to_column = last_to % 10;
    int to_row = (last_to - to_column) / 10;
    Serial.print("Last from: ");
    Serial.println(last_from);
    Serial.print("Last to: ");
    Serial.println(last_to);

    fen_board[to_row][to_column] = fen_board[from_row][from_column];
    fen_board[from_row][from_column] = ' ';
    last_from = -1;
    last_to = -1;
    move_completed = false;

    for(int i = 0; i < 8; i++){
      Serial.println("---------------------------------------------");
      for(int k = 0; k < 8; k++){
        Serial.print(" | ");
        Serial.print(fen_board[i][k]);

      }
      Serial.print(" | ");
      Serial.println();
    }
  }

}

void PieceDetect::printPieceBoard(){

  /*for(int i = 0; i < ROWS; i++){
    selectRow(0);
    selectRow(i);
    for(int k = 0; k < COLUMNS; k++){
      delay(25);
      Serial.print(readValue(k));
      Serial.print(" ");
    }
    Serial.println();
    //Serial.println(readMUX(i));
    delay(50);
  }*/

  pickRow(0);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      Serial.print(readValue(k));
      Serial.print(" ");
  }
  Serial.println();
  pickRow(1);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      Serial.print(readValue(k));
      Serial.print(" ");
  }
  Serial.println();
  pickRow(2);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      Serial.print(readValue(k));
      Serial.print(" ");
  }
  Serial.println();
  pickRow(3);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      Serial.print(readValue(k));
      Serial.print(" ");
  }
  Serial.println();
  pickRow(4);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      Serial.print(readValue(k));
      Serial.print(" ");
  }
  Serial.println();
  pickRow(5);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      Serial.print(readValue(k));
      Serial.print(" ");
  }
  Serial.println();
  pickRow(6);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      Serial.print(readValue(k));
      Serial.print(" ");
  }

  Serial.println();
  pickRow(7);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      Serial.print(readValue(k));
      Serial.print(" ");
  }

  Serial.println();
  Serial.println("-----------------------------------");

  /*rows.digitalWrite(ROW_PINS[0], HIGH);
  delay(500);
  rows.digitalWrite(ROW_PINS[0], LOW);*/
  delay(2500);
}

bool PieceDetect::checkMove(){
    /*if(!checkBoard()){
        findLastMove();
        if(last_from > -1 && last_to > -1){
          return true;
        }
    }

    return false;
    delay(READ_DELAY);*/

    scanBoard();
    checkDiff();
}

int PieceDetect::readValue(int _column){
    return data.digitalRead(DATA_PINS[_column]);
}

void PieceDetect::pickRow(int _row){
  for(int i = 0; i < 8; i++){
    digitalWrite(ROW_PINS[i], LOW);
  }
  digitalWrite(ROW_PINS[_row], HIGH);

}

void PieceDetect::scanBoard(){
  pickRow(0);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      temp_board[0][k] = readValue(k);
  }

  pickRow(1);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      temp_board[1][k] = readValue(k);
  }

  pickRow(2);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      temp_board[2][k] = readValue(k);
  }

  pickRow(3);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      temp_board[3][k] = readValue(k);
  }

  pickRow(4);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      temp_board[4][k] = readValue(k);
  }

  pickRow(5);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      temp_board[5][k] = readValue(k);
  }

  pickRow(6);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      temp_board[6][k] = readValue(k);
  }

  pickRow(7);
  delay(50);
  for(int k = 0; k < COLUMNS; k++){
      delay(1);
      temp_board[7][k] = readValue(k);
  }
  delay(200);
}

int PieceDetect::checkDiff(){
  for(int i = 0; i < 8; i++){
    for(int k = 0; k < 8; k++){

      if(temp_board[i][k] != last_board[i][k]){

        if(temp_board[i][k] == 1 && last_board[i][k] == 0){
          char piece_on_cell = fen_board[i][k];
          if(isLowerCase(piece_on_cell)){
            Serial.println("Toto se nepocita -> figurka vyhozena -> probiha kontrola...");
            if(act_confirm_scan < confirm_scans){
              Serial.println("KONTROLA");
              act_confirm_scan += 1;
              delay(100);
              scanBoard();
              checkDiff();
            }else{
              Serial.print("Potvrzene vyhozeni figurky: ");
              Serial.println(i * 10 + k);
              last_board[i][k] = 1;
              act_confirm_scan = 0;
            }
            
          }else{
            last_from = i * 10 + k;
            Serial.print("Figurka zmizela z: ");
            Serial.println(last_from);
            move_procc = true;
          }
          
        }else if(move_procc && temp_board[i][k] == 0 && last_board[i][k] == 1){
          last_to = i * 10 + k;
          move_procc = false;
          move_completed = true;
        }

      }

    }
  }
}

void PieceDetect::finishMove(){
  for(int i = 0; i < 8; i++){
    for(int k = 0; k < 8; k++){
      last_board[i][k] = temp_board[i][k];
    }
  }
  move_completed = false;
}

bool PieceDetect::moveCompleted(){
  return move_completed;
}

void PieceDetect::updateBoard(String _fen){
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

    for(int i = 0; i < 8; i++){
      for(int k = 0; k < 8; k++){
        if(fen_board[i][k] == ' '){
          last_board[i][k] = 1;
        }else{
          last_board[i][k] = 0;
        }
      }
    }

    while(!isSame()){
      Serial.println("poupravte figurky, kter mohly byt vyhozeny ze stredu");
      scanBoard();
    }
    Serial.println("Vse je v poradku");
}

bool PieceDetect::isSame(){
  for(int i = 0; i < 8; i++){
    for(int k = 0; k < 8; k++){
      if(last_board[i][k] != temp_board[i][k]){
        Serial.print("Error na: ");
        Serial.println(i* 10 + k);
        return false;
      }
    }
  }
  return true;
}

bool PieceDetect::startPoss(){
  /*while(!isSame()){
    Serial.println("Sachovnice neni pripravena na start hry");
      scanBoard();
  }*/
  if(isSame()){
    return true;
  }else{
    scanBoard();
    startPoss();
  }
}

bool PieceDetect::backTurn(){

  //pokud byla figurky vyhozena tak  ji vratim
  if(last_piece_out > -1){
    int from_column = last_piece_out % 10;
    int from_row = (last_piece_out - from_column) / 10;

    last_board[from_row][from_column] = 0;
  }

  scanBoard();

  if(isSame()){
    return true;
  }

  return false;

}