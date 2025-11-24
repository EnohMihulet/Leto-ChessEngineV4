#pragma once

#include "../chess/Common.h"
#include "../chess/Move.h"
#include "Search.h"

enum NodeType { Exact, UpperBound, LowerBound };
enum LookUpType {None, Score, AlphaIncrease, BetaIncrease};

constexpr uint32 TABLE_SIZE = 524288;
constexpr int16 SCORE_SENTINAL = -3103;

constexpr int16 MATE = 3200;
constexpr int16 MATE_BUFFER = 512;

inline bool isMateScore(int s) { return std::abs(s) > MATE - MATE_BUFFER; }

inline int16 toTTScore(int16 s, uint8 pliesFromRoot) {
	if (!isMateScore(s)) return s;
	return (s > 0) ? (s + pliesFromRoot) : (s - pliesFromRoot);
}

inline int16 fromTTScore(int16 s, uint8 pliesFromRoot) {
	if (!isMateScore(s)) return s;
	return (s > 0) ? (s - pliesFromRoot) : (s + pliesFromRoot);
}

typedef struct Entry {
	uint64 zobrist;
	Move bestMove;
	int16 score;
	uint8 depth;
	NodeType nodeType;
} Entry;

typedef struct ttLookUpData {
	LookUpType type;
	int16 value;
	Move move;
} ttLookUpData;

typedef struct TranspositionTable {
	Entry* table;

	TranspositionTable() { 
		table = new Entry[TABLE_SIZE]{}; 
		clearTable();
	}

	inline void clearTable() {
		for (uint32 i = 0; i < TABLE_SIZE; i++) table[i] = {0,NULL_MOVE,SCORE_SENTINAL,0,Exact};
	}

	inline uint32 index(uint64 zobrist) const {
		return (zobrist ^ (zobrist >> 32)) & (TABLE_SIZE - 1); // Or just zobrist & (TABLE_SIZE - 1) probably little to no difference
	}

	inline NodeType getNodeType(int16 alpha, int16 beta, int16 originalAlpha) const {
		if (alpha >= beta) return LowerBound;
		else if (alpha <= originalAlpha) return UpperBound;
		return Exact;
	}

	inline void storeEntry(const Entry& entry) {
		// NOTE: Might not working correctly depending on entry
		uint32 i = index(entry.zobrist);
		if (table[i].zobrist != 0 && table[i].depth > entry.depth)
			return;
		table[i] = entry;
	}

	inline void storeEntry(uint64 zobrist, Move m, uint8 pliesFromRoot, uint8 pliesRemaining, int16 alpha, int16 beta, int16 originalAlpha) {
		NodeType n = getNodeType(alpha, beta, originalAlpha);
		Entry e{zobrist, m, toTTScore(alpha, pliesFromRoot), pliesRemaining, n};
		uint32 i = index(e.zobrist);
		if (table[i].zobrist != 0 && table[i].depth > e.depth)
			return;
		table[i] = e;
	}

	inline void storeEntry(uint64 zobrist, Move m, uint8 pliesFromRoot, uint8 pliesRemaining, int16 score, NodeType n) {
		Entry e{zobrist, m, toTTScore(score, pliesFromRoot), pliesRemaining, n};
		uint32 i = index(e.zobrist);
		if (table[i].zobrist != 0 && table[i].depth > e.depth)
			return;
		table[i] = e;
	}

	inline ttLookUpData lookUp(uint64 zobrist, int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining, SearchStats& stats) {
		Entry entry = table[index(zobrist)];
		if (zobrist == entry.zobrist && entry.score != SCORE_SENTINAL) { 
			stats.ttHits++;
			if (entry.depth >= pliesRemaining) {
				int16 score = fromTTScore(entry.score, pliesFromRoot);
				stats.ttHitsUseful++;
				
				if (entry.nodeType == Exact) {
					stats.ttHitCutoffs++; 
					return {Score, score, entry.bestMove};
				}
				if (entry.nodeType == LowerBound && score >= beta) {
					stats.ttHitCutoffs++; 
					return {BetaIncrease, score, entry.bestMove};
				}
				if (entry.nodeType == UpperBound && score <= alpha) {
					stats.ttHitCutoffs++; 
					return {AlphaIncrease, score, entry.bestMove};
				}
				if (entry.nodeType == LowerBound) return {AlphaIncrease, std::max(alpha, score), entry.bestMove};
				else if (entry.nodeType == UpperBound) return {BetaIncrease, std::min(beta, score), entry.bestMove};
			}
		}
		return {None, -1, NULL_MOVE};
	};

	inline ttLookUpData lookUp(uint64 zobrist, int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining) {
		Entry entry = table[index(zobrist)];
		if (zobrist == entry.zobrist && entry.score != SCORE_SENTINAL) { 
			if (entry.depth >= pliesRemaining) {
				int16 score = fromTTScore(entry.score, pliesFromRoot);
				if (entry.nodeType == Exact) return {Score, score, entry.bestMove};
				else if (entry.nodeType == LowerBound && score >= beta) return {BetaIncrease, score, entry.bestMove};
				else if (entry.nodeType == UpperBound && score <= alpha) return {AlphaIncrease, score, entry.bestMove};
		
				if (entry.nodeType == LowerBound) return {AlphaIncrease, std::max(alpha, score), entry.bestMove};
				else if (entry.nodeType == UpperBound) return {BetaIncrease, std::min(beta, score), entry.bestMove};
			}
		}
		return { None, -1, NULL_MOVE};
	};

	inline Move getTTMove(uint64 zobrist) {
		Entry entry = table[index(zobrist)];
		if (zobrist == entry.zobrist) return entry.bestMove;
		return NULL_MOVE;
	}
} TranspositionTable;
