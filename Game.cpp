//
//  Game.cpp
//  Checkers
//
//  Created by rick gessner on 2/22/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include "Game.hpp"
#include <vector>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <math.h>

namespace ECE141 {

  enum class Reasons {tbd, forfeit, badmove, eliminated, missedJump, moved2, clockExpired};

  class RealGame : public Game {

    std::vector<Piece*> goldPieces;
    std::vector<Piece*> bluePieces;
    std::vector<Tile>   tiles[kBoardHeight];

    PieceColor  loserColor;
    Reasons     reason;
    float       priorDist;
    Piece*      priorPiece;
    const float stepLen;
    const float jumpLen;
    Player      *activePlayer;
    int         step;

  public:

    RealGame() : Game(), loserColor(PieceColor::blue), reason(Reasons::tbd), step(0),
      stepLen(sqrt(2)), jumpLen(sqrt(2)*3), priorDist(0), priorPiece(nullptr) {
      static TileColor theColors[2]={TileColor::light, TileColor::dark};
      int theColor=1;
      for(int theRow=0;theRow<kBoardHeight;theRow++) {
        for(int aColumn=0;aColumn<kBoardWidth;aColumn++) {
          Location theLocation(theRow, aColumn);
          theColor=!theColor;
          tiles[theRow].push_back( ECE141::Tile(theColors[theColor], theLocation)) ;
        }
        theColor=!theColor;
      }
    }

    virtual ~RealGame() { //delete pieces from vectors...
      for(auto p : bluePieces) {delete p;}
      for(auto p : goldPieces) {delete p;}
    }

    //-- USE: setup the players on the board -----------------
    void initPieces() {
      //now put the pieces onto the tiles...
      int theCol=1;
      for(int theRow=2;theRow>=0;theRow--) {
        for(int col=theCol;col<kBoardWidth;col+=2) {
          Piece * thePiece = new Piece(PieceColor::gold, Location(theRow,col));
          goldPieces.push_back(thePiece);
          tiles[theRow][col].piece = thePiece;
        }
        theCol=!theCol;
      }

      for(int theRow=5;theRow<kBoardHeight;theRow++) {
        for(int col=theCol;col<kBoardWidth;col+=2) {
          Piece * thePiece = new Piece(PieceColor::blue, Location(theRow,col));
          bluePieces.push_back(thePiece);
          tiles[theRow][col].piece = thePiece;
        }
        theCol=!theCol;
      }
    }

    //-- USE: make sure location is legit -----------------
    bool validLocation(const Location &aLocation) {
      if(aLocation.row>=0 && aLocation.row<kBoardHeight) {
        return (aLocation.col>=0 && aLocation.col<kBoardWidth);
      }
      return false;
    }

    //-- USE: return # of pieces on board for player -----------------
    size_t countAvailablePieces(PieceColor aColor) {
      auto theStart = PieceColor::blue==aColor ? bluePieces.begin() : goldPieces.begin();
      auto theEnd   = PieceColor::blue==aColor ? bluePieces.end() : goldPieces.end();

      size_t count=0;
      while (theStart != theEnd) {
        Piece *thePiece=(*theStart++);
        if(thePiece && thePiece->color==aColor && PieceKind::captured!=thePiece->kind) count++;
      }

      return count;
    }

    //-- USE: retrieve Nth piece for player (for analysis) -----------------
    virtual const Piece* getAvailablePiece(PieceColor aColor, int anIndex) {
      std::vector<Piece*> *thePieces=PieceColor::blue==aColor
        ? &bluePieces : &goldPieces;
      int count=0;
      for(auto e: *thePieces) {
        if(e->color==aColor && PieceKind::captured!=e->kind && anIndex==count++) {
          return e;
        }
      }
      return nullptr;
    }

    //-- USE: offer RO tile (usually for player analysis) -----------------
    virtual const Tile* getTileAt(const Location &aLocation) {
      return validLocation(aLocation) ? &tiles[aLocation.row][aLocation.col] : nullptr;
    }

    //-- USE: remove jumped piece -------------------------
    void removePieceAt(const Location &aLocation) {
      if(const Tile *theTile=getTileAt(aLocation)) {
        if(theTile->piece) {
          auto thePiece = const_cast<Piece*>(theTile->piece);
          thePiece->kind=PieceKind::captured;
          const_cast<Tile*>(theTile)->piece=nullptr;
        }
      }
    }

    //-- USE: call to determine if player has any available jump...
    bool playerHasJump(const Player &aPlayer) {
      //Vlad-the-compiler has a version of this function that actually works.
      //Students have to figure this out for themselves... :)
      return false;
    }

    //-- USE: check move distance between tiles ------------------------
    float distance(const Location &aSrc, const Location &aDest) {
      return sqrt(pow(aDest.col - aSrc.col, 2) + pow(aDest.row - aSrc.row, 2));
    }

    //-- USE: confirm that dest may be occupied -------------------------
    bool isAvailable(const Tile &aTile) {
      return (!aTile.piece) && (TileColor::dark==aTile.color);
    }

    //-- USE: confirm piece can move that direction  -------------------
    float isValidDirection(const Piece &aPiece, const Location &aSrc, const Location &aDest) {
      if(PieceKind::pawn==aPiece.kind) {
        return (PieceColor::gold==aPiece.color) ? aSrc.row<aDest.row : aSrc.row>aDest.row;
      }
      return true;
    }

    //-- USE: determine whether kingable piece is opponent first row  ----
    float isKingable(const Piece &aPiece, const Location &aDest) {
      if(PieceKind::pawn==aPiece.kind) {
        return PieceColor::blue==aPiece.color ? 0==aDest.row : (kBoardHeight-1)==aDest.row;
      }
      return false;
    }

    //-- USE: determine whether kingable piece is opponent first row  ----
    Piece* jumpedOpponent(const Piece &aPiece, const Location &aDest) {
      int theRow = std::min(aPiece.location.row, aDest.row)+1;
      int theCol = std::min(aPiece.location.col, aDest.col)+1;
      Location theOppoLoc(theRow, theCol);
      if(const Tile *theTile=getTileAt(theOppoLoc)) {
        if(theTile->piece) {
          return theTile->piece->color==aPiece.color ? nullptr : theTile->piece;
        }
      }
      return nullptr;
    }

    //-- USE: ensure given piece can move to given tile... -----------
    bool isValidMove(const Piece &aPiece,const Tile &aSrc,const Tile &aDest) {
      if(isAvailable(aDest)) {
        if(isValidDirection(aPiece, aSrc.location, aDest.location)) {
          if(priorDist==stepLen) return false; //CANT take 2 steps with one piece...

          float theDist = distance(aSrc.location, aDest.location);
          if(theDist>stepLen) {
            return jumpedOpponent(aPiece, aDest.location)!=nullptr;
          }
          return true; //slide...
        }
      }
      return false;
    }

    //-- USE: Ensure user doesn't change pieces during multi-moves --------
    Reasons validatePiece(const Piece &aPiece) {
      Piece *thePiecePtr=(Piece*)&aPiece;
      if(priorPiece && (priorPiece!=thePiecePtr) && (aPiece.color==activePlayer->color)) {
        return Reasons::moved2;
      }
      priorPiece=thePiecePtr;
      return Reasons::tbd;
    }

    //-- USE: accept player decision, update game state ------------------
    bool movePieceTo(const Piece &aPiece, const Location &aDestination) {
      reason=validatePiece(aPiece);
      if(Reasons::tbd==reason) {
        const Tile* theSrcTile=getTileAt(aPiece.location);
        const Tile* theDestTile=getTileAt(aDestination);

        if(isValidMove(aPiece, *theSrcTile, *theDestTile)) {
          const_cast<Tile*>(theSrcTile)->piece=(Piece*)nullptr;  //yank piece from src...
          const_cast<Tile*>(theDestTile)->piece=(Piece*)&aPiece; //place onto dest...

          priorDist=stepLen;
          if(auto theOpponent = jumpedOpponent(aPiece, aDestination)) {
            priorDist=jumpLen;
            removePieceAt(theOpponent->location);
          }
          else {
            if(playerHasJump(*activePlayer)) {
              reason=Reasons::missedJump;
              return false;
            }
          }

          const_cast<Piece&>(aPiece).location=aDestination;      //cache dest on the piece...

          if(isKingable(aPiece, aDestination)) {
            const_cast<Piece&>(aPiece).kind=PieceKind::king;
          }

          return true;
        }
        else reason=Reasons::badmove;
      }
      loserColor = activePlayer->color;
      return false;
    }

    //-- USE: identify terminal condition for game -------------------------
    bool gameCanContinue(Player &aPlayer) {

      if(Reasons::tbd!=reason) return false; //someone already lost...

      if(step<kMaxSteps) {
        size_t blue_count = countAvailablePieces(PieceColor::blue);
        size_t gold_count = countAvailablePieces(PieceColor::gold);

        if(aPlayer.color==PieceColor::blue) {
          if(!blue_count) {
            loserColor = PieceColor::blue;
            reason=Reasons::eliminated;
          }
        }
        else {
          if(!gold_count) {
            loserColor = PieceColor::gold;
            reason=Reasons::eliminated;
          }
        }
        return blue_count*gold_count>0;
      }
      reason=Reasons::clockExpired;
      return false;
    }

    //-- USE: Print conclusing message to console... -------------------------
    RealGame& showGameResults() {

      const char *theLoser=PieceColor::blue==loserColor ? "Blue" : "Gold";
      const char *theWinner=PieceColor::blue==loserColor ? "Gold" : "Blue";

      switch(reason) {
        case Reasons::eliminated:
          std::cout << theWinner << " has eliminated " << theLoser << " from the board." << std::endl;
          break;
        case Reasons::badmove:
          std::cout << theLoser << " is disqualified for choosing an illegal move." << std::endl;
          break;
        case Reasons::forfeit:
          std::cout << theLoser << " has forfeited the game." << std::endl;
          break;
        case Reasons::missedJump:
          std::cout << theLoser << " has forfeited by missing a jump." << std::endl;
          break;
        case Reasons::moved2:
          std::cout << theLoser << " is disqualified for moving two pieces." << std::endl;
          break;
        case Reasons::clockExpired:
          std::cout << "Game terminated because the clock expired." << std::endl;
          break;
        case Reasons::tbd:
          std::cout << "Game terminated for an unknown reason." << std::endl;
          break;
      }
      return *this;
    }

    //-- USE: dumps state of board to terminal... -------------------------
    void visualizeBoard() {
      const char* theSpacer = "|---|---|---|---|---|---|---|---|";

      std::cout << std::endl << "  Step " << ++step << std::endl;
      std::cout << "  " << theSpacer << std::endl;

      for(int row=0;row<kBoardHeight;row++) {
        std::cout << row << " |";
        for(int col=0;col<8;col++) {
          Location theLocation(row,col);
          auto &tile = tiles[row][col];
          if(!tile.piece) {
            std::cout << " . |";
          }
          else {
            char theColor = tile.piece->color == PieceColor::blue ? 'b' : 'g';
            if(PieceKind::king==tile.piece->kind) theColor=toupper(theColor);
            std::cout << " " << theColor << " |";
          }
        }
        std::cout << std::endl;
        if(row<kBoardHeight-1) std::cout << "  " << theSpacer << std::endl;
      }

      std::cout <<  "  " << theSpacer << std::endl
        << "    0   1   2   3   4   5   6   7" << std::endl;
    }

    //-- USE: iterates player turns until end state  -------------------------
    RealGame& run(Player &aPlayer1, Player &aPlayer2) {

      initPieces(); //once at the start of each game...

      activePlayer=&aPlayer1;
      while(Reasons::tbd==reason && gameCanContinue(*activePlayer)) {
        visualizeBoard();
        if(!activePlayer->takeTurn(*this)) {
          loserColor = activePlayer->color;
          reason=Reasons::forfeit;
        }
        else if(priorDist==jumpLen && playerHasJump(*activePlayer)){
          loserColor = activePlayer->color;
          reason=Reasons::missedJump;
        }
        activePlayer = (activePlayer==&aPlayer1) ? &aPlayer2 : &aPlayer1;
        priorPiece=nullptr; //reset state...
        priorDist=0;
      }
      visualizeBoard();

      return showGameResults();
    }
  };

  //-- USE: game factory  -------------------------
  Game* Game::create() {
    RealGame *theGame = new RealGame();
    return theGame;
  }

}
