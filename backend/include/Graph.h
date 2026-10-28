#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <utility>
#include <stdexcept>

// Graph representation for an N x N Boggle board.
// Each cell on the grid is connected to up to 8 adjacent neighbors
// (horizontal, vertical, and diagonal).
class Graph {
private:
    int size;                                 // Grid dimension (e.g., 3, 4, or 5)
    int totalCells;                           // size * size
    std::vector<std::vector<int>> adjList;    // Adjacency list: neighbor cell indices for each cell

public:
    // Default constructor
    Graph();

    // Constructor initializing graph of given N x N size
    explicit Graph(int boardSize);

    // Build or rebuild graph for a board of size N x N
    void build(int boardSize);

    // Get board dimension (N for N x N)
    int getSize() const;

    // Get total number of cells (N * N)
    int getTotalCells() const;

    // Check if (row, col) is within grid bounds
    bool isValid(int row, int col) const;

    // Check if 1D cell index is valid
    bool isValidIndex(int index) const;

    // Convert (row, col) to 1D index: row * size + col
    int toIndex(int row, int col) const;

    // Convert 1D index to (row, col)
    std::pair<int, int> toCoords(int index) const;

    // Get neighbor indices for a cell index
    const std::vector<int>& getNeighbors(int index) const;

    // Get neighbor indices for cell (row, col)
    const std::vector<int>& getNeighbors(int row, int col) const;

    // Get neighbor coordinates as (row, col) pairs
    std::vector<std::pair<int, int>> getNeighborCoords(int row, int col) const;
};

#endif // GRAPH_H
