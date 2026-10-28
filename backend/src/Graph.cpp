#include "../include/Graph.h"
#include <stdexcept>

Graph::Graph() : size(0), totalCells(0) {}

Graph::Graph(int boardSize) {
    build(boardSize);
}

void Graph::build(int boardSize) {
    if (boardSize <= 0) {
        throw std::invalid_argument("Board size must be greater than 0");
    }

    size = boardSize;
    totalCells = size * size;
    adjList.assign(totalCells, std::vector<int>());

    // 8 relative directions: horizontal, vertical, and diagonals
    const int dr[8] = {-1, -1, -1,  0, 0,  1,  1,  1};
    const int dc[8] = {-1,  0,  1, -1, 1, -1,  0,  1};

    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            int currentIdx = toIndex(r, c);

            for (int d = 0; d < 8; ++d) {
                int nr = r + dr[d];
                int nc = c + dc[d];

                if (isValid(nr, nc)) {
                    adjList[currentIdx].push_back(toIndex(nr, nc));
                }
            }
        }
    }
}

int Graph::getSize() const {
    return size;
}

int Graph::getTotalCells() const {
    return totalCells;
}

bool Graph::isValid(int row, int col) const {
    return (row >= 0 && row < size && col >= 0 && col < size);
}

bool Graph::isValidIndex(int index) const {
    return (index >= 0 && index < totalCells);
}

int Graph::toIndex(int row, int col) const {
    return row * size + col;
}

std::pair<int, int> Graph::toCoords(int index) const {
    if (!isValidIndex(index)) {
        throw std::out_of_range("Index is out of graph bounds");
    }
    return {index / size, index % size};
}

const std::vector<int>& Graph::getNeighbors(int index) const {
    if (!isValidIndex(index)) {
        throw std::out_of_range("Cell index is out of bounds");
    }
    return adjList[index];
}

const std::vector<int>& Graph::getNeighbors(int row, int col) const {
    if (!isValid(row, col)) {
        throw std::out_of_range("Coordinates (row, col) are out of bounds");
    }
    return adjList[toIndex(row, col)];
}

std::vector<std::pair<int, int>> Graph::getNeighborCoords(int row, int col) const {
    const auto& neighborIndices = getNeighbors(row, col);
    std::vector<std::pair<int, int>> coords;
    coords.reserve(neighborIndices.size());

    for (int idx : neighborIndices) {
        coords.push_back(toCoords(idx));
    }
    return coords;
}
