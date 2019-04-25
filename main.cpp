//
//  main.cpp
//  Checkers
//
//  Created by rick gessner on 2/22/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include "Game.hpp"
#include <vector>
#include <numeric>
#include "HangZ_ZanDPlayer.hpp"

int main(int argc, const char * argv[]) {
  
  ECE141::HangZ_ZanDPlayer player1;
  ECE141::HangZ_ZanDPlayer player2; //play against yourself for now...

  ECE141::Game *theGame=ECE141::Game::create();
  theGame->run(player1,player2);
  delete theGame;

  return 0;
}
