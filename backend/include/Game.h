#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include "AVL.h"

// Metadata for a single valid word on the board
struct WordInfo {
    std::string word;
    int points;
    bool isFound;

    WordInfo() : word(""), points(0), isFound(false) {}
    WordInfo(const std::string& w, int p, bool found = false)
        : word(w), points(p), isFound(found) {}
};

// Result of word submission check
enum class SubmitStatus {
    WORD_VALID,          // Valid word found for the first time
    WORD_DUPLICATE,      // Already discovered by player
    WORD_INVALID         // Not in the precomputed board words or invalid
};

// Return payload for submit word check
struct SubmitResponse {
    SubmitStatus status;
    std::string word;
    int pointsAwarded;
    int totalScore;
    std::string message;
};

// End-game word entry for sorted results display
struct EndGameWord {
    std::string word;
    int points;
    bool isFound;
};

// Game engine managing board words, score, counts, and player submissions
class Game {
private:
    std::unordered_map<std::string, WordInfo> wordMap; // Fast O(1) lookup & state
    AVLTree avlTree;                                   // Self-balancing BST for sorted in-order traversal
    std::vector<std::string> foundWordsList;           // Words in the order player found them
    int totalScore;

    // Helper to normalize input (lowercase, trim)
    static std::string normalizeWord(const std::string& str);

public:
    Game();

    // Reset game session
    void reset();

    // Populate game with precomputed valid words from board solver
    void setValidWords(const std::vector<std::string>& words);

    // Scoring calculation based on standard Boggle rules:
    // length < 3: 0, 3: 1, 4: 1, 5: 2, 6: 3, 7: 5, 8+: 11
    static int calculatePoints(const std::string& word);

    // Submit a word to check if valid or duplicate
    SubmitResponse submitWord(const std::string& rawWord);

    // Word counts breakdown by length: length -> { foundCount, totalCount }
    // Handles categories 3, 4, 5, 6, 7, 8+
    std::map<int, std::pair<int, int>> getWordCountsByLength() const;

    // End game: returns all valid words in alphabetical sorted order (via AVL in-order traversal)
    // with isFound flags
    std::vector<EndGameWord> getEndGameSortedWordList() const;

    // Getters for player stats
    int getTotalScore() const;
    int getFoundCount() const;
    int getTotalCount() const;
    const std::vector<std::string>& getFoundWords() const;
    bool isWordFound(const std::string& word) const;
};

#endif // GAME_H
