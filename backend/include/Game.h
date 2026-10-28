#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Graph.h"
#include "Trie.h"
#include "AVL.h"

// Information about a valid word on the board
struct WordInfo {
    std::vector<int> path; // 1D cell indices of the word on the board
    bool found;            // Whether the player found this word during gameplay

    WordInfo() : found(false) {}
    WordInfo(const std::vector<int>& p, bool f = false) : path(p), found(f) {}
};

// Supported difficulty levels
enum class Difficulty {
    EASY,
    MEDIUM,
    HARD,
    EXTREME
};

// Game and Solver engine for Boggle
class Game {
private:
    int boardSize;
    Difficulty difficulty;
    std::vector<char> board;
    Graph graph;
    Trie dictionary;

    // Solver data structures
    std::unordered_map<std::string, WordInfo> validWords; // All valid words found by DFS
    std::unordered_set<std::string> submittedWords;       // Duplicate detection during play
    std::vector<std::string> foundWordsOrder;             // Order player submitted words
    std::vector<int> totalCountsByLength;                 // Total word counts indexed by length
    std::vector<int> foundCountsByLength;                 // Player found counts indexed by length
    int score;

    // Internal DFS helper method for Trie-pruned board solving
    static void solveDFS(
        int currentIndex,
        std::string& currentWord,
        std::vector<int>& currentPath,
        std::vector<bool>& visited,
        const std::vector<char>& boardGrid,
        const Graph& boardGraph,
        const Trie& trie,
        std::unordered_map<std::string, WordInfo>& outWords
    );

public:
    Game();
    explicit Game(const std::string& dictionaryPath);

    // --- Person A: Dice Board Generator & Trie-Pruned DFS Solver ---

    // Generate random board letters using standard Boggle dice sets (size: 3, 4, or 5)
    static std::vector<char> generateDiceBoard(int size);

    // Solve the board using DFS with Trie-based prefix pruning
    static std::unordered_map<std::string, WordInfo> solveBoard(
        const std::vector<char>& boardGrid,
        int size,
        const Trie& trie,
        const Graph& boardGraph
    );

    // Generate board matching difficulty by regenerating until word count matches target range
    static std::vector<char> generateBoardForDifficulty(
        int size,
        Difficulty diff,
        const Trie& trie,
        const Graph& boardGraph,
        std::unordered_map<std::string, WordInfo>& outSolvedWords
    );

    // Get target word count range [minWords, maxWords] for a given difficulty and board size
    static std::pair<int, int> getDifficultyRange(Difficulty diff, int size);

    // Helper to convert difficulty string ("easy", "medium", "hard", "extreme") to enum
    static Difficulty parseDifficulty(const std::string& diffStr);
    static std::string difficultyToString(Difficulty diff);

    // --- Game Lifecycle & State Management ---

    // Initialize/start a new game with specified size (3, 4, 5) and difficulty
    bool startNewGame(int size, Difficulty diff);

    // Getters for state and API contract
    int getBoardSize() const;
    Difficulty getDifficulty() const;
    const std::vector<char>& getBoard() const;
    std::string getBoardString() const;
    const std::unordered_map<std::string, WordInfo>& getValidWords() const;
    int getTotalValidWordCount() const;
    const std::vector<int>& getTotalCountsByLength() const;
    const std::vector<int>& getFoundCountsByLength() const;
    int getScore() const;

    // Load or reload dictionary into Trie
    bool loadDictionary(const std::string& path);
    const Trie& getDictionary() const;

    // Submit word check (used by Person B's POST /api/submit)
    // Returns: {isValid, isDuplicate, wordScore, path}
    struct SubmitResult {
        bool isValid;
        bool isDuplicate;
        int wordScore;
        std::vector<int> path;
    };
    SubmitResult submitWord(const std::string& rawWord);

    // Scoring formula by word length
    static int calculateScore(int wordLength);

    // End game: Populate AVL Tree and return words in lexicographically sorted order
    std::vector<std::pair<std::string, bool>> getEndGameWords() const;
};

#endif // GAME_H
