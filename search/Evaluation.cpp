#include "Common.h"
#include "Evaluation.h"
#include "PieceSquareTables.h"

int16 calculateMgWeight(const GameState& gameState) {
	int16 mgWeight = 0;

	mgWeight += __builtin_popcountll(gameState.bitboards[WKnight]);
	mgWeight += __builtin_popcountll(gameState.bitboards[BKnight]);
	mgWeight += __builtin_popcountll(gameState.bitboards[WBishop]);
	mgWeight += __builtin_popcountll(gameState.bitboards[BBishop]);
	mgWeight += 2*__builtin_popcountll(gameState.bitboards[WRook]);
	mgWeight += 2*__builtin_popcountll(gameState.bitboards[BRook]);
	mgWeight += 4*__builtin_popcountll(gameState.bitboards[WQueen]);
	mgWeight += 4*__builtin_popcountll(gameState.bitboards[BQueen]);

	return mgWeight;
}

void initEval(GameState& gameState, EvalState& eval, Color us) {
	eval.core.phase = calculateMgWeight(gameState);

	evaluatePawns(gameState, eval.core, us);
	evaluateKnights(gameState, eval.core, us);
	evaluateBishops(gameState, eval.core, us);
	evaluateRooks(gameState, eval.core, us);
	evaluateQueen(gameState, eval.core, us);
	evaluateKing(gameState, eval.core, us);
}

void evaluatePawns(GameState& gameState, EvalCore& eval, Color us) {
	Color them = us == White ? Black : White;

	Bitboard allyPawns = gameState.bitboards[us == White ? WPawn : BPawn];
	Bitboard enemyPawns = gameState.bitboards[us == White ? BPawn : WPawn];
	Bitboard bb = allyPawns;
	while (bb) {
		uint8 sq = __builtin_ctzll(bb);
		sq = us == White ? sq : sq ^ 56;
		eval.mgSide[us] += MG_PSQT[WPawn][sq];
		eval.egSide[us] += EG_PSQT[WPawn][sq];
		eval.mgSide[us] += MG_PIECE_VALUES[WPawn];
		eval.egSide[us] += EG_PIECE_VALUES[WPawn];

		bb &= bb - 1;
	}

	bb = enemyPawns;
	while (bb) {
		uint8 sq = __builtin_ctzll(bb);
		sq = them == White ? sq : sq ^ 56;
		eval.mgSide[them] += MG_PSQT[WPawn][sq];
		eval.egSide[them] += EG_PSQT[WPawn][sq];
		eval.mgSide[them] += MG_PIECE_VALUES[WPawn];
		eval.egSide[them] += EG_PIECE_VALUES[WPawn];

		bb &= bb - 1;
	}

//	updatePawnStructureScore(gameState, delta, move);

	return;
}

void evaluateKnights(GameState& gameState, EvalCore& eval, Color us) {
	Color them = us == White ? Black : White;

	Bitboard allyKnights = gameState.bitboards[us == White ? WKnight : BKnight];
	Bitboard enemyKnights = gameState.bitboards[us == White ? BKnight : WKnight];
	
	uint8 allyPawnCount = __builtin_popcountll(gameState.bitboards[us == White ? WPawn : BPawn]); 
	uint8 enemyPawnCount = __builtin_popcountll(gameState.bitboards[them == White ? WPawn : BPawn]); 
	assert(allyPawnCount >= 0 && allyPawnCount <= 8 && enemyPawnCount >= 0 && enemyPawnCount <= 8);

	Bitboard bb = allyKnights;
	while (bb) {
		uint8 sq = __builtin_ctzll(bb);
		sq = us == White ? sq : sq ^ 56;
		eval.mgSide[us] += MG_PSQT[WKnight][sq];
		eval.egSide[us] += EG_PSQT[WKnight][sq];
		eval.mgSide[us] += MG_PIECE_VALUES[WKnight];
		eval.egSide[us] += EG_PIECE_VALUES[WKnight];

		eval.knightAdj[us] += KNIGHT_ADJUSTMENT[allyPawnCount];

		bb &= bb - 1;
	}

	bb = enemyKnights;
	while (bb) {
		uint8 sq = __builtin_ctzll(bb);
		sq = them == White ? sq : sq ^ 56;
		eval.mgSide[them] += MG_PSQT[WKnight][sq];
		eval.egSide[them] += EG_PSQT[WKnight][sq];
		eval.mgSide[them] += MG_PIECE_VALUES[WKnight];
		eval.egSide[them] += EG_PIECE_VALUES[WKnight];

		eval.knightAdj[them] += KNIGHT_ADJUSTMENT[enemyPawnCount];

		bb &= bb - 1;
	}

	if (__builtin_popcountll(allyKnights) >= 2) eval.knightPair[us] = KNIGHT_PAIR;
	if (__builtin_popcountll(enemyKnights) >= 2) eval.knightPair[them] = KNIGHT_PAIR;

	return;
}

void evaluateBishops(GameState& gameState, EvalCore& eval, Color us) {
	Color them = us == White ? Black : White;

	Bitboard allyBishops = gameState.bitboards[us == White ? WBishop : BBishop];
	Bitboard enemyBishops = gameState.bitboards[us == White ? BBishop : WBishop];
	Bitboard bb = allyBishops;
	while (bb) {
		uint8 sq = __builtin_ctzll(bb);
		sq = us == White ? sq : sq ^ 56;
		eval.mgSide[us] += MG_PSQT[WBishop][sq];
		eval.egSide[us] += EG_PSQT[WBishop][sq];
		eval.mgSide[us] += MG_PIECE_VALUES[WBishop];
		eval.egSide[us] += EG_PIECE_VALUES[WBishop];

		bb &= bb - 1;
	}

	bb = enemyBishops;
	while (bb) {
		uint8 sq = __builtin_ctzll(bb);
		sq = them == White ? sq : sq ^ 56;
		eval.mgSide[them] += MG_PSQT[WBishop][sq];
		eval.egSide[them] += EG_PSQT[WBishop][sq];
		eval.mgSide[them] += MG_PIECE_VALUES[WBishop];
		eval.egSide[them] += EG_PIECE_VALUES[WBishop];

		bb &= bb - 1;
	}

	if (__builtin_popcountll(allyBishops) >= 2) eval.bishopPair[us] = BISHOP_PAIR;
	if (__builtin_popcountll(enemyBishops) >= 2) eval.bishopPair[them] = BISHOP_PAIR;

	return;
}

void evaluateRooks(GameState& gameState, EvalCore& eval, Color us) {
	Color them = us == White ? Black : White;

	Bitboard allyRooks = gameState.bitboards[us == White ? WRook : BRook];
	Bitboard enemyRooks = gameState.bitboards[us == White ? BRook: WRook];
	uint8 allyPawnCount = __builtin_popcountll(gameState.bitboards[us == White ? WPawn : BPawn]); 
	uint8 enemyPawnCount = __builtin_popcountll(gameState.bitboards[them == White ? WPawn : BPawn]); 
	assert(allyPawnCount >= 0 && allyPawnCount <= 8 && enemyPawnCount >= 0 && enemyPawnCount <= 8);

	Bitboard bb = allyRooks;
	while (bb) {
		uint8 sq = __builtin_ctzll(bb);
		sq = us == White ? sq : sq ^ 56;
		eval.mgSide[us] += MG_PSQT[WRook][sq];
		eval.egSide[us] += EG_PSQT[WRook][sq];
		eval.mgSide[us] += MG_PIECE_VALUES[WRook];
		eval.egSide[us] += EG_PIECE_VALUES[WRook];

		eval.rookAdj[us] += ROOK_ADJUSTMENT[allyPawnCount];

		bb &= bb - 1;
	}

	bb = enemyRooks;
	while (bb) {
		uint8 sq = __builtin_ctzll(bb);
		sq = them == White ? sq : sq ^ 56;
		eval.mgSide[them] += MG_PSQT[WRook][sq];
		eval.egSide[them] += EG_PSQT[WRook][sq];
		eval.mgSide[them] += MG_PIECE_VALUES[WRook];
		eval.egSide[them] += EG_PIECE_VALUES[WRook];

		eval.rookAdj[them] += ROOK_ADJUSTMENT[enemyPawnCount];

		bb &= bb - 1;
	}

	if (__builtin_popcountll(allyRooks) >= 2) eval.rookPair[us] = ROOK_PAIR;
	if (__builtin_popcountll(enemyRooks) >= 2) eval.rookPair[them] = ROOK_PAIR;

	return;
}

void evaluateQueen(GameState& gameState, EvalCore& eval, Color us) {
	Color them = us == White ? Black : White;

	Bitboard allyQueens = gameState.bitboards[us == White ? WQueen : BQueen];
	Bitboard enemyQueens = gameState.bitboards[us == White ? BQueen : WQueen];
	Bitboard bb = allyQueens;
	while (bb) {
		uint8 sq = __builtin_ctzll(bb);
		sq = us == White ? sq : sq ^ 56;
		eval.mgSide[us] += MG_PSQT[WQueen][sq];
		eval.egSide[us] += EG_PSQT[WQueen][sq];
		eval.mgSide[us] += MG_PIECE_VALUES[WQueen];
		eval.egSide[us] += EG_PIECE_VALUES[WQueen];

		bb &= bb - 1;
	}

	bb = enemyQueens;
	while (bb) {
		uint8 sq = __builtin_ctzll(bb);
		sq = them == White ? sq : sq ^ 56;
		eval.mgSide[them] += MG_PSQT[WQueen][sq];
		eval.egSide[them] += EG_PSQT[WQueen][sq];
		eval.mgSide[them] += MG_PIECE_VALUES[WQueen];
		eval.egSide[them] += EG_PIECE_VALUES[WQueen];

		bb &= bb - 1;
	}

	return;
}

void evaluateKing(GameState& gameState, EvalCore& eval, Color us) {
	Color them = us == White ? Black : White;

	Bitboard allyKing = gameState.bitboards[us == White ? WKing : BKing];
	Bitboard enemyKing = gameState.bitboards[us == White ? BKing : WKing];
	Bitboard allyPawns, enemyPawns;
	assert(allyKing);
	assert(enemyKing);

	// Ally
	uint8 sq = __builtin_ctzll(allyKing);
	int8 rank = sq / 8;
	int8 file = sq & 7;
	bool checkPawnShield = false;

	if (us == White) {
		allyPawns = gameState.bitboards[WPawn];
		enemyPawns = gameState.bitboards[BPawn];
		checkPawnShield = rank < 2 && (file < 3 || file > 4);
	}
	else {
		sq ^= 56;
		allyPawns = gameState.bitboards[BPawn];
		enemyPawns = gameState.bitboards[WPawn];
		checkPawnShield = rank > 5 && (file < 3 || file > 4);
	}

	eval.mgSide[us] += MG_PSQT[WKing][sq];
	eval.egSide[us] += EG_PSQT[WKing][sq];
	eval.mgSide[us] += MG_PIECE_VALUES[WKing];
	eval.egSide[us] += EG_PIECE_VALUES[WKing];

	if (checkPawnShield) updateKingShieldScore(eval, allyPawns, file, 1, us);

	// Enemy
	sq = __builtin_ctzll(enemyKing);
	rank = sq / 8;
	file = sq & 7;
	checkPawnShield = false;
	if (them == White) checkPawnShield = rank < 2 && (file < 3 || file > 4);
	else {
		sq ^= 56;
		checkPawnShield = rank > 5 && (file < 3 || file > 4);
	}

	eval.mgSide[them] += MG_PSQT[WKing][sq];
	eval.egSide[them] += EG_PSQT[WKing][sq];
	eval.mgSide[them] += MG_PIECE_VALUES[WKing];
	eval.egSide[them] += EG_PIECE_VALUES[WKing];

	if (checkPawnShield) updateKingShieldScore(eval, enemyPawns, file, 1, them);

	
	return;
}

void updateEval(GameState& gameState, Move move, Color us, EvalState& eval, std::vector<EvalDelta>& evalStack) {
	EvalDelta delta{};
	Piece moved = gameState.pieceAt(move.getStartSquare());
	Piece capture = move.isEnPassant() ? us == White ? BPawn : WPawn : gameState.pieceAt(move.getTargetSquare());

	switch (moved) {
		case WPawn: case BPawn:
			updatePawnScore(gameState, delta.core, move, us);
			break;
		case WKnight: case BKnight:
			updateKnightScore(gameState, delta.core, move, us);
			break;
		case WBishop: case BBishop:
			updateBishopScore(gameState, delta.core, move, us);
			break;
		case WRook: case BRook:
			updateRookScore(gameState, delta.core, move, us);
			break;
		case WQueen: case BQueen:
			updateQueenScore(gameState, delta.core, move, us);
			break;
		case WKing: case BKing:
			updateKingScore(gameState, delta.core, move, us);
			break;
		default: break;
	}

	switch (capture) {
		case WPawn: case BPawn:
			updatePawnScore(gameState, delta.core, move, us, true);
			break;
		case WKnight: case BKnight:
			delta.core.phase -= MG_WEIGHT_TABLE[capture];
			updateKnightScore(gameState, delta.core, move, us, true);
			break;
		case WBishop: case BBishop:
			delta.core.phase -= MG_WEIGHT_TABLE[capture];
			updateBishopScore(gameState, delta.core, move, us, true);
			break;
		case WRook: case BRook:
			delta.core.phase -= MG_WEIGHT_TABLE[capture];
			updateRookScore(gameState, delta.core, move, us, true);
			break;
		case WQueen: case BQueen:
			delta.core.phase -= MG_WEIGHT_TABLE[capture];
			updateQueenScore(gameState, delta.core, move, us, true);
			break;
		default: break;
	}

	applyEvalDelta(eval, delta);
	evalStack.push_back(delta);
}

void applyEvalDelta(EvalState& evalState, EvalDelta& evalDelta) {
	evalState.core.phase += evalDelta.core.phase;
	for (uint8 c = 0; c < 2; c++) {
		evalState.core.mgSide[c] += evalDelta.core.mgSide[c];
		evalState.core.egSide[c] += evalDelta.core.egSide[c];
		evalState.core.bishopPair[c] += evalDelta.core.bishopPair[c];
		evalState.core.knightPair[c] += evalDelta.core.knightPair[c];
		evalState.core.rookPair[c] += evalDelta.core.rookPair[c];
		evalState.core.kingSafety[c] += evalDelta.core.kingSafety[c];
		evalState.core.pawnStructure[c] += evalDelta.core.pawnStructure[c];
		evalState.core.knightAdj[c] += evalDelta.core.knightAdj[c];
		evalState.core.rookAdj[c] += evalDelta.core.rookAdj[c];
	}
}

void undoEvalUpdate(EvalState& evalState, std::vector<EvalDelta>& evalStack) {
	EvalDelta evalDelta = evalStack.back();
	evalStack.pop_back();
	evalState.core.phase -= evalDelta.core.phase;
	for (uint8 c = 0; c < 2; c++) {
		evalState.core.mgSide[c] -= evalDelta.core.mgSide[c];
		evalState.core.egSide[c] -= evalDelta.core.egSide[c];
		evalState.core.bishopPair[c] -= evalDelta.core.bishopPair[c];
		evalState.core.knightPair[c] -= evalDelta.core.knightPair[c];
		evalState.core.rookPair[c] -= evalDelta.core.rookPair[c];
		evalState.core.kingSafety[c] -= evalDelta.core.kingSafety[c];
		evalState.core.pawnStructure[c] -= evalDelta.core.pawnStructure[c];
		evalState.core.knightAdj[c] -= evalDelta.core.knightAdj[c];
		evalState.core.rookAdj[c] -= evalDelta.core.rookAdj[c];
	}
}

int16 getEval(EvalState& eval, Color us) {
	Color them = us == White ? Black : White;
	int16 mgPhase = std::min((int16)TOTAL_PHASE, eval.core.phase);
	int16 egPhase = TOTAL_PHASE - mgPhase;
	int16 score = (mgPhase * (eval.core.mgSide[us] - eval.core.mgSide[them]) + egPhase * (eval.core.egSide[us] - eval.core.egSide[them])) / TOTAL_PHASE;
	score += eval.core.bishopPair[us] - eval.core.bishopPair[them] + eval.core.knightPair[us] - eval.core.knightPair[them] + eval.core.rookPair[us] - eval.core.rookPair[them];
	score += (eval.core.pawnStructure[us] - eval.core.pawnStructure[them]) + (eval.core.kingSafety[us] - eval.core.kingSafety[them]);
	score += (eval.core.knightAdj[us] - eval.core.knightAdj[them]) + (eval.core.rookAdj[us] - eval.core.rookAdj[them]);
	return score;
}

void updatePawnScore(GameState& gameState, EvalCore& delta, Move move, Color us, bool captured) {
	Color them = us == White ? Black : White;
	uint8 from = move.getStartSquare();
	uint8 to = move.getTargetSquare();

	if (captured) {
		if (move.isEnPassant()) to += us == White ? -8 : 8;
		delta.mgSide[them] -= MG_PSQT[WPawn][them == White ? to : to ^ 56];
		delta.egSide[them] -= EG_PSQT[WPawn][them == White ? to : to ^ 56];
		delta.mgSide[them] -= MG_PIECE_VALUES[WPawn];
		delta.egSide[them] -= EG_PIECE_VALUES[WPawn];

		delta.knightAdj[them] -= __builtin_popcountll(gameState.bitboards[them == White ? WKnight : BKnight]) * KNIGHT_ADJUSTMENT_PER;
		delta.rookAdj[them] -= __builtin_popcountll(gameState.bitboards[them == White ? WRook : BRook]) * ROOK_ADJUSTMENT_PER;

//		// If a pawn captured this pawn, the structure is already updated
//		if (gameState.pieceAt(move.getStartSquare()) != them == Black ? WPawn : BPawn) {
//			updatePawnStructureScore(gameState, delta, move, true);
//		}
		return;
	}

	if (move.isPromotion()) {
		delta.mgSide[us] -= MG_PIECE_VALUES[WPawn];
		delta.egSide[us] -= EG_PIECE_VALUES[WPawn];

		Bitboard knights = gameState.bitboards[us == White ? WKnight : BKnight];
		Bitboard rooks = gameState.bitboards[us == White ? WRook : BRook];
		int knightCount = __builtin_popcountll(knights);
		int rookCount = __builtin_popcountll(rooks);

		delta.knightAdj[us] -= knightCount*KNIGHT_ADJUSTMENT_PER;
		delta.rookAdj[us] -= rookCount*ROOK_ADJUSTMENT_PER;

		if (move.isQueenPromotion()) {
			delta.phase += 4; // NOTE: This is temporary so that the assertion that a static eval equals the incremental eval holds
			delta.mgSide[us] += MG_PIECE_VALUES[WQueen];
			delta.egSide[us] += EG_PIECE_VALUES[WQueen];
			delta.mgSide[us] += MG_PSQT[WQueen][us == White ? to : to ^ 56] - MG_PSQT[WPawn][us == White ? from : from ^ 56];
			delta.egSide[us] += EG_PSQT[WQueen][us == White ? to : to ^ 56] - EG_PSQT[WPawn][us == White ? from : from ^ 56];
		}
		else if (move.isRookPromotion()) {
			delta.phase += 2; // NOTE: This is temporary so that the assertion that a static eval equals the incremental eval holds
			delta.mgSide[us] += MG_PIECE_VALUES[WRook];
			delta.egSide[us] += EG_PIECE_VALUES[WRook];
			delta.mgSide[us] += MG_PSQT[WRook][us == White ? to : to ^ 56] - MG_PSQT[WPawn][us == White ? from : from ^ 56];
			delta.egSide[us] += EG_PSQT[WRook][us == White ? to : to ^ 56] - EG_PSQT[WPawn][us == White ? from : from ^ 56];

			if (rookCount == 1) delta.rookPair[us] += ROOK_PAIR;
		}
		else if (move.isKnightPromotion()) {
			delta.phase += 1; // NOTE: This is temporary so that the assertion that a static eval equals the incremental eval holds
			delta.mgSide[us] += MG_PIECE_VALUES[WKnight];
			delta.egSide[us] += EG_PIECE_VALUES[WKnight];
			delta.mgSide[us] += MG_PSQT[WKnight][us == White ? to : to ^ 56] - MG_PSQT[WPawn][us == White ? from : from ^ 56];
			delta.egSide[us] += EG_PSQT[WKnight][us == White ? to : to ^ 56] - EG_PSQT[WPawn][us == White ? from : from ^ 56];

			if (knightCount == 1) delta.knightPair[us] += KNIGHT_PAIR;
		}
		else if (move.isBishopPromotion()) {
			delta.phase += 1; // NOTE: This is temporary so that the assertion that a static eval equals the incremental eval holds
			delta.mgSide[us] += MG_PIECE_VALUES[WBishop];
			delta.egSide[us] += EG_PIECE_VALUES[WBishop];
			delta.mgSide[us] += MG_PSQT[WBishop][us == White ? to : to ^ 56] - MG_PSQT[WPawn][us == White ? from : from ^ 56];
			delta.egSide[us] += EG_PSQT[WBishop][us == White ? to : to ^ 56] - EG_PSQT[WPawn][us == White ? from : from ^ 56];

			Bitboard bishops = gameState.bitboards[us == White ? WBishop : BBishop];
			int pre = __builtin_popcountll(bishops);
			if (pre == 1) delta.bishopPair[us] += BISHOP_PAIR;
		}
	}
	else {
		delta.mgSide[us] += MG_PSQT[WPawn][us == White ? to : to ^ 56] - MG_PSQT[WPawn][us == White ? from : from ^ 56];
		delta.egSide[us] += EG_PSQT[WPawn][us == White ? to : to ^ 56] - EG_PSQT[WPawn][us == White ? from : from ^ 56];
	}

	updateKingShieldPawnMove(gameState, delta, move, us);
//	updatePawnStructureScore(gameState, delta, move);

	return;
}

void updatePawnStructureScore(GameState& gameState, EvalCore& delta, Move move, Color us, bool captured) {
	// TODO: Incrementally update structure score
	Bitboard wPawns = gameState.bitboards[WPawn];
	Bitboard bPawns = gameState.bitboards[BPawn];
	delta.pawnStructure[White] = 0;
	delta.pawnStructure[Black] = 0;

	Bitboard bb = wPawns;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		uint16 file = sq & 7;
		uint16 rank = sq / 8;

		Bitboard filesMask = FILES[file];
		if (file == 0) filesMask |= FILES[file+1];
		else if (file == 7) filesMask |= FILES[file-1]; 
		else filesMask |= FILES[file-1] | FILES[file+1];

		Bitboard ranksMask = 0;
		for (int i = rank + 1; i <= 7; i++) {
			ranksMask |= RANKS[i];
		}

		if ((bPawns & filesMask & ranksMask) == 0) {
			delta.pawnStructure[White] += PASSED_PAWNS[rank];
		}
		if (__builtin_popcountll(wPawns & filesMask) == 1) {
			delta.pawnStructure[White] += ISOLATED_PAWNS;
		}
		if (__builtin_popcountll(wPawns & FILES[file]) > 1) {
			delta.pawnStructure[White] += DOUBLED_PAWNS;
		}
		
		bb &= bb - 1;
	}

	bb = bPawns;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		uint16 file = sq & 7;
		uint16 rank = sq / 8;

		Bitboard filesMask = FILES[file];
		if (file == 0) filesMask |= FILES[file+1];
		else if (file == 7) filesMask |= FILES[file-1]; 
		else filesMask |= FILES[file-1] | FILES[file+1];

		Bitboard ranksMask = 0;
		for (int i = rank - 1; i >= 0; i--) {
			ranksMask |= RANKS[i];
		}

		if ((wPawns & filesMask & ranksMask) == 0) {
			delta.pawnStructure[Black] = PASSED_PAWNS[7 - rank];
		}
		if ((wPawns & (filesMask ^ FILES[file])) == 0) {
			delta.pawnStructure[Black] = ISOLATED_PAWNS;
		}
		if (__builtin_popcountll(bPawns & FILES[file]) > 1) {
			delta.pawnStructure[Black] = DOUBLED_PAWNS;
		}

		bb &= bb - 1;
	}
	

}

void updateKnightScore(GameState& gameState, EvalCore& delta, Move move, Color us, bool captured) {
	Color them = us == White ? Black : White;
	uint8 from = move.getStartSquare();
	uint8 to = move.getTargetSquare();

	if (captured) {
		delta.mgSide[them] -= MG_PSQT[WKnight][them == White ? to : to ^ 56];
		delta.egSide[them] -= EG_PSQT[WKnight][them == White ? to : to ^ 56];
		delta.mgSide[them] -= MG_PIECE_VALUES[WKnight];
		delta.egSide[them] -= EG_PIECE_VALUES[WKnight];

		int16 pawnCount = __builtin_popcountll(gameState.bitboards[them == White ? WPawn : BPawn]);
		delta.knightAdj[them] -= KNIGHT_ADJUSTMENT[pawnCount];

		Bitboard knights = gameState.bitboards[them == White ? WKnight : BKnight];
		int pre = __builtin_popcountll(knights);
		if (pre == 2) delta.knightPair[them] += -KNIGHT_PAIR;
	}
	else {
		delta.mgSide[us] += MG_PSQT[WKnight][us == White ? to : to ^ 56] - MG_PSQT[WKnight][us == White ? from : from ^ 56];
		delta.egSide[us] += EG_PSQT[WKnight][us == White ? to : to ^ 56] - EG_PSQT[WKnight][us == White ? from : from ^ 56];
	}
}

void updateBishopScore(GameState& gameState, EvalCore& delta, Move move, Color us, bool captured) {
	Color them = us == White ? Black : White;
	uint8 from = move.getStartSquare();
	uint8 to = move.getTargetSquare();

	if (captured) {
		delta.mgSide[them] -= MG_PSQT[WBishop][them == White ? to : to ^ 56];
		delta.egSide[them] -= EG_PSQT[WBishop][them == White ? to : to ^ 56];
		delta.mgSide[them] -= MG_PIECE_VALUES[WBishop];
		delta.egSide[them] -= EG_PIECE_VALUES[WBishop];

		Bitboard bishops = gameState.bitboards[them == White ? WBishop : BBishop];
		int pre = __builtin_popcountll(bishops);
		if (pre == 2) delta.bishopPair[them] += -BISHOP_PAIR;
	}
	else {
		delta.mgSide[us] += MG_PSQT[WBishop][us == White ? to : to ^ 56] - MG_PSQT[WBishop][us == White ? from : from ^ 56];
		delta.egSide[us] += EG_PSQT[WBishop][us == White ? to : to ^ 56] - EG_PSQT[WBishop][us == White ? from : from ^ 56];
	}
}

void updateRookScore(GameState& gameState, EvalCore& delta, Move move, Color us, bool captured) {
	// TODO: Open file bonus (on pawn capture or rook move update)
	Color them = us == White ? Black : White;
	uint8 from = move.getStartSquare();
	uint8 to = move.getTargetSquare();

	if (captured) {
		delta.mgSide[them] -= MG_PSQT[WRook][them == White ? to : to ^ 56];
		delta.egSide[them] -= EG_PSQT[WRook][them == White ? to : to ^ 56];
		delta.mgSide[them] -= MG_PIECE_VALUES[WRook];
		delta.egSide[them] -= EG_PIECE_VALUES[WRook];

		int16 pawnCount = __builtin_popcountll(gameState.bitboards[them == White ? WPawn : BPawn]);
		delta.rookAdj[them] -= ROOK_ADJUSTMENT[pawnCount];

		Bitboard rooks = gameState.bitboards[them == White ? WRook : BRook];
		int pre = __builtin_popcountll(rooks);
		if (pre == 2) delta.rookPair[them] += -ROOK_PAIR;
	}
	else {
		delta.mgSide[us] += MG_PSQT[WRook][us == White ? to : to ^ 56] - MG_PSQT[WRook][us == White ? from : from ^ 56];
		delta.egSide[us] += EG_PSQT[WRook][us == White ? to : to ^ 56] - EG_PSQT[WRook][us == White ? from : from ^ 56];
	}
}

void updateQueenScore(GameState& gameState, EvalCore& delta, Move move, Color us, bool captured) {
	Color them = us == White ? Black : White;
	uint8 from = move.getStartSquare();
	uint8 to = move.getTargetSquare();

	if (captured) {
		delta.mgSide[them] -= MG_PSQT[WQueen][them == White ? to : to ^ 56];
		delta.egSide[them] -= EG_PSQT[WQueen][them == White ? to : to ^ 56];
		delta.mgSide[them] -= MG_PIECE_VALUES[WQueen];
		delta.egSide[them] -= EG_PIECE_VALUES[WQueen];
	}
	else {
		delta.mgSide[us] += MG_PSQT[WQueen][us == White ? to : to ^ 56] - MG_PSQT[WQueen][us == White ? from : from ^ 56];
		delta.egSide[us] += EG_PSQT[WQueen][us == White ? to : to ^ 56] - EG_PSQT[WQueen][us == White ? from : from ^ 56];
	}
}

void updateKingScore(GameState& gameState, EvalCore& delta, Move move, Color us) {
	// TODO: Pawn shield, king safety, castling bonus?
	Color them = us == White ? Black : White;
	uint8 from = move.getStartSquare();
	uint8 to = move.getTargetSquare();

	delta.mgSide[us] += MG_PSQT[WKing][us == White ? to : to ^ 56] - MG_PSQT[WKing][us == White ? from : from ^ 56];
	delta.egSide[us] += EG_PSQT[WKing][us == White ? to : to ^ 56] - EG_PSQT[WKing][us == White ? from : from ^ 56];
	if (move.isKingSideCastle()) {
		delta.mgSide[us] += MG_PSQT[WRook][5] - MG_PSQT[WRook][7];
		delta.egSide[us] += EG_PSQT[WRook][5] - EG_PSQT[WRook][7];
		// delta.kingSafety[us] += CASTLED; NOTE: No way of doing this for static evaluation so it is left out for now
	}
	else if (move.isQueenSideCastle()) {
		delta.mgSide[us] += MG_PSQT[WRook][3] - MG_PSQT[WRook][0];
		delta.egSide[us] += EG_PSQT[WRook][3] - EG_PSQT[WRook][0];
		// delta.kingSafety[us] += CASTLED;
	}
	
	updateKingShieldKingMove(gameState, delta, from, to, us);
}

void updateKingShieldScore(EvalCore& core, Bitboard& allyPawns, uint8 file, int8 sign, Color us) {
	uint8 pawnRank1, pawnRank2;

	if (us == White)  {
		pawnRank1 = 1;
		pawnRank2 = 2;
	}
	else {
		pawnRank1 = 6;
		pawnRank2 = 5;
	}

	Bitboard strongShieldPawns = allyPawns & RANKS[pawnRank1];
	Bitboard midShieldPawns = allyPawns & RANKS[pawnRank2];

	core.kingSafety[us] += sign * STRONG_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file]);
	core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file]);

	switch (file) {
	case 0:
		core.kingSafety[us] += sign * STRONG_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file+1]);
		core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file+1]);
		core.kingSafety[us] += sign * STRONG_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file+2]); // NOTE: Should pawn shield include these?
		core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file+2]);
		break;
	case 1: case 6:
		core.kingSafety[us] += sign * STRONG_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file-1]);
		core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file-1]);
		core.kingSafety[us] += sign * STRONG_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file+1]);
		core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file+1]);
		break;
	case 2:
		core.kingSafety[us] += sign * STRONG_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file-1]);
		core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file-1]);
		core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file+1]);
		core.kingSafety[us] += sign * WEAK_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file+1]);

		break;
	case 5:
		core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file-1]);
		core.kingSafety[us] += sign * WEAK_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file-1]);
		core.kingSafety[us] += sign * STRONG_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file+1]);
		core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file+1]);
		break;
	case 7:
		core.kingSafety[us] += sign * STRONG_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file-1]);
		core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file-1]);
		core.kingSafety[us] += sign * STRONG_PAWN_SHIELD * __builtin_popcountll(strongShieldPawns & FILES[file-2]); // NOTE: Should pawn shield include these?
		core.kingSafety[us] += sign * MID_PAWN_SHIELD * __builtin_popcountll(midShieldPawns & FILES[file-2]);
		break;
	}
}

void updateKingShieldKingMove(GameState& gameState, EvalCore& delta, uint8 from, uint8 to, Color us) {
	Bitboard allyPawns;
	uint8 kingSq;

	for (uint8 i = 0; i < 2; i++) {
		kingSq = i==0 ? from : to;
		uint8 file = kingSq & 7;
		uint8 rank = kingSq / 8;
		int8 sign = i == 0 ? -1 : 1;
		bool checkPawnShield = false;
	
		if (us == White) {
			allyPawns = gameState.bitboards[WPawn];
			checkPawnShield = rank < 2 && (file < 3 || file > 4);
		}
		else {
			allyPawns = gameState.bitboards[BPawn];
			checkPawnShield = rank > 5 && (file < 3 || file > 4);
		}
	
		if (checkPawnShield) updateKingShieldScore(delta, allyPawns, file, sign, us);
	}
}

void updateKingShieldPawnMove(GameState& gameState, EvalCore& delta, Move move, Color us) {
	uint8 kingSq = __builtin_ctzll(gameState.bitboards[us == White ? WKing : BKing]);
	
	uint8 file = kingSq & 7;
	uint8 rank = kingSq / 8;
	Piece captured = EMPTY;
	
	for (uint8 i = 0; i < 2; i++) {
		if (i == 1) captured = gameState.tempMakeMove(move);
		int8 sign = i == 0 ? -1 : 1;

		Bitboard allyPawns = gameState.bitboards[us == White ? WPawn : BPawn];
		bool checkPawnShield = false;
		checkPawnShield = us == White ? rank < 2 && (file < 3 || file > 4) : rank > 5 && (file < 3 || file > 4);
	
		if (checkPawnShield) updateKingShieldScore(delta, allyPawns, file, sign, us);
	}

	gameState.tempUnmakeMove(move, captured);
}

