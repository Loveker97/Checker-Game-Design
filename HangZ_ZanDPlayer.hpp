#ifndef HangZ_ZanDPlayer_hpp
#define HangZ_ZanDPlayer_hpp

#include "Piece.hpp"
#include "Player.hpp"
#include "Game.hpp"
#include <unordered_map>
#include <vector>

namespace ECE141 {


	class HangZ_ZanDPlayer : public Player {
	public:
		HangZ_ZanDPlayer() {}
		
		std::unordered_map<const Piece*, std::vector<std::vector<Location>>> Neighborhood(Game &aGame, int firsttime) {
			std::unordered_map<const Piece*, std::vector<std::vector<Location>>> ump;
			size_t theCount = aGame.countAvailablePieces(color);
			int flag = (color == PieceColor::blue ? -1 : 1);
			int row_add[] = { 1, 1, -1, -1 };
			int col_add[] = { -1, 1, -1, 1 };
			for (int pos = 0; pos < theCount; pos++) {
				const Piece *aPiece = aGame.getAvailablePiece(color, pos);
				if (!aPiece) continue;
				int isking = (aPiece->kind == PieceKind::king ? 4 : 2);
				for (int i = 0; i < isking; ++i) {
					Location nlocation(aPiece->location.row + flag * row_add[i], aPiece->location.col + col_add[i]);
					std::vector<Location> path;
					if (!aGame.validLocation(nlocation)) continue;
					const Tile* ntile = aGame.getTileAt(nlocation);
					if (!(TileColor::dark == ntile->color)) continue;
					if (!ntile->piece) {
						path.push_back(nlocation);
						ump[aPiece].push_back(path);
					}
					else {
						HangZ_ZanDPlayer::dfs(aGame, ump, aPiece, aPiece->location, path, {}, firsttime);
					}
				}
			}
			return ump;
		}

		void dfs(Game &aGame, std::unordered_map<const Piece*, std::vector<std::vector<Location>>> &ump,
			const Piece* &aPiece, Location location, std::vector<Location> path, std::vector<Location> visit_path, int firsttime) {
			int flag = (color == PieceColor::blue ? -1 : 1);
			int row_add[] = { 1, 1, -1, -1 };
			int col_add[] = { -1, 1, -1, 1 };
			int isking = (aPiece->kind == PieceKind::king ? 4 : 2);
			for (int i = 0; i < isking; ++i) {
				Location jumpover_location(location.row + flag * row_add[i], location.col + col_add[i]);
				Location nlocation(location.row + flag * row_add[i] * 2, location.col + col_add[i] * 2);
				if (!aGame.validLocation(jumpover_location) || !aGame.validLocation(nlocation)) continue;
				const Tile* jumpover_tile = aGame.getTileAt(jumpover_location);
				const Tile* ntile = aGame.getTileAt(nlocation);
				if (jumpover_tile->piece && (jumpover_tile->piece->color != color) && !ntile->piece) {
					if (firsttime == 0) ump.clear();
					firsttime = 1;
					int visited = 0;
					for (auto i : visit_path) {
						if (i.row == jumpover_location.row && i.col == jumpover_location.col) visited = 1;
					}
					if (visited == 1) continue;
					visit_path.push_back(jumpover_location);
					path.push_back(nlocation);
					ump[aPiece].push_back(path);
					dfs(aGame, ump, aPiece, nlocation, path, visit_path, firsttime);
					path.pop_back();
				}
			}
		}

		std::pair<const Piece*, std::vector<Location>> Objective(std::unordered_map<const Piece*, std::vector<std::vector<Location>>> map, Game & aGame) {
			int flag = (color == PieceColor::blue ? -1 : 1);
			int max_eat = 1, max_eat_path = -1, lowest_pawn = INT_MAX, allking = 1;
			const Piece* max_eat_piece;
			const Piece* lowest_pawn_piece;
			for (auto iter : map) {
				const Piece* k = iter.first;
				if (k->kind == PieceKind::pawn) {
					allking = 0;
					if (k->location.row * flag < lowest_pawn) {
						lowest_pawn = k->location.row * flag;
						lowest_pawn_piece = k;
					}
				}
				for (int i = 0; i < map[k].size(); ++i) {
					if (max_eat < map[k][i].size()) {
						max_eat = map[k][i].size();
						max_eat_piece = k;
						max_eat_path = i;
					}
				}
			}
			if (max_eat > 1) {
				std::pair<const Piece*, std::vector<Location>> result({ max_eat_piece, map[max_eat_piece][max_eat_path] });
				return result;
			}
			if (allking) {
				int r1 = rand() % map.size();
				auto iter = map.begin();
				while (r1--) {
					iter++;
				}
				const Piece* k = iter->first;
				int r2 = rand() % map[k].size();
				std::vector<Location> BestLoc = map[k][r2];
				std::pair<const Piece*, std::vector<Location>> result({ k, BestLoc });
				return result;
			}
			std::vector<Location> BestLoc = map[lowest_pawn_piece][0];
			std::pair<const Piece*, std::vector<Location>> result({ lowest_pawn_piece, BestLoc });
			return result;
			//auto iter = map.begin();
			//const Piece* k = iter->first;
			//std::vector<Location> BestLoc = map[k][0];
			//std::pair<const Piece*, std::vector<Location>> result({ k, BestLoc });
			//return result;
		}

		bool      takeTurn(Game &aGame) {
			int firsttime = 0;
			std::unordered_map<const Piece*, std::vector<std::vector<Location>>> ump = HangZ_ZanDPlayer::Neighborhood(aGame, firsttime);
			if (ump.size() == 0) return false;
			int n = ump.size();
			//for (auto iter = ump.begin(); iter != ump.end(); iter++) {
			//	const Piece* k = iter->first;
			//	for (int j = 0; j < ump[k].size(); ++j) {
			//		for (int m = 0; m < ump[k][j].size(); ++m)
			//			std::cout << k -> location.row << ',' << k->location.col << ':' << ump[k][j][m].row << ',' << ump[k][j][m].col << std::endl;
			//	}
			//}
			std::pair<const Piece*, std::vector<Location>> aPair = Objective(ump, aGame);
			for (int i = 0; i < aPair.second.size(); ++i) {
				aGame.movePieceTo(*(aPair.first), aPair.second[i]);
			}
			return true;
		}
	};
}

#endif /* HangZ_ZanDPlayer_hpp */