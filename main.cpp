#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "chess/Common.h"
#include "search/Search.h"

#include "helpers/GameStateHelper.h"
#include "movegen/MoveGenTest.h"
#include "helpers/Perft.h"
#include "movegen/PrecomputedTables.h"


int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	GameState gameState((std::string)DEFAULT_FEN_POSITION);
	std::vector<MoveInfo> history;
	history.reserve(256);

	// iterativeDeepeningSearch(gameState, history);

	std::string command;
	while (std::getline(std::cin, command)) {
		if (command == "uci") {
			std::cout << "id name ChessV4" << std::endl;
			std::cout << "id author EnohMihulet" << std::endl;
			std::cout << "uciok" << std::endl;
		}

		else if (command == "isready") {
			std::cout << "readyok" << std::endl;
		}

		else if (command == "ucinewgame") {
			clearTranspositionTable();
			history.clear();
			gameState.setPosition((std::string) DEFAULT_FEN_POSITION);
		}

		else if (command.rfind("position", 0) == 0) {
			std::istringstream ss(command);
			std::string token;
			ss >> token;
			ss >> token;

			if (token == "startpos") {
				gameState.setPosition((std::string) DEFAULT_FEN_POSITION);
			}
			else if (token == "fen") {
				std::string fen;
				std::getline(ss, fen);
				size_t movesIndex = fen.find(" moves ");
				if (movesIndex != std::string::npos)
					fen = fen.substr(0, movesIndex);
				gameState.setPosition(fen);
			}

			size_t movesPos = command.find("moves");
			if (movesPos != std::string::npos) {
				std::istringstream moves(command.substr(movesPos + 6));
				std::string moveStr;
				while (moves >> moveStr) {
					Move move(gameState, moveStr);
					gameState.makeMove(move, history);
				}
			}
		}

		else if (command.rfind("go", 0) == 0) {
			int16 depth = 5;
			std::istringstream ss(command);
			std::string token;
			while (ss >> token) if (token == "depth") ss >> depth;

			Move bestMove = iterativeDeepeningSearch(gameState, history);
			std::cout << gameState.toFenString() << std::endl;
			std::cout << "bestmove " << bestMove.moveToString() << std::endl;
		}

		else if (command == "quit") {
			break;
		}
	}

	return 0;
}
