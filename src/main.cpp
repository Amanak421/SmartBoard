#include <Arduino.h>

#include "motor_movement.h"
#include "servercom.h"

#include "chess_board.h"

  int board[8][8] = {
      {2, 3, 4, 5, 6, 4, 3, 2},
      {1, 1, 1, 1, 1, 1, 1, 1},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {1, 1, 1, 1, 1, 1, 1, 1},
      {2, 3, 4, 5, 6, 4, 3, 2}
  };

MotorMovement motor_move;

ServerCom servercom;

ChessBoard chess_board;

char ssid[] = "andrlink";
char password[] = "1kub157201";

String instream = "";

int on_move = 0;
int player_color = 1;
String player_color_str = "b";

const int CHECK_DELAY = 5000;
unsigned long last_activation = 0;

bool game_started = false;

const int INTERNET = 0;
const int ENGINE = 1;

int game_mode = -1;

void readCommand(String _command);

void setup() {
  Serial.begin(9600);

  Serial.print("Probehlo...");

  motor_move.setBoard(board);   //nastaví původní šachovnici

  servercom.begin(ssid, password); //připojí se k síti


  Serial.println("Zadejte command: ");

}

void loop() {
  // put your main code here, to run repeatedly:


  if(Serial.available() > 0){
    instream = Serial.readString();
    instream.trim();
    readCommand(instream);
  }

  if(millis() - last_activation > CHECK_DELAY && game_started && game_mode == INTERNET){

    if(on_move != player_color){

      if(servercom.httpGetLastOnMove() != player_color_str && servercom.httpGetLastOnMove() != "error"){
          //je potřeba získat šachovnici
          motor_move.printBoard();
          servercom.encodeJson(servercom.httpGetChessboard());
          servercom.decodeChessstring(servercom.chessstring);
          servercom.printBoard();
          motor_move.doMoveFromServer(servercom.retLastMove());
          servercom.decodeChessBoard();
          int helper_board[8][8];
          for(int i = 0; i < 8; i++){
            for(int k = 0; k < 8; k++){
              helper_board[i][k] = servercom.motor_move_board[i][k];
            }
          }
          motor_move.setBoard(helper_board);
          motor_move.printBoard();
          if(servercom.retSpecMove() != "none" || servercom.retSpecMove() != ""){

            if(servercom.retSpecMove() == "exchd"){
                Serial.println("Vymente prave posunuteho pesce za damu...");
                Serial.println("Potvrdte napsanim 'OK' ");
                bool is_exch_completed = false;
                while(!is_exch_completed){
                  if(Serial.available() > 0){
                    String _ok = Serial.readString();
                    if(_ok == "OK"){
                      is_exch_completed = true;
                    }else{
                      Serial.println("Nejprve potvrdte provedenou vymenu!!!");
                    }
                  }
                }
            }else if(servercom.retSpecMove() == "exchs"){
              Serial.println("Vymente prave posunuteho pesce za strelce...");
                Serial.println("Potvrdte napsanim 'OK' ");
                bool is_exch_completed = false;
                while(!is_exch_completed){
                  if(Serial.available() > 0){
                    String _ok = Serial.readString();
                    if(_ok == "OK"){
                      is_exch_completed = true;
                    }else{
                      Serial.println("Nejprve potvrdte provedenou vymenu!!!");
                    }
                  }
                }

            }else if(servercom.retSpecMove() == "exchk"){
              Serial.println("Vymente prave posunuteho pesce za kone...");
                Serial.println("Potvrdte napsanim 'OK' ");
                bool is_exch_completed = false;
                while(!is_exch_completed){
                  if(Serial.available() > 0){
                    String _ok = Serial.readString();
                    if(_ok == "OK"){
                      is_exch_completed = true;
                    }else{
                      Serial.println("Nejprve potvrdte provedenou vymenu!!!");
                    }
                  }
                }
              
            }else if(servercom.retSpecMove() == "exchv"){

              Serial.println("Vymente prave posunuteho pesce za vez...");
                Serial.println("Potvrdte napsanim 'OK' ");
                bool is_exch_completed = false;
                while(!is_exch_completed){
                  if(Serial.available() > 0){
                    String _ok = Serial.readString();
                    if(_ok == "OK"){
                      is_exch_completed = true;
                    }else{
                      Serial.println("Nejprve potvrdte provedenou vymenu!!!");
                    }
                  }
                }
              
            }

          }
          on_move = 1;
      }else{
        Serial.println("Druhý hráč ještě neodehrál...");
      }

    }else{
      Serial.println("Jsi na tahu update namá cenu...");
    }

    /*kotrola databáze*/
    last_activation = millis();

  }

  if(game_mode == ENGINE){
    String _com = instream.substring(0, instream.indexOf(' '));
    if(_com == "pm"){
      Serial.print("Parametr 1: ");
      String par1 = instream.substring(instream.indexOf(' ') + 1, instream.indexOf(' ') + 3);
      Serial.println(par1.toInt());
      Serial.print("Parametr 2: ");
      String par2 = instream.substring(instream.length() - 2, instream.length());
      Serial.println(par2.toInt());

      chess_board.doMove(par1.toInt(), par2.toInt());
      Serial.println(chess_board.encodeFEN());
      chess_board.getNextMove(chess_board.encodeFEN());
      motor_move.doMoveFromServer(chess_board.last_from, chess_board.last_to);
      motor_move.setBoard(chess_board.num_board);
      on_move = 0;
      instream = "";
    }else if(_com == "pms"){
      Serial.print("Parametr 1: ");
      String par1 = instream.substring(instream.indexOf(' ') + 1, instream.indexOf(' ') + 3);
      Serial.println(par1);
      Serial.print("Parametr 2: ");
      String par2 = instream.substring(instream.indexOf(' ') + 4, instream.lastIndexOf(' '));
      Serial.println(par2);
      Serial.print("Parametr 3: ");
      String par3 = instream.substring(instream.lastIndexOf(' ') + 1);
      Serial.println(par3);

      int from = par1.toInt();
      int to = par2.toInt();

      chess_board.doMove(from, to, par3);
      Serial.println(chess_board.encodeFEN());
      chess_board.getNextMove(chess_board.encodeFEN());
      motor_move.doMoveFromServer(chess_board.last_from, chess_board.last_to);
      motor_move.setBoard(chess_board.num_board);
      on_move = 0;
      instream = "";
    }
  }

}

void readCommand(String _command){

  String command = _command;
    String _com = command.substring(0, command.indexOf(' '));
    if(_com == "cpmb"){
      Serial.print("Parametr 1: ");
      String par1 = command.substring(command.indexOf(' ') + 1, command.indexOf(' ') + 3);
      Serial.println(par1.toInt());
      Serial.print("Parametr 2: ");
      String par2 = command.substring(command.length() - 2, command.length());
      Serial.println(par2.toInt());

      motor_move.printBoard();
      motor_move.computeCellMovement(par1.toInt(), par2.toInt());
      motor_move.printMoves();
      motor_move.printBoard();
    }else if(_com == "clm"){
      motor_move.clearMoves();
      Serial.println("Pohyby vymazany");
    }else if(_com ==  "mte"){
      String _motor = command.substring(command.indexOf(' ') + 1, command.indexOf(' ') + 2);
      String _dir = command.substring(command.length() - 2, command.length());
      int dir = _dir.toInt();
      if(_motor == "X"){
        Serial.print("Motor X");
        Serial.print("DIR: ");
        Serial.println(dir);
        motor_move.moveToEndstop(MOTOR_X, dir);
      }else if(_motor == "Y"){
        Serial.print("Motor Y");
        Serial.print("DIR: ");
        Serial.println(dir);
        motor_move.moveToEndstop(MOTOR_Y, dir);
      }else{
        Serial.println("Nezname parametry!");
      }
    }else if(_com == "mst"){
      String _state = command.substring(command.indexOf(' ') + 1, command.indexOf(' ') + 2);
      int st = _state.toInt();
      Serial.println(st);
      if(st == 1){
        motor_move.setMagnetState(ON);
        Serial.println("Magnet zapnut");
      }else if(st == 0){
        motor_move.setMagnetState(OFF);
        Serial.println("Magnet vypnut");
      }else{
        Serial.println("Nezname parametry!");
      }
    }else if(_com == "rteh"){
      Serial.println("Provadim pohyb...");
      motor_move.moveToEndstop(MOTOR_X, 1);
      motor_move.moveToEndstop(MOTOR_X, 1);
      motor_move.moveToEndstop(MOTOR_X, -1);
      motor_move.moveToEndstop(MOTOR_X, -1);
      motor_move.moveToEndstop(MOTOR_Y, 1);
      motor_move.moveToEndstop(MOTOR_Y, 1);
      motor_move.moveToEndstop(MOTOR_Y, -1);
      motor_move.moveToEndstop(MOTOR_Y, -1);
    }else if(_com == "rth"){
      Serial.println("Provádím pohyb");
      motor_move.returnToHome();
    }else if(_com == "mtc"){
      String _dir = command.substring(command.indexOf(' ') + 1, command.indexOf(' ') + 2);
      int dir = _dir.toInt();
      if(dir == 1){
        motor_move.moveCorToCen(1);
      }else if(dir == 0){
        motor_move.moveCorToCen(-1);
      }
    }else if(_com == "dmfv"){
      motor_move.doMotorMove();
    }else if(_com == "sgid"){
      String _game_id = command.substring(command.indexOf(' ') + 1, command.length());
      int game_id = _game_id.toInt();
      Serial.print("Herni id nastaveno na: ");
      Serial.println(game_id);

      servercom.setGameId(game_id);
      game_started = true;

    }else if(_com == "opm"){

      if(on_move == player_color){

        Serial.print("Parametr 1: ");
        String par1 = command.substring(command.indexOf(' ') + 1, command.indexOf(' ') + 3);
        Serial.println(par1.toInt());
        Serial.print("Parametr 2: ");
        String par2 = command.substring(command.length() - 2, command.length());
        Serial.println(par2.toInt());

        servercom.printBoard();

        motor_move.doMoveWithoutMotors(par1.toInt(), par2.toInt());
        servercom.doMove(par1.toInt(), par2.toInt());
        String last_move = par1 + "_" + par2;
        servercom.httpSend(servercom.encodeChessstring(), player_color_str, last_move, "none");
        motor_move.printBoard();
        servercom.printBoard();
        Serial.println(servercom.encodeChessstring());

        on_move = 0;

      }else{
        Serial.println("Nejsi na tahu...");
      }

      
    }else if(_com == "str"){
      String _state = command.substring(command.indexOf(' ') + 1, command.indexOf(' ') + 2);
      int state = _state.toInt();
      motor_move.setReverse(state);
      servercom.setReverse(state);
      Serial.print("Reverse nataveno na: ");
      Serial.println(state);

    }else if(_com == "opms"){

      Serial.print("Parametr 1: ");
      String par1 = command.substring(command.indexOf(' ') + 1, command.indexOf(' ') + 3);
      Serial.println(par1);
      Serial.print("Parametr 2: ");
      String par2 = command.substring(command.indexOf(' ') + 4, command.lastIndexOf(' '));
      Serial.println(par2);
      Serial.print("Parametr 3: ");
      String par3 = command.substring(command.lastIndexOf(' ') + 1);
      Serial.println(par3);

      int from = par1.toInt();
      int to = par2.toInt();

      motor_move.doSpecialMoveWithouMotors(from, to, par3);
      servercom.doSpecialMove(from, to, par3);
      String last_move = par1 + "_" + par2;
      servercom.httpSend(servercom.encodeChessstring(), player_color_str, last_move, par3);
      motor_move.printBoard();
      servercom.printBoard();
      Serial.println(servercom.encodeChessstring());

      on_move = 0;

    }/*else if(_com == "pm"){
      Serial.print("Parametr 1: ");
      String par1 = command.substring(command.indexOf(' ') + 1, command.indexOf(' ') + 3);
      Serial.println(par1.toInt());
      Serial.print("Parametr 2: ");
      String par2 = command.substring(command.length() - 2, command.length());
      Serial.println(par2.toInt());

      chess_board.doMove(par1.toInt(), par2.toInt());
      Serial.println(chess_board.encodeFEN());
      chess_board.getNextMove(chess_board.encodeFEN());
    }*/else if(_com == "stgm"){
      String _state = command.substring(command.indexOf(' ') + 1, command.indexOf(' ') + 2);
      int state = _state.toInt();
      game_mode = state;
      Serial.print("Herni mod nastaveny na: ");
      Serial.println(state);
    }

}