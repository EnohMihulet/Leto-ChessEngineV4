#pragma once

#include <SDL.h>

#include "../chess/GameState.h"
#include "GuiCommon.h"

void drawFenString(GameState& gameState, GuiState& guiState, char* buf);

void drawBoard(GameState& gameState, GuiState& guiState);

void showPieceMoves(GuiState& guiState, uint8 index);

void tryMakeSelectedMove(GuiState& guiState, GameState& gameState, uint8 index);

void showPinnedRays(GuiState& guiState, ImDrawList* drawList);

void drawPiecesOnBoard(GameState& gameState, ImDrawList* drawList);

void drawBoardLabels(ImDrawList* drawList);

void pieceSelector(GameState& gameState, GuiState& guiState);

void drawBitboards(GameState &gameState, GuiState& guiState);

void drawToggles(GuiState& guiState);

void drawUndoButton(GameState& gameState, GuiState& guiState);

void drawSelectPinnedRayIndex(GuiState& guiState);

void drawPinnedRayBitboard(GuiState& guiState);

void drawIsSquareAttacked(GameState& gameState);

void drawEval(GameState& gameState, GuiState& guiState);

void drawEngineMakeMove(GameState& gameState, GuiState& guiState);

void makeGuiMove(GameState& gameState, GuiState& guiState, Move move);

void unmakeGuiMove(GameState& gameState, GuiState& guiState);

int16 guiQuiescenceEval(GameState& gameState, GuiState& guiState);

