#include "../include/Trie.h"
#include <fstream>
#include <cctype>
#include <algorithm>

TrieNode::TrieNode() : isEndOfWord(false) {
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        children[i] = nullptr;
    }
}

TrieNode::~TrieNode() {
    // Destructor handled by Trie::destroy to prevent deep recursion stack overflow
}

Trie::Trie() : root(new TrieNode()), wordCount(0) {}

Trie::~Trie() {
    clear();
    delete root;
    root = nullptr;
}

void Trie::destroy(TrieNode* node) {
    if (!node) return;
    for (int i = 0; i < TrieNode::ALPHABET_SIZE; ++i) {
        if (node->children[i]) {
            destroy(node->children[i]);
            node->children[i] = nullptr;
        }
    }
    delete node;
}

void Trie::clear() {
    if (root) {
        for (int i = 0; i < TrieNode::ALPHABET_SIZE; ++i) {
            if (root->children[i]) {
                destroy(root->children[i]);
                root->children[i] = nullptr;
            }
        }
        root->isEndOfWord = false;
    }
    wordCount = 0;
}

std::string Trie::normalize(const std::string& word) const {
    std::string result;
    result.reserve(word.size());
    for (char ch : word) {
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
        }
    }
    return result;
}

bool Trie::isValidWord(const std::string& word) const {
    if (word.length() < 3) return false; // Standard Boggle minimum word length is 3
    for (char ch : word) {
        if (ch < 'A' || ch > 'Z') return false;
    }
    return true;
}

void Trie::insert(const std::string& word) {
    std::string clean = normalize(word);
    if (!isValidWord(clean)) return;

    TrieNode* curr = root;
    for (char ch : clean) {
        int idx = ch - 'A';
        if (!curr->children[idx]) {
            curr->children[idx] = new TrieNode();
        }
        curr = curr->children[idx];
    }

    if (!curr->isEndOfWord) {
        curr->isEndOfWord = true;
        wordCount++;
    }
}

bool Trie::findPrefix(const std::string& prefix) const {
    std::string clean = normalize(prefix);
    if (clean.empty()) return true;

    TrieNode* curr = root;
    for (char ch : clean) {
        int idx = ch - 'A';
        if (idx < 0 || idx >= TrieNode::ALPHABET_SIZE || !curr->children[idx]) {
            return false;
        }
        curr = curr->children[idx];
    }
    return true;
}

bool Trie::search(const std::string& word) const {
    std::string clean = normalize(word);
    if (clean.empty()) return false;

    TrieNode* curr = root;
    for (char ch : clean) {
        int idx = ch - 'A';
        if (idx < 0 || idx >= TrieNode::ALPHABET_SIZE || !curr->children[idx]) {
            return false;
        }
        curr = curr->children[idx];
    }
    return curr != nullptr && curr->isEndOfWord;
}

bool Trie::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::string word;
    while (file >> word) {
        insert(word);
    }

    file.close();
    return true;
}

int Trie::getWordCount() const {
    return wordCount;
}

bool Trie::isEmpty() const {
    return wordCount == 0;
}

