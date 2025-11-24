#pragma once

#include "../chess/GameState.h"
#include "../search/MoveSorter.h"
#include "Common.h"
#include "Evaluation.h"

constexpr uint64 TIME_PER_MOVE = 5000;
constexpr uint64 MAX_PLY = 30;

typedef struct SearchContext {
	uint64 startTime;
	Move bestMoveThisIteration = 0;
	bool fullSearch = true;
	bool searchCanceled;
} SearchContext;


enum MoveBucket : uint8 {
	B_PV, B_TT, B_Promo, B_GoodCap, B_Killer1, B_Counter, B_FollowUp, B_Killer2, B_QuietHist, B_BadCap, B_Other, B_Count
};

typedef struct SearchStats {
	uint64 nodes = 0;
	uint64 prunedNodes = 0;
	uint64 betaCutOffs = 0;

	uint64 ttProbes = 0;
	uint64 ttHits = 0;
	uint64 ttHitsUseful = 0;
	uint64 ttHitCutoffs = 0;

	uint64 ttStores = 0;
	uint64 ttStoresExact = 0;
	uint64 ttStoresLower = 0;
	uint64 ttStoresUpper = 0;

	uint64 plyNodes[MAX_PLY] = {};
	uint64 legalMoves[MAX_PLY] = {};
	uint64 cutoffCount[MAX_PLY] = {};
	uint64 cutoffIndexSum[MAX_PLY] = {};
	uint64 firstMoveCutoffs[MAX_PLY] = {};

	uint64 bucketTried[B_Count] = {};
	uint64 bucketCutoffs[B_Count] = {};
	uint64 bucketIndexSum[B_Count] = {};
	uint64 bucketFirstCutoffs[B_Count] = {};
} SearchStats;

typedef struct SearchTimes {
	uint64 total = 0;

	uint64 transpositionLookUp = 0;
	uint64 transpositionInsertion = 0;

	uint64 evaluation = 0;
	uint64 gameResultCheck = 0;
	uint64 pickContextSetup = 0;

	uint64 moveGeneration = 0;
	uint64 moveScoring = 0;
	uint64 movePicking = 0;

	uint64 moveMaking = 0;
	uint64 moveUnmaking = 0;

	uint64 repetitionPush = 0;
	uint64 repetitionPop = 0;
} SearchTimes;

typedef struct MovePool {
	std::array<MoveList, MAX_PLY> pool;

	MoveList& getMoveList(uint8 depth) {
		pool[depth].clear();
		return pool[depth];
	}
} MovePool;

typedef struct MoveScorePool {
	std::array<ScoreList, MAX_PLY> pool;

	ScoreList& getScoreList(uint8 depth) {
		pool[depth].clear();
		return pool[depth];
	}

} MoveScorePool;

typedef struct QuiescencePool {
	std::array<MoveList, 5> pool;

	MoveList& getMoveList(uint8 depth) {
		pool[depth].clear();
		return pool[depth];
	}
} QuiescencePool;

constexpr std::array<std::array<uint8, MAX_MOVE_COUNT>, MAX_PLY> generateLateMoveReduction() {
	std::array<std::array<uint8, MAX_MOVE_COUNT>, MAX_PLY> r;
	for (uint8 ply = 0; ply < MAX_PLY; ply++) {
		for (uint8 m = 0; m < MAX_MOVE_COUNT; m++) {
			if (ply < 5 || m < 2) {
				r[ply][m] = 0;
				continue;
			}
			uint8 lp = uint32_log2(ply);
			uint8 lm = uint32_log2(m);
			uint8 reduction = lp * lm / 4;

			if (lp >= 3 || lm >= 3) reduction += 1;

			reduction = std::min(reduction, (uint8)3);
			uint8 maxPly = (ply > 2 ? ply - 2 : 0);
			r[ply][m] = std::min(reduction, maxPly);
		}
	}
	return r;
}

constexpr std::array<std::array<uint8, MAX_MOVE_COUNT>, MAX_PLY> LMR_TABLE = generateLateMoveReduction();

int16 quiescenceSearch(GameState& gameState, EvalState& evalState, std::vector<MoveInfo>& history, Move pvMove, int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining);

Move iterativeDeepeningSearch(GameState& gameState, std::vector<MoveInfo>& history);

// Used for GUI
Move iterativeDeepeningSearch(GameState& gameState, std::vector<MoveInfo>& history, std::string& headerStats, std::string& TTStats, std::string& perPlyStats, std::string& searchTimes);

// Debug version
int16 alphaBetaSearch(GameState& gameState, EvalState& evalState, std::vector<MoveInfo>& history, SearchContext& context, 
			  int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining, SearchStats& stats, SearchTimes& times);

// Release version
int16 alphaBetaSearch(GameState& gameState, EvalState& evalState, std::vector<MoveInfo>& history, SearchContext& context, 
			  int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining);

void clearTranspositionTable();

uint8 getLMR(Move move, uint8 depth, uint8 moveNum, bool isCheck, bool inPV, Move ttMove, MTEntry killers, uint16 histScore);

MoveBucket getBucketType(uint16 score);

std::string getHeaderSearchStats(const SearchStats& s, int16 depth, const Move& bestMove, double elapsed_ms, uint64 zobrist);

std::string getTTSearchStats(const SearchStats& s);

std::string getPerPlySearchStats(const SearchStats& s);

void printSearchStats(const SearchStats& s, int16 depth, const Move& bestMove, double elapsed_ms, uint64 zobrist);

std::string getSearchTimes(const SearchTimes& t);

void printSearchTimes(const SearchTimes& t);
