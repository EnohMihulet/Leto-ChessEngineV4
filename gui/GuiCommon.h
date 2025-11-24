#pragma once

#include "../chess/Common.h"
#include "../search/Evaluation.h"

#include "../external/imgui/imgui.h"
#include "Move.h"

constexpr uint32 WINDOW_WIDTH = 1512;
constexpr uint32 WINDOW_HEIGHT = 982;

constexpr float SQUARE_SIZE = 60.0f;
constexpr int16 COL_COUNT = 8;
constexpr int16 RANK_COUNT = 8;
constexpr int16 BOARD_HEIGHT = RANK_COUNT * SQUARE_SIZE;


static const ImU32 DARK_SQUARE_COLOR = IM_COL32(25, 110, 70, 255);
static const ImU32 LIGHT_SQUARE_COLOR = IM_COL32(220, 180, 140, 255);
static const ImU32 BLACK_COLOR = IM_COL32(0, 0, 0, 255);
static const ImU32 WHITE_COLOR = IM_COL32(255, 255, 255, 255);
static const ImU32 MOVE_HIGHLIGHT_COLOR = IM_COL32(255, 170, 70, 255);
static const ImU32 CHECK_MASK_COLOR = IM_COL32(252, 3, 44, 255);
static const ImU32 PINNED_PIECES_COLOR = IM_COL32(255, 0, 247, 255);
static const ImU32 PINNED_RAYS_COLOR = IM_COL32(162, 0, 255, 255);

constexpr int16 BitboardStringsSize = 15;
static const char* BitboardStrings[BitboardStringsSize] = {"WPawns", "WKnight", "WBishop", "WRook", "WQueen", "WKing", "BPawn", "BKnight",
					"BBishop", "BRook", "BQueen", "BKing", "AllIndex", "WhiteIndex", "BlackIndex"};

typedef struct GuiState {
	std::vector<MoveInfo> history;
	std::vector<Move> movesMade;
	std::vector<EvalDelta> evalStack;
	EvalState staticState;
	EvalState increState;
	int32 staticEval;
	int32 increEval;
	MoveList allMoves;
	MoveList selectedPieceMoves;
	Bitboard checkMask; 
	Bitboard pinnedPieces; 
	std::array<Bitboard, 64> pinnedRays;
	int32 bbSelectedIndex;
	int32 selectedPieceSq;
	int32 selectedRayIndex;
	bool showMoves;
	bool showCheckMask;
	bool showPinnedPieces;
	bool showPinnedRays;
} GuiState;
