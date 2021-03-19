#include <Arduino.h>

#include "motor_movement.h"

  int board[8][8] = {
      {2, 3, 4, 5, 6, 4, 3, 2},
      {1, 1, 1, 1, 1, 1, 1, 1},
      {0, 1, 0, 1, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 1, 1, 5, 0, 1, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {1, 1, 1, 1, 1, 1, 1, 1},
      {2, 3, 4, 5, 6, 4, 3, 2}
  };

MotorMovement motor_move;

void setup() {
  Serial.begin(9600);

  motor_move.setBoard(board);
  motor_move.printBoard();

  //motor_move.computeCellMovement(11, 22);
  motor_move.addEnPassantMove(motor_move.test1, motor_move.test2, 1);
  motor_move.printMoves();

  

  //Serial.println(motor_move.checkPath(motor_move.test1, motor_move.test2, motor_move.test_move));

}

void loop() {
  // put your main code here, to run repeatedly:
}