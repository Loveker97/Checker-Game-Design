//
//  Player.cpp
//  Checkers
//
//  Created by rick gessner on 2/22/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include "Player.hpp"
#include "Game.hpp"
#include <iostream>

namespace ECE141 {

  int Player::pcount = 0; //init our static member to track # of players...

  static PieceColor autoColor() { //auto assigns a color
    return (++Player::pcount % 2) ? PieceColor::blue : PieceColor::gold;
  }

  Player::Player() : color(autoColor()) {}

  bool Player::takeTurn(Game &aGame) {
    size_t theCount=aGame.countAvailablePieces(color);
    for(int pos=0;pos<theCount;pos++) {
      /*
       //NOTICE HOW WE CHECK FOR CAPTURED PIECES?
       if(const Piece *thePiece = aGame.getAvailablePiece(this->color, pos)) {
       
         //add logic here to find your best move, then move ONE piece per turn...
       
         aGame.movePieceTo(*thePiece, Location(1,2)); -- choose wisely...
       }
      */
    }
    return false; //if you return false, you forfeit!
  }
}