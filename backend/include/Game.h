#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <utility>
#include "AVL.h"
#include "Graph.h"
#include "Trie.h"

// Metadata for a single valid word on the board
struct WordInfo {
    std::string word;
    int points;
    bool isFound;
    std::vector<int> path; // 1D cell indices of the word on the board

    WordInfo() : word(""), points(0), isFound(false) {}
    WordInfo(const std::string& w, int p, bool found = false, const std::vector<int>& pth = {})
        : word(w), points(p), isFound(found), path(pth) {}
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
    std::vector<int> path;
};

// End-game word entry for sorted results display
struct EndGameWord {
    std::string word;
    int points;
    bool isFound;
};

// Supported difficulty levels: Easy, Medium, Hard
enum class Difficulty {
    EASY,
    MEDIUM,
    HARD
};

// Game engine managing board generation, solver, score, counts, and player submissions
class Game {
private:
    int boardSize;
    Difficulty difficulty;
    std::vector<char> boardLetters;
    Graph boardGraph;
    Trie dictionary;

    std::unordered_map<std::string, WordInfo> wordMap; // Fast O(1) lookup & state
    AVLTree avlTree;                                   // Self-balancing BST for sorted in-order traversal
    std::vector<std::string> foundWordsList;           // Words in the order player found them
    int totalScore;

    // Helper to normalize input (lowercase, trim)
    static std::string normalizeWord(const std::string& str);

    // Internal DFS helper method for Trie-pruned board solving (Person A)
    static void solveDFS(
        int currentIndex,
        std::string& currentWord,
        std::vector<int>& currentPath,
        std::vector<bool>& visited,
        const std::vector<char>& boardGrid,
        const Graph& graph,
        const Trie& trie,
        std::unordered_map<std::string, WordInfo>& outWords
    );

public:
    Game();
    explicit Game(const std::string& dictionaryPath);

    // Reset game session
    void reset();

    // --- Person A: Dice Board Generator & Trie-Pruned DFS Solver ---

    // Generate random board letters using standard Boggle dice sets (size: 3, 4, or 5)
    static std::vector<char> generateDiceBoard(int size);

    // Solve the board using DFS with Trie-based prefix pruning
    static std::unordered_map<std::string, WordInfo> solveBoard(
        const std::vector<char>& boardGrid,
        int size,
        const Trie& trie,
        const Graph& graph
    );

    // Generate board matching difficulty by regenerating until word count matches target range
    static std::vector<char> generateBoardForDifficulty(
        int size,
        Difficulty diff,
        const Trie& trie,
        const Graph& graph,
        std::unordered_map<std::string, WordInfo>& outSolvedWords
    );

    // Get target word count range [minWords, maxWords] for a given difficulty and board size
    static std::pair<int, int> getDifficultyRange(Difficulty diff, int size);

    // Helper conversions for difficulty
    static Difficulty parseDifficulty(const std::string& diffStr);
    static std::string difficultyToString(Difficulty diff);

    // Timer duration in seconds based on difficulty:
    // Easy: 5 min (300s), Medium: 4 min (240s), Hard: 3 min (180s - classic)
    static int getTimerSeconds(Difficulty diff);

    // Initialize/start a new game with specified size (3, 4, 5) and difficulty
    bool startNewGame(int size, Difficulty diff);

    // Populate game with precomputed valid words
    void setValidWords(const std::vector<std::string>& words);
    void setValidWords(const std::unordered_map<std::string, WordInfo>& words);

    // Scoring calculation based on standard Boggle rules:
    // length < 3: 0, 3: 1, 4: 1, 5: 2, 6: 3, 7: 5, 8+: 11
    static int calculatePoints(const std::string& word);
    static int calculateScore(int wordLength);

    // Submit a word to check if valid or duplicate
    SubmitResponse submitWord(const std::string& rawWord);

    // Word counts breakdown by length: length -> { foundCount, totalCount }
    std::map<int, std::pair<int, int>> getWordCountsByLength() const;

    // End game: returns all valid words in alphabetical sorted order (via AVL in-order traversal)
    std::vector<EndGameWord> getEndGameSortedWordList() const;

    // Dictionary management
    bool loadDictionary(const std::string& path);
    const Trie& getDictionary() const;

    // Getters for game state
    int getBoardSize() const;
    Difficulty getDifficulty() const;
    const std::vector<char>& getBoard() const;
    std::string getBoardString() const;
    int getTotalScore() const;
    int getScore() const;
    int getFoundCount() const;
    int getTotalCount() const;
    const std::vector<std::string>& getFoundWords() const;
    bool isWordFound(const std::string& word) const;
    const std::unordered_map<std::string, WordInfo>& getValidWords() const;
};

#endif // GAME_H
