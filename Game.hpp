//
//  Game.hpp
//  Checkers
//
//  Created by rick gessner on 2/22/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#ifndef Game_hpp
#define Game_hpp

#include "Player.hpp"
#include "Piece.hpp"
#include  "stdio.h"

namespace ECE141 {
  
  class Game {
  public:
    
    static const int kBoardWidth = 8;
    static const int kBoardHeight= 8;
    static const int kPiecesPerSide = 12;
    static const int kMaxSteps = 200; //max # of steps before game is terminated...

    virtual ~Game() {}
    
    static  Game*         create(); //factory to make a game...
    
    virtual Game&         run(Player &aPlayer1, Player &aPlayer2)=0; //call from main
    
      //The following methods are used by your player to interact with game...
    
    virtual size_t        countAvailablePieces(PieceColor aColor)=0;
    virtual const Piece*  getAvailablePiece(PieceColor aColor, int anIndex)=0;
    virtual bool          validLocation(const Location &aLocation)=0;
    virtual const Tile*   getTileAt(const Location &aLocation)=0;

    virtual bool          movePieceTo(const Piece &aPiece, const Location &aLocation)=0;    

  };
}

#endif /* Game_hpp */
