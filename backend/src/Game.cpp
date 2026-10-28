#include "../include/Game.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <cctype>

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

Game::Game() : boardSize(4), difficulty(Difficulty::MEDIUM), score(0) {
    graph.build(4);
    totalCountsByLength.assign(17, 0);
    foundCountsByLength.assign(17, 0);
}

Game::Game(const std::string& dictionaryPath) : boardSize(4), difficulty(Difficulty::MEDIUM), score(0) {
    graph.build(4);
    totalCountsByLength.assign(17, 0);
    foundCountsByLength.assign(17, 0);
    loadDictionary(dictionaryPath);
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

    // Random engine
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffledDice.begin(), shuffledDice.end(), g);

    std::uniform_int_distribution<int> faceDist(0, 5);
    std::vector<char> boardLetters;
    boardLetters.reserve(size * size);

    for (int i = 0; i < size * size; ++i) {
        int face = faceDist(g);
        boardLetters.push_back(shuffledDice[i][face]);
    }

    return boardLetters;
}

void Game::solveDFS(
    int currentIndex,
    std::string& currentWord,
    std::vector<int>& currentPath,
    std::vector<bool>& visited,
    const std::vector<char>& boardGrid,
    const Graph& boardGraph,
    const Trie& trie,
    std::unordered_map<std::string, WordInfo>& outWords
) {
    visited[currentIndex] = true;
    currentWord.push_back(boardGrid[currentIndex]);
    currentPath.push_back(currentIndex);

    // Prefix Pruning: if current word prefix does not exist in Trie, backtrack immediately
    if (!trie.findPrefix(currentWord)) {
        visited[currentIndex] = false;
        currentWord.pop_back();
        currentPath.pop_back();
        return;
    }

    // Check if the current path forms a valid word of length >= 3
    if (currentWord.length() >= 3 && trie.search(currentWord)) {
        if (outWords.find(currentWord) == outWords.end()) {
            outWords[currentWord] = WordInfo(currentPath, false);
        }
    }

    // Explore 8-directional neighbors via Person B's Graph
    const auto& neighbors = boardGraph.getNeighbors(currentIndex);
    for (int nextIndex : neighbors) {
        if (!visited[nextIndex]) {
            solveDFS(nextIndex, currentWord, currentPath, visited, boardGrid, boardGraph, trie, outWords);
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
    const Graph& boardGraph
) {
    std::unordered_map<std::string, WordInfo> solvedWords;
    int total = size * size;
    std::vector<bool> visited(total, false);
    std::string currentWord;
    std::vector<int> currentPath;

    for (int i = 0; i < total; ++i) {
        solveDFS(i, currentWord, currentPath, visited, boardGrid, boardGraph, trie, solvedWords);
    }

    return solvedWords;
}

std::pair<int, int> Game::getDifficultyRange(Difficulty diff, int size) {
    if (size == 3) {
        switch (diff) {
            case Difficulty::EASY:    return {10, 30};
            case Difficulty::MEDIUM:  return {31, 60};
            case Difficulty::HARD:    return {61, 95};
            case Difficulty::EXTREME: return {96, 200};
        }
    } else if (size == 5) {
        switch (diff) {
            case Difficulty::EASY:    return {40, 90};
            case Difficulty::MEDIUM:  return {91, 170};
            case Difficulty::HARD:    return {171, 280};
            case Difficulty::EXTREME: return {281, 600};
        }
    }

    // Default 4x4
    switch (diff) {
        case Difficulty::EASY:    return {25, 55};
        case Difficulty::MEDIUM:  return {56, 110};
        case Difficulty::HARD:    return {111, 180};
        case Difficulty::EXTREME: return {181, 400};
    }
    return {30, 100};
}

std::vector<char> Game::generateBoardForDifficulty(
    int size,
    Difficulty diff,
    const Trie& trie,
    const Graph& boardGraph,
    std::unordered_map<std::string, WordInfo>& outSolvedWords
) {
    auto [minWords, maxWords] = getDifficultyRange(diff, size);

    std::vector<char> bestBoard;
    std::unordered_map<std::string, WordInfo> bestWords;
    int bestDiff = 999999;

    const int MAX_ATTEMPTS = 60;
    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        std::vector<char> candidate = generateDiceBoard(size);
        auto solved = solveBoard(candidate, size, trie, boardGraph);
        int count = static_cast<int>(solved.size());

        if (count >= minWords && count <= maxWords) {
            outSolvedWords = std::move(solved);
            return candidate;
        }

        // Track closest board to range midpoint in case target range isn't reached
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
    std::string s = diffStr;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    if (s == "easy") return Difficulty::EASY;
    if (s == "hard") return Difficulty::HARD;
    if (s == "extreme") return Difficulty::EXTREME;
    return Difficulty::MEDIUM;
}

std::string Game::difficultyToString(Difficulty diff) {
    switch (diff) {
        case Difficulty::EASY: return "easy";
        case Difficulty::MEDIUM: return "medium";
        case Difficulty::HARD: return "hard";
        case Difficulty::EXTREME: return "extreme";
    }
    return "medium";
}

bool Game::startNewGame(int size, Difficulty diff) {
    if (size != 3 && size != 4 && size != 5) {
        size = 4;
    }

    boardSize = size;
    difficulty = diff;
    graph.build(boardSize);
    score = 0;
    submittedWords.clear();
    foundWordsOrder.clear();

    totalCountsByLength.assign(boardSize * boardSize + 1, 0);
    foundCountsByLength.assign(boardSize * boardSize + 1, 0);

    board = generateBoardForDifficulty(boardSize, difficulty, dictionary, graph, validWords);

    // Populate total counts by length
    for (const auto& kv : validWords) {
        size_t len = kv.first.length();
        if (len < totalCountsByLength.size()) {
            totalCountsByLength[len]++;
        }
    }

    return true;
}

int Game::getBoardSize() const {
    return boardSize;
}

Difficulty Game::getDifficulty() const {
    return difficulty;
}

const std::vector<char>& Game::getBoard() const {
    return board;
}

std::string Game::getBoardString() const {
    return std::string(board.begin(), board.end());
}

const std::unordered_map<std::string, WordInfo>& Game::getValidWords() const {
    return validWords;
}

int Game::getTotalValidWordCount() const {
    return static_cast<int>(validWords.size());
}

const std::vector<int>& Game::getTotalCountsByLength() const {
    return totalCountsByLength;
}

const std::vector<int>& Game::getFoundCountsByLength() const {
    return foundCountsByLength;
}

int Game::getScore() const {
    return score;
}

int Game::calculateScore(int wordLength) {
    if (wordLength <= 2) return 0;
    if (wordLength <= 4) return 1;
    if (wordLength == 5) return 2;
    if (wordLength == 6) return 3;
    if (wordLength == 7) return 5;
    return 11; // 8 or more letters
}

Game::SubmitResult Game::submitWord(const std::string& rawWord) {
    std::string clean = rawWord;
    std::transform(clean.begin(), clean.end(), clean.begin(), [](unsigned char c) { return std::toupper(c); });

    SubmitResult res;
    res.isValid = false;
    res.isDuplicate = false;
    res.wordScore = 0;

    if (submittedWords.find(clean) != submittedWords.end()) {
        res.isDuplicate = true;
        auto it = validWords.find(clean);
        if (it != validWords.end()) {
            res.isValid = true;
            res.path = it->second.path;
        }
        return res;
    }

    auto it = validWords.find(clean);
    if (it != validWords.end()) {
        res.isValid = true;
        res.path = it->second.path;
        res.wordScore = calculateScore(static_cast<int>(clean.length()));

        submittedWords.insert(clean);
        foundWordsOrder.push_back(clean);
        it->second.found = true;
        score += res.wordScore;

        size_t len = clean.length();
        if (len < foundCountsByLength.size()) {
            foundCountsByLength[len]++;
        }
    }

    return res;
}

std::vector<std::pair<std::string, bool>> Game::getEndGameWords() const {
    AVLTree avl;
    for (const auto& kv : validWords) {
        avl.insert(kv.first);
    }

    std::vector<std::string> sortedWords = avl.inOrderTraversal();
    std::vector<std::pair<std::string, bool>> result;
    result.reserve(sortedWords.size());

    for (const auto& w : sortedWords) {
        auto it = validWords.find(w);
        bool isFound = (it != validWords.end()) ? it->second.found : false;
        result.emplace_back(w, isFound);
    }

    return result;
}
