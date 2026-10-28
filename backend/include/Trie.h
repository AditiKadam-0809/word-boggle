#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>

// Node representation for Prefix Tree (Trie)
// Storing uppercase English alphabet characters ('A' through 'Z')
struct TrieNode {
    static constexpr int ALPHABET_SIZE = 26;
    TrieNode* children[ALPHABET_SIZE];
    bool isEndOfWord;

    TrieNode();
    ~TrieNode();
};

// Custom Trie data structure used to store dictionary words
// and provide fast O(L) prefix checking for Boggle DFS pruning.
class Trie {
private:
    TrieNode* root;
    int wordCount;

    // Helper functions
    void destroy(TrieNode* node);
    std::string normalize(const std::string& word) const;
    bool isValidWord(const std::string& word) const;

public:
    Trie();
    ~Trie();

    // Prevent unintended shallow copies
    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    // Core requirements
    // Inserts a word into the Trie (normalizes to uppercase)
    void insert(const std::string& word);

    // Checks if any valid dictionary word starts with the given prefix (critical for DFS pruning)
    bool findPrefix(const std::string& prefix) const;

    // Checks if the exact word exists in the dictionary
    bool search(const std::string& word) const;

    // Loads words line-by-line from a dictionary file (e.g. data/words.txt)
    // Returns true on success, false on failure (e.g. file cannot be opened)
    bool loadFromFile(const std::string& filePath);

    // Returns total count of words inserted
    int getWordCount() const;

    // Clears all nodes from the Trie and resets count
    void clear();

    // Checks if the Trie is empty
    bool isEmpty() const;
};

#endif // TRIE_H

