#include <vector>
#include <sstream>
#include <thread>

#include "../external/imgui/imgui.h"

#include "BoardView.h"

#include "Common.h"
#include "GuiCommon.h"
#include "../chess/GameState.h"
#include "../movegen/MoveGen.h"
#include "../search/Search.h"
#include "Move.h"


void drawFenString(GameState &gameState, GuiState& guiState, char* buf) {
	ImGui::Begin("FEN String");
	if (ImGui::InputText("Current FEN", buf, 256)) {
		gameState.setPosition(std::string(buf));
	
		guiState.allMoves.clear();
		generateAllMoves(gameState, guiState.allMoves, gameState.colorToMove, guiState.checkMask, guiState.pinnedPieces, guiState.pinnedRays);

		initEval(gameState, guiState.staticState, gameState.colorToMove);
		initEval(gameState, guiState.increState, gameState.colorToMove);
	}
	ImGui::End();
}

void drawBoard(GameState& gameState, GuiState& guiState) {
	ImGui::Begin("Board");

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 windowPos = ImGui::GetWindowPos();

	ImVec2 headerMin = ImVec2(windowPos.x, windowPos.y);
	ImVec2 headerMax = ImVec2(windowPos.x + SQUARE_SIZE * COL_COUNT, windowPos.y + 20);
	drawList->AddRectFilled(headerMin, headerMax, BLACK_COLOR);

	for (int16 index = 0; index < 64; index++) {
		int16 rank = index / 8;
		int16 col = index & 7;
		ImVec2 sqMin = ImVec2(windowPos.x + col * SQUARE_SIZE, headerMax.y + (RANK_COUNT - 1 - rank) * SQUARE_SIZE);
		ImVec2 sqMax = ImVec2(sqMin.x + SQUARE_SIZE, sqMin.y + SQUARE_SIZE);

		ImGui::SetCursorScreenPos(sqMin);
		ImGui::PushID(index);
		bool pressed = ImGui::InvisibleButton("tile", ImVec2(SQUARE_SIZE, SQUARE_SIZE));
		ImGui::PopID();

		if (pressed) {
			Piece pieceAtSq = gameState.pieceAt(index);
			if (pieceAtSq != EMPTY && getPieceColor(pieceAtSq) == gameState.colorToMove && guiState.selectedPieceSq != index) showPieceMoves(guiState, index);
			else tryMakeSelectedMove(guiState, gameState, index);
		}

		ImU32 color = ((rank + col) % 2 == 0) ? DARK_SQUARE_COLOR : LIGHT_SQUARE_COLOR ;
		for (Move& move : guiState.selectedPieceMoves) {
			if (guiState.selectedPieceSq == -1) break;
			if (move.getTargetSquare() == index) color = MOVE_HIGHLIGHT_COLOR;
		}

		if (guiState.showCheckMask) {
			if (1ULL << index & guiState.checkMask) color = CHECK_MASK_COLOR;
		}
		if (guiState.showPinnedPieces) {
			if (1ULL << index & guiState.pinnedPieces) color = PINNED_PIECES_COLOR;
		}

		drawList->AddRectFilled(sqMin, sqMax, color);
	}

	if (guiState.showPinnedRays) showPinnedRays(guiState, drawList);

	drawPiecesOnBoard(gameState, drawList);

	drawBoardLabels(drawList);

	ImGui::End();
}

void showPieceMoves(GuiState& guiState, uint8 index) {
	guiState.selectedPieceSq = index;
	guiState.selectedPieceMoves.clear();
	for (Move move : guiState.allMoves) {
		if (move.getStartSquare() == guiState.selectedPieceSq) guiState.selectedPieceMoves.push(move);
	}
}

void tryMakeSelectedMove(GuiState& guiState, GameState& gameState, uint8 index) {
	for (Move& move : guiState.selectedPieceMoves) {
		if (index == move.getTargetSquare() && guiState.selectedPieceSq == move.getStartSquare()) {
			makeGuiMove(gameState, guiState, move);
		}
	}
}

void showPinnedRays(GuiState& guiState, ImDrawList* drawList) {
	ImVec2 windowPos = ImGui::GetWindowPos();

	ImVec2 headerMax = ImVec2(windowPos.x + SQUARE_SIZE * COL_COUNT, windowPos.y + 20);

	for (uint8 i = 0; i < 64; i++) {
		if (guiState.pinnedRays[i] != 0) {
			Bitboard rayBB = guiState.pinnedRays[i];
			while (rayBB) {
				uint8 sq = __builtin_ctzll(rayBB);
				ImVec2 sqMin = ImVec2(windowPos.x + (sq % 8) * SQUARE_SIZE, headerMax.y + (RANK_COUNT - 1 - (sq / 8)) * SQUARE_SIZE);
				ImVec2 sqMax = ImVec2(sqMin.x + SQUARE_SIZE, sqMin.y + SQUARE_SIZE);

				drawList->AddRectFilled(sqMin, sqMax, PINNED_RAYS_COLOR);
				rayBB &= rayBB - 1;
			}
		}
	}
}

void drawPiecesOnBoard(GameState& gameState, ImDrawList* drawList) {
	ImVec2 windowPos = ImGui::GetWindowPos();

	ImVec2 headerMax = ImVec2(windowPos.x + SQUARE_SIZE * COL_COUNT, windowPos.y + 20);

	for (int16 row = 0; row < RANK_COUNT; row++) {
		for (int16 col = 0; col < COL_COUNT; col++) {

			ImVec2 sqMin = ImVec2(windowPos.x + col * SQUARE_SIZE, headerMax.y + (RANK_COUNT - 1 - row) * SQUARE_SIZE);
			uint8 index = (row * COL_COUNT + col);

			const char ch = pieceToChar(gameState.board[index]);
			if (ch != '\0') {
				ImFont* font = ImGui::GetFont();
				float base = ImGui::GetFontSize();
				float scale = 2.0f;
				float fsize = base * scale;
			
				bool is_white_piece = (ch >= 'A' && ch <= 'Z');
				ImU32 piece_color = is_white_piece ? WHITE_COLOR : BLACK_COLOR;
			
				char pieceChar[2] = { ch, '\0' };
			
				ImVec2 textSize = font->CalcTextSizeA(fsize, FLT_MAX, 0.0f, pieceChar);
				ImVec2 text_pos = ImVec2(sqMin.x + 0.5f * (SQUARE_SIZE - textSize.x), sqMin.y + 0.5f * (SQUARE_SIZE - textSize.y));
			
				drawList->AddText(font, fsize, text_pos, piece_color, pieceChar);
			}
		}
	}
}

void drawBoardLabels(ImDrawList* drawList) {
	ImVec2 windowPos = ImGui::GetWindowPos();

	ImVec2 headerMax = ImVec2(windowPos.x + SQUARE_SIZE * COL_COUNT, windowPos.y + 20);


	ImVec2 boardMin = ImVec2(windowPos.x, headerMax.y);
	ImVec2 boardMax = ImVec2(windowPos.x + SQUARE_SIZE * COL_COUNT, headerMax.y + BOARD_HEIGHT);

	float borderThickness = 2.0f;
	drawList->AddRect(boardMin, boardMax, BLACK_COLOR, 0.0f, 0, borderThickness);

	ImFont* font = ImGui::GetFont();
	float base = ImGui::GetFontSize();
	float labelSize = base * 1.4f;

	for (int16 col = 0; col < COL_COUNT; ++col) {
		char fileChar[2] = { static_cast<char>('a' + col), '\0' };

		ImVec2 sqCenterBottom = ImVec2( windowPos.x + col * SQUARE_SIZE + SQUARE_SIZE * 0.5f, headerMax.y + BOARD_HEIGHT + 2.0f);

		ImVec2 fileSize = font->CalcTextSizeA(labelSize, FLT_MAX, 0.0f, fileChar);
		ImVec2 filePosBottom = ImVec2(sqCenterBottom.x - fileSize.x * 0.5f, sqCenterBottom.y);

		ImVec2 sqCenterTop = ImVec2(windowPos.x + col * SQUARE_SIZE + SQUARE_SIZE * 0.5f, headerMax.y - fileSize.y - 2.0f);
		ImVec2 filePosTop = ImVec2(sqCenterTop.x - fileSize.x * 0.5f, sqCenterTop.y);

		drawList->AddText(font, labelSize, filePosBottom, WHITE_COLOR, fileChar);
		drawList->AddText(font, labelSize, filePosTop, WHITE_COLOR, fileChar);
	}

	for (int16 row = 0; row < RANK_COUNT; ++row) {
		char rankChar[2] = { static_cast<char>('1' + row), '\0' };

		float yTopOfRow = headerMax.y + (RANK_COUNT - 1 - row) * SQUARE_SIZE;
		float yCenter = yTopOfRow + SQUARE_SIZE * 0.5f;

		ImVec2 rankSize = font->CalcTextSizeA(labelSize, FLT_MAX, 0.0f, rankChar);

		ImVec2 leftPos = ImVec2(boardMin.x - rankSize.x - 4.0f, yCenter - rankSize.y * 0.5f);
		ImVec2 rightPos = ImVec2(boardMax.x + 4.0f, yCenter - rankSize.y * 0.5f);

		drawList->AddText(font, labelSize, leftPos, WHITE_COLOR, rankChar);
		drawList->AddText(font, labelSize, rightPos, WHITE_COLOR, rankChar);
	}
}

void drawBitboards(GameState& gameState, GuiState& guiState) {
	ImGui::Begin("Bitboard viewer");

	ImGui::Combo("Bitboards", &guiState.bbSelectedIndex, BitboardStrings, IM_ARRAYSIZE(BitboardStrings));
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 windowPos = ImGui::GetWindowPos();

	const float SQUARE_SIZE = 40.0f;
	const int16 numColumns = 8;
	const int16 RANK_COUNT = 8;

	const ImU32 gridColor = ImGui::GetColorU32(ImGuiCol_Border);
	const ImU32 frameColor = ImGui::GetColorU32(ImGuiCol_BorderShadow);
	const float gridThickness = 1.0f;
	const float frameThickness = 2.0f;

	ImVec2 headerMin = ImVec2(windowPos.x, windowPos.y);
	ImVec2 headerMax = ImVec2(windowPos.x + SQUARE_SIZE * numColumns, windowPos.y + 55);

	Bitboard bb = gameState.bitboards[guiState.bbSelectedIndex];

	ImVec2 boardMin = ImVec2(windowPos.x, headerMax.y);
	ImVec2 boardMax = ImVec2(windowPos.x + SQUARE_SIZE * numColumns, headerMax.y + SQUARE_SIZE * RANK_COUNT);
	int16 BOARD_HEIGHT = RANK_COUNT * SQUARE_SIZE;

	drawList->AddRectFilled(boardMin, boardMax, ImGui::GetColorU32(ImGuiCol_WindowBg));

	for (int16 row = 0; row < RANK_COUNT; row++) {
		for (int16 col = 0; col < numColumns; col++) {
			ImVec2 sqMin = ImVec2(windowPos.x + col * SQUARE_SIZE, (BOARD_HEIGHT + headerMax.y) - row * SQUARE_SIZE);
			ImVec2 sqMax = ImVec2(windowPos.x + (col + 1) * SQUARE_SIZE, (BOARD_HEIGHT + headerMax.y) - (row + 1) * SQUARE_SIZE);
			uint8 index = (row * 8 + col);

			ImU32 fill = (bb & (1ULL << index)) ? BLACK_COLOR : WHITE_COLOR;

			drawList->AddRectFilled(sqMin, sqMax, fill);
			drawList->AddRect(sqMin, sqMax, gridColor, 0.0f, 0, gridThickness);
		}
	}

	drawList->AddRect(boardMin, boardMax, frameColor, 0.0f, 0, frameThickness);

	ImGui::End();
}

void drawToggles(GuiState& guiState) {

	ImGui::Checkbox("Show check mask", &guiState.showCheckMask);

	ImGui::Checkbox("Show pinned pieces", &guiState.showPinnedPieces);

	ImGui::Checkbox("Show pinnedRays", &guiState.showPinnedRays);
}

void drawUndoButton(GameState& gameState, GuiState& guiState) {
	if (ImGui::Button("Undo")) {
		if (guiState.movesMade.size() == 0) return;

		unmakeGuiMove(gameState, guiState);

	}
}

void drawSelectPinnedRayIndex(GuiState& guiState) {
	ImGui::Begin("Pinned ray index");

	if (ImGui::BeginTable("Pinned ray index", 8, ImGuiTableFlags_SizingFixedFit)) {

		for (int row = 0; row < 8; ++row) {
			ImGui::TableNextRow();
			for (int col = 0; col < 8; ++col) {
				int v = (7 - row) * 8 + col;

				ImGui::TableSetColumnIndex(col);
				ImGui::PushID(v);

				ImGuiSelectableFlags selectFlags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups;
				bool pressed = ImGui::Selectable(std::to_string(v).c_str(), guiState.selectedRayIndex == v, selectFlags, ImVec2(12, 12));
				ImGui::PopID();

				if (pressed) guiState.selectedRayIndex = v;
			}
		}
		ImGui::EndTable();
	}

	ImGui::End();
}

void drawPinnedRayBitboard(GuiState& guiState) {
	ImGui::Begin("Pinned ray viewer");

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 windowPos = ImGui::GetWindowPos();

	const float SQUARE_SIZE = 40.0f;
	const int16 numColumns = 8;
	const int16 RANK_COUNT = 8;

	const ImU32 gridColor = ImGui::GetColorU32(ImGuiCol_Border);
	const ImU32 frameColor = ImGui::GetColorU32(ImGuiCol_BorderShadow);
	const float gridThickness = 1.0f;
	const float frameThickness = 2.0f;

	ImVec2 headerMin = ImVec2(windowPos.x, windowPos.y);
	ImVec2 headerMax = ImVec2(windowPos.x + SQUARE_SIZE * numColumns, windowPos.y + 20);

	Bitboard bb = guiState.pinnedRays[guiState.selectedRayIndex];

	ImVec2 boardMin = ImVec2(windowPos.x, headerMax.y);
	ImVec2 boardMax = ImVec2(windowPos.x + SQUARE_SIZE * numColumns, headerMax.y + SQUARE_SIZE * RANK_COUNT);
	int16 BOARD_HEIGHT = RANK_COUNT * SQUARE_SIZE;

	drawList->AddRectFilled(boardMin, boardMax, ImGui::GetColorU32(ImGuiCol_WindowBg));

	for (int16 row = 0; row < RANK_COUNT; row++) {
		for (int16 col = 0; col < numColumns; col++) {
			ImVec2 sqMin = ImVec2(windowPos.x + col * SQUARE_SIZE, (BOARD_HEIGHT + headerMax.y) - row * SQUARE_SIZE);
			ImVec2 sqMax = ImVec2(windowPos.x + (col + 1) * SQUARE_SIZE, (BOARD_HEIGHT + headerMax.y) - (row + 1) * SQUARE_SIZE);
			uint8 index = (row * 8 + col);

			ImU32 fill = (bb & (1ULL << index)) ? BLACK_COLOR : WHITE_COLOR;

			drawList->AddRectFilled(sqMin, sqMax, fill);
			drawList->AddRect(sqMin, sqMax, gridColor, 0.0f, 0, gridThickness);
		}
	}

	drawList->AddRect(boardMin, boardMax, frameColor, 0.0f, 0, frameThickness);

	ImGui::End();
}

void drawIsSquareAttacked(GameState& gameState) {
	static int32 sq = 0;
	ImGui::Begin("Is Square Attacked?");
	ImGui::InputInt("index", &sq);
	Color them = gameState.colorToMove == White ? Black : White;
	ImGui::Text(isSquareAttacked(gameState, 1ULL << sq, them) ? "TRUE" : "FALSE");
	ImGui::End();
}


void drawEval(GameState& gameState, GuiState& guiState) {
	ImGui::Begin("Static Eval vs Incremental Eval");

	auto drawEvalTable = [](const char* tableId, const EvalState& ev) {

		auto rowSides = [](const char* label, const int16 (&arr)[2]) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
			ImGui::TableNextColumn(); ImGui::Text("%hd", arr[White]);
			ImGui::TableNextColumn(); ImGui::Text("%hd", arr[Black]);
			ImGui::TableNextColumn(); ImGui::Text("%d", int(arr[White]) - int(arr[Black]));
		};

		if (ImGui::BeginTable(tableId, 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
			ImGui::TableSetupColumn("Category");
			ImGui::TableSetupColumn("White");
			ImGui::TableSetupColumn("Black");
			ImGui::TableSetupColumn("Net (W-B)");
			ImGui::TableHeadersRow();

			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::TextUnformatted("Phase");
			ImGui::TableNextColumn(); ImGui::Text("%hd", ev.core.phase);
			ImGui::TableNextColumn(); ImGui::Text("%hd", TOTAL_PHASE - ev.core.phase);
			ImGui::TableNextColumn(); ImGui::Text("%hd", TOTAL_PHASE);

			rowSides("MG Score", ev.core.mgSide);
			rowSides("EG Score", ev.core.egSide);

			rowSides("Pawn Structure", ev.core.pawnStructure);
			rowSides("King Safety", ev.core.kingSafety);
			rowSides("Bishop Pair", ev.core.bishopPair);
			rowSides("Knight Pair", ev.core.knightPair);
			rowSides("Rook Pair", ev.core.rookPair);
			rowSides("Knight Adj", ev.core.knightAdj);
			rowSides("Rook Adj", ev.core.rookAdj);

			// rowSides("Knight Mobility", ev.core.knightMobility);
			// rowSides("Bishop Mobility", ev.core.bishopMobility);
			// rowSides("Rook Mobility", ev.core.rookMobility);
			// rowSides("Queen Mobility", ev.core.queenMobility);

			ImGui::EndTable();
		}
	};

	{
		std::stringstream ss;
		int16 eval = guiQuiescenceEval(gameState, guiState);
		ss << "Quiescence Eval (for White): " << eval * (gameState.colorToMove == White ? 1 : -1);
		ImGui::Text("%s", ss.str().c_str());
	}

	{
		std::stringstream ss;
		ss << "Static Eval (for White): " << guiState.staticEval;
		ImGui::Text("%s", ss.str().c_str());
		drawEvalTable("Static Eval Details", guiState.staticState);
	}

	{
		std::stringstream ss;
		ss << "Incremental Eval (for White): " << guiState.increEval;
		ImGui::Text("%s", ss.str().c_str());
		drawEvalTable("Incremental Eval Details", guiState.increState);
	}

	ImGui::End();
}


void drawEngineMakeMove(GameState& gameState, GuiState& guiState) {
	ImGui::Begin("Engine Options");

	static bool showStats = false;
	static std::string headerStats;
	static std::string ttStats;
	static std::string perPlyStats;
	static std::string timeStats;

	if (ImGui::Button("Make Engine Move")) {
		Move move = iterativeDeepeningSearch(gameState, guiState.history, headerStats, ttStats, perPlyStats, timeStats);
	
		makeGuiMove(gameState, guiState, move);

		ImGui::Text("%s", move.moveToString().c_str());
		showStats = true;
	}
	ImGui::End();

	if (showStats) {
		if (ImGui::Begin("Header Stats", &showStats)) ImGui::TextUnformatted(headerStats.c_str()); ImGui::End();
		ImGui::Begin("TT Stats"); ImGui::TextUnformatted(ttStats.c_str()); ImGui::End();
		ImGui::Begin("Per Ply Stats"); ImGui::TextUnformatted(perPlyStats.c_str()); ImGui::End();
		ImGui::Begin("Search Times"); ImGui::TextUnformatted(timeStats.c_str()); ImGui::End();
	}
}

void makeGuiMove(GameState& gameState, GuiState& guiState, Move move) {
	updateEval(gameState, move, gameState.colorToMove, guiState.increState, guiState.evalStack);
	guiState.increEval = getEval(guiState.increState, White);

	gameState.makeMove(move, guiState.history);
	guiState.movesMade.push_back(move);

	guiState.staticState = {};
	initEval(gameState, guiState.staticState, White);
	guiState.staticEval = getEval(guiState.staticState, White);

	guiState.selectedPieceSq = -1;
	guiState.selectedPieceMoves.clear();
	guiState.allMoves.clear();

	generateAllMoves(gameState, guiState.allMoves, gameState.colorToMove, guiState.checkMask, guiState.pinnedPieces, guiState.pinnedRays);
}

void unmakeGuiMove(GameState& gameState, GuiState& guiState) {
	Move move = guiState.movesMade[guiState.movesMade.size() - 1];
	gameState.unmakeMove(move, guiState.history);
	undoEvalUpdate(guiState.increState, guiState.evalStack);
	guiState.increEval = getEval(guiState.increState, White);

	guiState.staticState = {};
	initEval(gameState, guiState.staticState, gameState.colorToMove);
	guiState.staticEval = getEval(guiState.staticState, White);

	guiState.movesMade.pop_back();
	guiState.allMoves.clear();
	generateAllMoves(gameState, guiState.allMoves, gameState.colorToMove, guiState.checkMask, guiState.pinnedPieces, guiState.pinnedRays);
}

int16 recursiveQuiescenceEval(GameState& gameState, GuiState& guiState, uint8 pliesFromRoot, int16 alpha, int16 beta) {
	int16 staticEval = getEval(guiState.increState, gameState.colorToMove);
	if (pliesFromRoot >= 5) return staticEval;

	bool isCheck = isSquareAttacked(gameState, gameState.bitboards[gameState.colorToMove == White ? WKing : BKing], gameState.colorToMove == White ? Black : White);

	int16 bestEval = isCheck ? NEG_INF : staticEval;
	if (!isCheck) {
		if (bestEval >= beta) return bestEval;
		if (bestEval > alpha) alpha = bestEval;
	}


	MoveList moves;
	if (isCheck) generateAllMoves(gameState, moves, gameState.colorToMove);
	else generateAllCaptureMoves(gameState, moves, gameState.colorToMove);

	uint16 movesSize = moves.back;

	if (movesSize == 0) return isCheck ? NEG_INF + pliesFromRoot : 0;

	for (uint16 i = 0; i < movesSize; i++) {
		Move move = moves.list[i];

		updateEval(gameState, move, gameState.colorToMove, guiState.increState, guiState.evalStack);
		gameState.makeMove(move, guiState.history);

		int16 score =recursiveQuiescenceEval(gameState, guiState, pliesFromRoot + 1, -beta, -alpha);

		gameState.unmakeMove(move, guiState.history);
		undoEvalUpdate(guiState.increState, guiState.evalStack);

		if (score >= beta) {
			return score;
		}
		if (score > bestEval) {
			bestEval = score;
		}
		if (score > alpha){
			alpha = score;
		}
	}

	return alpha;
}

int16 guiQuiescenceEval(GameState& gameState, GuiState& guiState) {
	return recursiveQuiescenceEval(gameState, guiState, 0, NEG_INF, POS_INF);
}
