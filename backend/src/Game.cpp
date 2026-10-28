#include "../include/Game.h"
#include <cctype>
#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>

// Standard Boggle 16 dice (4x4)
static const std::vector<std::string> DICE_4X4 = {
    "AAEEGN", "ABBJOO", "ACHOPS", "AFFKPS",
    "AOOTTW", "CIMOTU", "DEILRX", "DELRVY",
    "DISTTY", "EEGHNW", "EEINSU", "EHRTVW",
    "EIOSST", "ELRTTY", "HIMNUQ", "HLNNRZ"
};

// Standard 3x3 dice subset
static const std::vector<std::string> DICE_3X3 = {
    "AAEEGN", "ABBJOO", "ACHOPS",
    "AOOTTW", "CIMOTU", "DEILRX",
    "DISTTY", "EEINSU", "HLNNRZ"
};

// Big Boggle 25 dice (5x5)
static const std::vector<std::string> DICE_5X5 = {
    "AAAFRS", "AAEEEE", "AAFIRS", "ADENNN", "AEEEEM",
    "AEEGMU", "AEGMNN", "AFIRSY", "BJKQXZ", "CCNSTW",
    "CEIILT", "CEILPT", "CEIPST", "DHHNOT", "DHHLOR",
    "DHLNOR", "DDLNOR", "EIIITT", "EMOTTT", "ENSSSU",
    "FIPRSY", "GORRVW", "HIPRRY", "NOOTUW", "OOOTTU"
};

Game::Game() : boardSize(4), difficulty(Difficulty::MEDIUM), totalScore(0) {
    boardGraph.build(4);
}

Game::Game(const std::string& dictionaryPath) : boardSize(4), difficulty(Difficulty::MEDIUM), totalScore(0) {
    boardGraph.build(4);
    loadDictionary(dictionaryPath);
}

std::string Game::normalizeWord(const std::string& str) {
    if (str.empty()) return "";

    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }

    if (start >= str.size()) return "";

    size_t end = str.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(str[end]))) {
        --end;
    }

    std::string result;
    result.reserve(end - start + 1);
    for (size_t i = start; i <= end; ++i) {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(str[i]))));
    }
    return result;
}

void Game::reset() {
    wordMap.clear();
    avlTree.clear();
    foundWordsList.clear();
    totalScore = 0;
}

bool Game::loadDictionary(const std::string& path) {
    return dictionary.loadFromFile(path);
}

const Trie& Game::getDictionary() const {
    return dictionary;
}

std::vector<char> Game::generateDiceBoard(int size) {
    const std::vector<std::string>* diceSet = &DICE_4X4;
    if (size == 3) {
        diceSet = &DICE_3X3;
    } else if (size == 5) {
        diceSet = &DICE_5X5;
    }

    std::vector<std::string> shuffledDice = *diceSet;

    // Use high-resolution time to avoid MinGW std::random_device deterministic seed bug
    static std::mt19937 g(static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    ));
    std::shuffle(shuffledDice.begin(), shuffledDice.end(), g);

    std::uniform_int_distribution<int> faceDist(0, 5);
    std::vector<char> board;
    board.reserve(size * size);

    for (int i = 0; i < size * size; ++i) {
        int face = faceDist(g);
        board.push_back(shuffledDice[i][face]);
    }

    return board;
}

void Game::solveDFS(
    int currentIndex,
    std::string& currentWord,
    std::vector<int>& currentPath,
    std::vector<bool>& visited,
    const std::vector<char>& boardGrid,
    const Graph& graph,
    const Trie& trie,
    std::unordered_map<std::string, WordInfo>& outWords
) {
    visited[currentIndex] = true;
    currentWord.push_back(boardGrid[currentIndex]);
    currentPath.push_back(currentIndex);

    // Prune immediately if current prefix does not exist in Trie
    if (!trie.findPrefix(currentWord)) {
        visited[currentIndex] = false;
        currentWord.pop_back();
        currentPath.pop_back();
        return;
    }

    // Record valid word (standard Boggle min length >= 3)
    if (currentWord.length() >= 3 && trie.search(currentWord)) {
        std::string lowerWord = normalizeWord(currentWord);
        if (outWords.find(lowerWord) == outWords.end()) {
            int pts = calculatePoints(lowerWord);
            outWords[lowerWord] = WordInfo(lowerWord, pts, false, currentPath);
        }
    }

    // Explore 8-directional neighbors via Person B's Graph
    const auto& neighbors = graph.getNeighbors(currentIndex);
    for (int nextIndex : neighbors) {
        if (!visited[nextIndex]) {
            solveDFS(nextIndex, currentWord, currentPath, visited, boardGrid, graph, trie, outWords);
        }
    }

    // Backtrack
    visited[currentIndex] = false;
    currentWord.pop_back();
    currentPath.pop_back();
}

std::unordered_map<std::string, WordInfo> Game::solveBoard(
    const std::vector<char>& boardGrid,
    int size,
    const Trie& trie,
    const Graph& graph
) {
    std::unordered_map<std::string, WordInfo> solvedWords;
    int total = size * size;
    std::vector<bool> visited(total, false);
    std::string currentWord;
    std::vector<int> currentPath;

    for (int i = 0; i < total; ++i) {
        solveDFS(i, currentWord, currentPath, visited, boardGrid, graph, trie, solvedWords);
    }

    return solvedWords;
}

// Calibrated 3 Difficulty Levels: Easy, Medium, Hard
std::pair<int, int> Game::getDifficultyRange(Difficulty diff, int size) {
    if (size == 3) {
        switch (diff) {
            case Difficulty::EASY:   return {35, 60}; // Rich 3x3 board
            case Difficulty::MEDIUM: return {25, 34}; // Balanced
            case Difficulty::HARD:   return {12, 24}; // Tricky/sparse
        }
    } else if (size == 5) {
        switch (diff) {
            case Difficulty::EASY:   return {200, 350}; // Rich 5x5 board
            case Difficulty::MEDIUM: return {130, 199}; // Balanced
            case Difficulty::HARD:   return {80, 129};  // Tough
        }
    }

    // 4x4 Classic Board (Exact puzzle-boggle.com distribution)
    switch (diff) {
        case Difficulty::EASY:   return {75, 120}; // Avg ~77 words
        case Difficulty::MEDIUM: return {65, 74};  // Avg ~70 words
        case Difficulty::HARD:   return {45, 64};  // Classic challenge (~55-68 words)
    }
    return {60, 90};
}

int Game::getTimerSeconds(Difficulty diff) {
    switch (diff) {
        case Difficulty::EASY:   return 300; // 5 minutes
        case Difficulty::MEDIUM: return 240; // 4 minutes
        case Difficulty::HARD:   return 180; // 3 minutes (classic Boggle timer)
    }
    return 180;
}

std::vector<char> Game::generateBoardForDifficulty(
    int size,
    Difficulty diff,
    const Trie& trie,
    const Graph& graph,
    std::unordered_map<std::string, WordInfo>& outSolvedWords
) {
    auto range = getDifficultyRange(diff, size);
    int minWords = range.first;
    int maxWords = range.second;

    std::vector<char> bestBoard;
    std::unordered_map<std::string, WordInfo> bestWords;
    int bestDiff = 999999;

    const int MAX_ATTEMPTS = 100;
    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        std::vector<char> candidate = generateDiceBoard(size);
        auto solved = solveBoard(candidate, size, trie, graph);
        int count = static_cast<int>(solved.size());

        if (count >= minWords && count <= maxWords) {
            outSolvedWords = std::move(solved);
            return candidate;
        }

        int targetMid = (minWords + maxWords) / 2;
        int distance = std::abs(count - targetMid);
        if (distance < bestDiff) {
            bestDiff = distance;
            bestBoard = candidate;
            bestWords = std::move(solved);
        }
    }

    outSolvedWords = std::move(bestWords);
    return bestBoard;
}

Difficulty Game::parseDifficulty(const std::string& diffStr) {
    std::string s = normalizeWord(diffStr);
    if (s == "easy") return Difficulty::EASY;
    if (s == "hard") return Difficulty::HARD;
    return Difficulty::MEDIUM;
}

std::string Game::difficultyToString(Difficulty diff) {
    switch (diff) {
        case Difficulty::EASY:   return "easy";
        case Difficulty::MEDIUM: return "medium";
        case Difficulty::HARD:   return "hard";
    }
    return "medium";
}

bool Game::startNewGame(int size, Difficulty diff) {
    if (size != 3 && size != 4 && size != 5) {
        size = 4;
    }

    reset();
    boardSize = size;
    difficulty = diff;
    boardGraph.build(boardSize);

    std::unordered_map<std::string, WordInfo> solved;
    boardLetters = generateBoardForDifficulty(boardSize, difficulty, dictionary, boardGraph, solved);

    setValidWords(solved);
    return true;
}

void Game::setValidWords(const std::vector<std::string>& words) {
    reset();

    for (const auto& w : words) {
        std::string clean = normalizeWord(w);
        if (clean.length() < 3) continue;

        if (wordMap.find(clean) == wordMap.end()) {
            int pts = calculatePoints(clean);
            wordMap[clean] = WordInfo(clean, pts, false);
            avlTree.insert(clean);
        }
    }
}

void Game::setValidWords(const std::unordered_map<std::string, WordInfo>& words) {
    reset();

    for (const auto& kv : words) {
        std::string clean = normalizeWord(kv.first);
        if (clean.length() < 3) continue;

        if (wordMap.find(clean) == wordMap.end()) {
            wordMap[clean] = kv.second;
            avlTree.insert(clean);
        }
    }
}

int Game::calculatePoints(const std::string& word) {
    return calculateScore(static_cast<int>(word.length()));
}

int Game::calculateScore(int wordLength) {
    if (wordLength <= 2) return 0;
    if (wordLength <= 4) return 1;
    if (wordLength == 5) return 2;
    if (wordLength == 6) return 3;
    if (wordLength == 7) return 5;
    return 11; // 8 or more letters
}

SubmitResponse Game::submitWord(const std::string& rawWord) {
    std::string clean = normalizeWord(rawWord);

    if (clean.empty() || clean.length() < 3) {
        return {SubmitStatus::WORD_INVALID, clean, 0, totalScore, "Word must be at least 3 letters long", {}};
    }

    auto it = wordMap.find(clean);
    if (it == wordMap.end()) {
        return {SubmitStatus::WORD_INVALID, clean, 0, totalScore, "Not a valid word on this board", {}};
    }

    if (it->second.isFound) {
        return {SubmitStatus::WORD_DUPLICATE, clean, 0, totalScore, "Word already found", it->second.path};
    }

    // First time finding this valid word
    it->second.isFound = true;
    totalScore += it->second.points;
    foundWordsList.push_back(clean);

    return {SubmitStatus::WORD_VALID, clean, it->second.points, totalScore, "Word found!", it->second.path};
}

std::map<int, std::pair<int, int>> Game::getWordCountsByLength() const {
    std::map<int, std::pair<int, int>> counts;
    for (int len = 3; len <= 8; ++len) {
        counts[len] = {0, 0};
    }

    for (const auto& pair : wordMap) {
        int len = static_cast<int>(pair.first.length());
        int bucket = (len >= 8) ? 8 : len;

        if (bucket >= 3) {
            counts[bucket].second++; // Total count
            if (pair.second.isFound) {
                counts[bucket].first++; // Found count
            }
        }
    }

    return counts;
}

std::vector<EndGameWord> Game::getEndGameSortedWordList() const {
    std::vector<std::string> sortedWords = avlTree.inOrderTraversal();
    std::vector<EndGameWord> result;
    result.reserve(sortedWords.size());

    for (const auto& w : sortedWords) {
        auto it = wordMap.find(w);
        if (it != wordMap.end()) {
            result.push_back({w, it->second.points, it->second.isFound});
        }
    }

    return result;
}

int Game::getBoardSize() const {
    return boardSize;
}

Difficulty Game::getDifficulty() const {
    return difficulty;
}

const std::vector<char>& Game::getBoard() const {
    return boardLetters;
}

std::string Game::getBoardString() const {
    return std::string(boardLetters.begin(), boardLetters.end());
}

int Game::getTotalScore() const {
    return totalScore;
}

int Game::getScore() const {
    return totalScore;
}

int Game::getFoundCount() const {
    return static_cast<int>(foundWordsList.size());
}

int Game::getTotalCount() const {
    return static_cast<int>(wordMap.size());
}

const std::vector<std::string>& Game::getFoundWords() const {
    return foundWordsList;
}

bool Game::isWordFound(const std::string& word) const {
    std::string clean = normalizeWord(word);
    auto it = wordMap.find(clean);
    return (it != wordMap.end() && it->second.isFound);
}

const std::unordered_map<std::string, WordInfo>& Game::getValidWords() const {
    return wordMap;
}
