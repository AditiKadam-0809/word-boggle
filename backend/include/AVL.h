#ifndef AVL_H
#define AVL_H

#include <string>
#include <vector>

// Node representation for AVL Tree storing words
struct AVLNode {
    std::string word;
    int height;
    AVLNode* left;
    AVLNode* right;

    explicit AVLNode(const std::string& w);
};

// Self-balancing Binary Search Tree (AVL Tree)
// Used to store words and produce an in-order lexicographically sorted list
class AVLTree {
private:
    AVLNode* root;
    int nodeCount;

    // Helper functions
    int getHeight(AVLNode* node) const;
    int getBalanceFactor(AVLNode* node) const;
    void updateHeight(AVLNode* node);

    // Tree Rotations
    AVLNode* rotateRight(AVLNode* y); // LL rotation
    AVLNode* rotateLeft(AVLNode* x);  // RR rotation

    // Recursive helpers
    AVLNode* insertHelper(AVLNode* node, const std::string& word, bool& inserted);
    void inOrderHelper(AVLNode* node, std::vector<std::string>& result) const;
    bool searchHelper(AVLNode* node, const std::string& word) const;
    void destroyTree(AVLNode* node);

public:
    AVLTree();
    ~AVLTree();

    // Prevent unintended shallow copies
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    // Core requirements
    void insert(const std::string& word);
    std::vector<std::string> inOrderTraversal() const;

    // Lookup & size
    bool contains(const std::string& word) const;
    int size() const;
    bool empty() const;
    void clear();
};

#endif // AVL_H

