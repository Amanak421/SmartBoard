#include <Arduino.h>

#include "motor_movement.h"
#include "servercom.h"

#include "chess_board.h"
#include "validate_move.h"
#include "display.h"
#include "piece_detect.h"

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

ValidateMove validmove;

Display display;

PieceDetect piece_detect;

char ssid[] = "andrlink";
char password[] = "1kub157201";

String instream = "";

int on_move = 0;
int player_color = 1;
int player_color_char = 'w';    //používá se pro valid move a ten má šachovnici jak je ve fyzické podobě -> hraju za bílí
String player_color_str = "b";

const int CHECK_DELAY_INTERNET = 5000;
unsigned long last_int_activation = 0;
const int CHECK_DELAY = 250;
unsigned long last_activation = 0;

bool game_started = false;

/*const int INTERNET = 0;
const int ENGINE = 1;*/

int game_mode = -1;

/* flagy pro mod internet */

bool getBoard = false;


void readCommand(String _command);

void setup() {
  Serial.begin(9600);

  Serial.print("Probehlo...");

  motor_move.setBoard(board);   //nastaví původní šachovnici

  servercom.begin(ssid, password); //připojí se k síti

  //validmove.showPossibleMoves(13);
  /*Serial.print("Mohu provest tah z 64 na 44: ");
  Serial.println(validmove.validateMove(64, 44));*/

  display.begin();
	display.setPage();
	servercom.scan();
	display.setWifiList(servercom.ssids);

  motor_move.returnToHome();

  while(!piece_detect.startPoss()){
    piece_detect.printPieceBoard();
  }


  Serial.println("Zadejte command: ");
  

}

void loop() {

  //piece_detect.printPieceBoard();
  //piece_detect.checkMove();
  /*piece_detect.checkMove();
  if(piece_detect.moveCompleted()){
    Serial.print("Odebrano z: ");
    Serial.println(piece_detect.last_from);
    Serial.print("Umisteno na: ");
    Serial.println(piece_detect.last_to);
    if(validmove.validateMove(piece_detect.last_from + 1, piece_detect.last_to + 1)){
      Serial.println("Tah byl proveden uspesne!");
      piece_detect.finishMove();
    }else{
      Serial.println("Selhalo!!!!!");
      while(true){

      }
    }
  }*/
  


  if(Serial.available() > 0){
    instream = Serial.readString();
    instream.trim();
    readCommand(instream);
  }

  display.updateCursor();
	display.updateButton();

	if(display.wifiRefresh()){
		servercom.scan();
		display.setWifiList(servercom.ssids);
    display.wifiRefreshComplete();
	}else if(display.wifiConnect()){
    const char* ssid = display.ssid();
    const char* pass = display.pass();
    servercom.begin(ssid, pass);
    display.wifiLoadComplete();
  }else if(display.wifiDissconnect()){
    WiFi.disconnect();
    display.wifiDisconnectComplete();
  }else if(display.gameStart()){
    game_started = true;
    game_mode = display.gameMode();
  }

  if(millis() - last_activation > CHECK_DELAY && game_started && game_mode == INTERNET){

    if(on_move != player_color){

      if(getBoard){
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
          piece_detect.updateBoard(servercom.encodeFEN());
          while(!piece_detect.isSame()){   //lze doplnit do loopu asi
          Serial.println("poupravte figurky, kter mohly byt vyhozeny ze stredu");
          piece_detect.scanBoard();
          }
          Serial.println("Vse je v poradku");
          validmove.updateBoard(servercom.encodeFEN());

          validmove.setPossEnnPassCell(servercom.retEnnPassCell());
          
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
          getBoard = false;
      }

      if(millis() - last_int_activation > CHECK_DELAY_INTERNET){
          //je potřeba získat šachovnici
          if(servercom.httpGetLastOnMove() != player_color_str && servercom.httpGetLastOnMove() != "error"){
            getBoard = true;
          }else{
            getBoard = false;
            Serial.println("Druhy hrac jeste neodehral...");
          }

          last_int_activation = millis();
      }

    }else{
      Serial.println("Jsi na tahu update nemá cenu...");

      /* DETEKCE FIGUREK  */

      piece_detect.checkMove();   //načte desku
      if(piece_detect.moveCompleted()){
        Serial.print("Odebrano z: ");
        Serial.println(piece_detect.last_from);
        Serial.print("Umisteno na: ");
        Serial.println(piece_detect.last_to);
        if(validmove.validateMove(piece_detect.last_from + 1, piece_detect.last_to + 1) && !validmove.checkMate(player_color_char)){
          Serial.println("Tah byl proveden uspesne!");
          
          servercom.printBoard();

          piece_detect.last_from += 1;
          piece_detect.last_to += 1;

          /*int reverse_from = (70 + (2 * (piece_detect.last_from % 10))) - piece_detect.last_from;
          int reverse_to = (70 + (2 * (piece_detect.last_to % 10))) - piece_detect.last_to;*/

          Serial.print("FROM: ");
          Serial.println(piece_detect.last_from);
          Serial.print("TO:");
          Serial.println(piece_detect.last_to);


          servercom.updateLastMove(piece_detect.last_from, piece_detect.last_to);   //aktualizuje poslední tah ve třídě

          String last_move = String(piece_detect.last_from) + "_" + String(piece_detect.last_to);    //vytvoří last_move (spojí poslední tah do jednoho stringu)

          if(piece_detect.lastTurnCastK()){     //zahrana rosada na kralovu stranu

            motor_move.doSpecialMoveWithouMotors(piece_detect.last_from, piece_detect.last_to, "castk");
            servercom.doSpecialMove(piece_detect.last_from, piece_detect.last_to, "castk");
            servercom.httpSend(servercom.encodeChessstring(), player_color_str, last_move, "castk");
            servercom.setCastle('b', 'k', false);

          }else if(piece_detect.lastTurnCastQ()){   //zahrana rosada na stranu damy

            motor_move.doSpecialMoveWithouMotors(piece_detect.last_from, piece_detect.last_to, "castq");
            servercom.doSpecialMove(piece_detect.last_from, piece_detect.last_to, "castq");
            servercom.httpSend(servercom.encodeChessstring(), player_color_str, last_move, "castq");
            servercom.setCastle('b', 'q', false);

          }else if(piece_detect.checkEnnPass(piece_detect.last_from, piece_detect.last_to)){

            motor_move.doSpecialMoveWithouMotors(piece_detect.last_from, piece_detect.last_to, "ennpass");
            servercom.doSpecialMove(piece_detect.last_from, piece_detect.last_to, "ennpass");
            servercom.httpSend(servercom.encodeChessstring(), player_color_str, last_move, "ennpass");

          }else{
            motor_move.doMoveWithoutMotors(piece_detect.last_from, piece_detect.last_to);   //provede tah na interní šachovnici ve třídě motorů
            servercom.doMove(piece_detect.last_from, piece_detect.last_to);   //provede tah na desce třídy pro komunikaci se serverem
            //chess_board.doMove(piece_detect.last_from, piece_detect.last_to);
            servercom.httpSend(servercom.encodeChessstring(), player_color_str, last_move, "none");   //odešle šachovnici na server
          }

          Serial.println("MOTORY: ");
          motor_move.printBoard();
          Serial.println("SERVER: ");
          servercom.printBoard();

          validmove.updateBoard(servercom.encodeFEN());   //aktualizuje šachovnici pro validaci tahů
          piece_detect.updateBoard(servercom.encodeFEN());  //aktualizuje šachovnici pro detekci figurek

          Serial.println(servercom.encodeChessstring());

          piece_detect.finishMove();

          on_move = 0;
          getBoard = false;

        }else if(validmove.checkMate(player_color_char)){

          Serial.println("Prohral jsi!!!");
          while(true){

          }

        getBoard = false;
        
        }else{
          Serial.println("Selhalo!!!!!");
          while(!piece_detect.backTurn()){
            Serial.println("Vratte figurky na puvodni misto!!!");
          }
          Serial.println("Vse je vporadku... Muzete tahnout validni tah");
        }

        getBoard = false;
      }
    }

    //kotrola databáze
    last_activation = millis();

  }

  if(game_mode == ENGINE && game_started){

    display.onTurn(on_move);

    piece_detect.checkMove();   //načte desku
    if(piece_detect.moveCompleted()){
      Serial.print("Odebrano z: ");
      Serial.println(piece_detect.last_from);
      Serial.print("Umisteno na: ");
      Serial.println(piece_detect.last_to);
      if(validmove.validateMove(piece_detect.last_from + 1, piece_detect.last_to + 1)){

        Serial.println("Tah byl proveden uspesne!");
        Serial.println("Generuji tah oponenta");
        display.onTurn(1);

        if(piece_detect.lastTurnCastK()){
          chess_board.doMove(piece_detect.last_from + 1, piece_detect.last_to + 1, "castk");
        }else if(piece_detect.lastTurnCastQ()){
          chess_board.doMove(piece_detect.last_from + 1, piece_detect.last_to + 1, "castq");
        }else if(piece_detect.checkEnnPass(piece_detect.last_from + 1, piece_detect.last_to + 1)){
          chess_board.doMove(piece_detect.last_from + 1, piece_detect.last_to + 1, "ennpass");
        }else{
          chess_board.doMove(piece_detect.last_from + 1, piece_detect.last_to + 1);
        }
        
        piece_detect.finishMove();

        Serial.println(chess_board.encodeFEN());
        chess_board.getNextMove(chess_board.encodeFEN());
        motor_move.doMoveFromServer(chess_board.last_from, chess_board.last_to);
        motor_move.setBoard(chess_board.num_board);
        motor_move.printBoard();
        validmove.updateBoard(chess_board.encodeFEN());
        piece_detect.updateBoard(chess_board.encodeFEN());
         display.engineInfoError(CORRECT_PIECE);
        while(!piece_detect.isSame()){   //lze doplnit do loopu asi
          Serial.println("poupravte figurky, kter mohly byt vyhozeny ze stredu");
          display.engineInfoError(CORRECT_PIECE);
          piece_detect.scanBoard();
        }
        display.engineInfoError(NONE);
        Serial.println("Vse je v poradku");
      }else{
        Serial.println("Selhalo!!!!!");
        display.engineInfoError(ILLEGAL_MOVE);
        while(!piece_detect.backTurn()){
          Serial.println("Vratte figurky na puvodni misto!!!");
        }
        display.engineInfoError(NONE);
        Serial.println("Vse je vporadku... Muzete tahnout validni tah");
      }
    }

    /************* KONZOLE *****************************/
    String _com = instream.substring(0, instream.indexOf(' '));
    if(_com == "pm"){
      Serial.print("Parametr 1: ");
      String par1 = instream.substring(instream.indexOf(' ') + 1, instream.indexOf(' ') + 3);
      Serial.println(par1.toInt());
      Serial.print("Parametr 2: ");
      String par2 = instream.substring(instream.length() - 2, instream.length());
      Serial.println(par2.toInt());


      if(validmove.validateMove(par1.toInt(), par2.toInt())){
        chess_board.doMove(par1.toInt(), par2.toInt());
        Serial.println(chess_board.encodeFEN());
        chess_board.getNextMove(chess_board.encodeFEN());
        motor_move.doMoveFromServer(chess_board.last_from, chess_board.last_to);
        motor_move.setBoard(chess_board.num_board);
        validmove.updateBoard(chess_board.encodeFEN());
        on_move = 0;
      }else{
        Serial.println("NELEGALNI TAH!");
      }
      
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


      if(validmove.validateMove(from, to)){
        chess_board.doMove(from, to, par3);
        Serial.println(chess_board.encodeFEN());
        chess_board.getNextMove(chess_board.encodeFEN());
        motor_move.doMoveFromServer(chess_board.last_from, chess_board.last_to);
        motor_move.setBoard(chess_board.num_board);
        validmove.updateBoard(chess_board.encodeFEN());
        on_move = 0;
      }else{
        Serial.println("NELEGALNI TAH!");
      }

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
      //piece_detect.setPlayerColor(1);

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
      chess_board.setReverse(state);
      piece_detect.setReverse(state);
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

