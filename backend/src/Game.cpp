#include "../include/Game.h"
#include <cctype>
#include <algorithm>

Game::Game() : totalScore(0) {}

std::string Game::normalizeWord(const std::string& str) {
    if (str.empty()) return "";

    // Find start and end to trim whitespace
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

void Game::setValidWords(const std::vector<std::string>& words) {
    reset();

    for (const auto& w : words) {
        std::string clean = normalizeWord(w);
        if (clean.length() < 3) continue;

        // Prevent duplicate entries
        if (wordMap.find(clean) == wordMap.end()) {
            int pts = calculatePoints(clean);
            wordMap[clean] = WordInfo(clean, pts, false);
            avlTree.insert(clean);
        }
    }
}

int Game::calculatePoints(const std::string& word) {
    size_t len = word.length();
    if (len < 3) return 0;
    if (len == 3 || len == 4) return 1;
    if (len == 5) return 2;
    if (len == 6) return 3;
    if (len == 7) return 5;
    return 11; // 8 or more letters
}

SubmitResponse Game::submitWord(const std::string& rawWord) {
    std::string clean = normalizeWord(rawWord);

    if (clean.empty() || clean.length() < 3) {
        return {SubmitStatus::INVALID, clean, 0, totalScore, "Word must be at least 3 letters long"};
    }

    auto it = wordMap.find(clean);
    if (it == wordMap.end()) {
        return {SubmitStatus::INVALID, clean, 0, totalScore, "Not a valid word on this board"};
    }

    if (it->second.isFound) {
        return {SubmitStatus::DUPLICATE, clean, 0, totalScore, "Word already found"};
    }

    // First time finding this valid word
    it->second.isFound = true;
    totalScore += it->second.points;
    foundWordsList.push_back(clean);

    return {SubmitStatus::VALID, clean, it->second.points, totalScore, "Word found!"};
}

std::map<int, std::pair<int, int>> Game::getWordCountsByLength() const {
    // Categories: 3, 4, 5, 6, 7, 8 (where 8 represents 8+)
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
    // 1. Retrieve all words in alphabetical order from AVL Tree in-order traversal
    std::vector<std::string> sortedWords = avlTree.inOrderTraversal();

    std::vector<EndGameWord> result;
    result.reserve(sortedWords.size());

    // 2. Fetch point values and found status from wordMap
    for (const auto& w : sortedWords) {
        auto it = wordMap.find(w);
        if (it != wordMap.end()) {
            result.push_back({w, it->second.points, it->second.isFound});
        }
    }

    return result;
}

int Game::getTotalScore() const {
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

