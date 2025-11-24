#pragma once

#include "../chess/GameState.h"

constexpr int16 MG_PIECE_VALUES[6] = {82, 337, 365, 477, 1025, 20000};
constexpr int16 EG_PIECE_VALUES[6] = {94, 281, 297, 512, 936, 20000};

constexpr int16 MG_WEIGHT_TABLE[13] = {0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0, 0};

constexpr int16 KNIGHT_ADJUSTMENT[9] = {-20, -16, -12, -8, -4,  0,  4,  8, 12};
constexpr int16 KNIGHT_ADJUSTMENT_PER = 4;
constexpr int16 ROOK_ADJUSTMENT[9] = {15,  12,   9,  6,  3,  0, -3, -6, -9};
constexpr int16 ROOK_ADJUSTMENT_PER = -3;

constexpr int16 BISHOP_PAIR = 40;
constexpr int16 KNIGHT_PAIR = -10;
constexpr int16 ROOK_PAIR = 25;

constexpr int16 PASSED_PAWNS[7] = {5, 10, 15, 20, 35, 60, 100};
constexpr int16 CONNECTED_PAST_PAWNS = 30;
constexpr int16 DOUBLED_PAWNS = -5;
constexpr int16 ISOLATED_PAWNS = -15;
constexpr int16 BACKWARD_PAWN = -10;

constexpr int16 ROOK_OPEN_FILE = 20;
constexpr int16 ROOK_SEMI_OPEN_FILE = 10;

constexpr int16 CASTLED = 15;
constexpr int16 STRONG_PAWN_SHIELD = 15;
constexpr int16 MID_PAWN_SHIELD = 10;
constexpr int16 WEAK_PAWN_SHIELD = 5;
constexpr int16 EXPOSED_KING = -40;

constexpr int16 KNIGHT_MOBILITY_BONUS = 4;
constexpr int16 BISHOP_MOBILITY_BONUS = 3;
constexpr int16 ROOK_MOBILITY_BONUS = 2;
constexpr int16 QUEEN_MOBILITY_BONUS = 1;

constexpr uint16 TOTAL_PHASE = 24;

typedef struct EvalCore {
	int16 mgSide[2] = {0,0};
	int16 egSide[2] = {0,0};
	int16 pawnStructure[2] = {0,0};
	int16 kingSafety[2] = {0,0};
	int16 bishopPair[2] = {0,0};
	int16 knightPair[2] = {0,0};
	int16 rookPair[2] = {0,0};
	int16 knightAdj[2] = {0,0};
	int16 rookAdj[2] = {0,0};
	// int16 knightMobility[2];
	// int16 bishopMobility[2];
	// int16 rookMobility[2];
	// int16 queenMobility[2];
	int16 phase;
} EvalCore;

typedef struct EvalState {
	EvalCore core;
} EvalState;

typedef struct EvalDelta {
	EvalCore core;
} EvalDelta;

void initEval(GameState& gameState, EvalState& eval, Color color);

void evaluatePawns(GameState& gameState, EvalCore& eval, Color us);
void evaluateKnights(GameState& gameState, EvalCore& eval, Color us);
void evaluateBishops(GameState& gameState, EvalCore& eval, Color us);
void evaluateRooks(GameState& gameState, EvalCore& eval, Color us);
void evaluateQueen(GameState& gameState, EvalCore& eval, Color us);
void evaluateKing(GameState& gameState, EvalCore& eval, Color us);

void updateEval(GameState& gameState, Move move, Color us, EvalState& evalState, std::vector<EvalDelta>& evalStack);
void applyEvalDelta(EvalState& evalState, EvalDelta& evalDelta);
void undoEvalUpdate(EvalState& evalState, std::vector<EvalDelta>& evalStack);
int16 getEval(EvalState& eval, Color us);

void updatePawnScore(GameState& gameState, EvalCore& eval, Move move, Color us, bool captured=false);
void updateKnightScore(GameState& gameState, EvalCore& eval, Move move, Color us, bool captured=false);
void updateBishopScore(GameState& gameState, EvalCore& eval, Move move, Color us, bool captured=false);
void updateRookScore(GameState& gameState, EvalCore& eval, Move move, Color us, bool captured=false);
void updateQueenScore(GameState& gameState, EvalCore& eval, Move move, Color us, bool captured=false);
void updateKingScore(GameState& gameState, EvalCore& eval, Move move, Color us);

void updatePawnStructureScore(GameState& gameState, EvalCore& eval, Move move, Color us, bool captured=false);

void updateKingShieldScore(EvalCore& delta, Bitboard& allyPawns, uint8 file, int8 sign, Color us);
void updateKingShieldKingMove(GameState& gameState, EvalCore& delta, uint8 from, uint8 to, Color us);
void updateKingShieldPawnMove(GameState& gameState, EvalCore& delta, Move move, Color us);
