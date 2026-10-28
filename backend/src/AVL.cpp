#include "../include/AVL.h"
#include <algorithm>

AVLNode::AVLNode(const std::string& w)
    : word(w), height(1), left(nullptr), right(nullptr) {}

AVLTree::AVLTree() : root(nullptr), nodeCount(0) {}

AVLTree::~AVLTree() {
    clear();
}

int AVLTree::getHeight(AVLNode* node) const {
    return node ? node->height : 0;
}

int AVLTree::getBalanceFactor(AVLNode* node) const {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

void AVLTree::updateHeight(AVLNode* node) {
    if (node) {
        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
    }
}

// Right rotation (LL Case)
AVLNode* AVLTree::rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    // Perform rotation
    x->right = y;
    y->left = T2;

    // Update heights
    updateHeight(y);
    updateHeight(x);

    return x;
}

// Left rotation (RR Case)
AVLNode* AVLTree::rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    // Perform rotation
    y->left = x;
    x->right = T2;

    // Update heights
    updateHeight(x);
    updateHeight(y);

    return y;
}

AVLNode* AVLTree::insertHelper(AVLNode* node, const std::string& word, bool& inserted) {
    // 1. Standard BST insertion
    if (!node) {
        inserted = true;
        return new AVLNode(word);
    }

    if (word < node->word) {
        node->left = insertHelper(node->left, word, inserted);
    } else if (word > node->word) {
        node->right = insertHelper(node->right, word, inserted);
    } else {
        // Duplicate word, do not insert again
        inserted = false;
        return node;
    }

    // 2. Update height of current ancestor node
    updateHeight(node);

    // 3. Get balance factor to check if this node became unbalanced
    int balance = getBalanceFactor(node);

    // 4. Rebalance if needed:

    // Case 1: Left-Left (LL)
    if (balance > 1 && word < node->left->word) {
        return rotateRight(node);
    }

    // Case 2: Right-Right (RR)
    if (balance < -1 && word > node->right->word) {
        return rotateLeft(node);
    }

    // Case 3: Left-Right (LR)
    if (balance > 1 && word > node->left->word) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }

    // Case 4: Right-Left (RL)
    if (balance < -1 && word < node->right->word) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

void AVLTree::insert(const std::string& word) {
    if (word.empty()) return;
    bool inserted = false;
    root = insertHelper(root, word, inserted);
    if (inserted) {
        ++nodeCount;
    }
}

void AVLTree::inOrderHelper(AVLNode* node, std::vector<std::string>& result) const {
    if (!node) return;
    inOrderHelper(node->left, result);
    result.push_back(node->word);
    inOrderHelper(node->right, result);
}

std::vector<std::string> AVLTree::inOrderTraversal() const {
    std::vector<std::string> result;
    result.reserve(nodeCount);
    inOrderHelper(root, result);
    return result;
}

bool AVLTree::searchHelper(AVLNode* node, const std::string& word) const {
    if (!node) return false;
    if (node->word == word) return true;
    if (word < node->word) return searchHelper(node->left, word);
    return searchHelper(node->right, word);
}

bool AVLTree::contains(const std::string& word) const {
    return searchHelper(root, word);
}

int AVLTree::size() const {
    return nodeCount;
}

bool AVLTree::empty() const {
    return nodeCount == 0;
}

void AVLTree::destroyTree(AVLNode* node) {
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

void AVLTree::clear() {
    destroyTree(root);
    root = nullptr;
    nodeCount = 0;
}
