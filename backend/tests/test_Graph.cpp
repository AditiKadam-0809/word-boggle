#include <iostream>
#include <cassert>
#include "../include/Graph.h"

void test3x3() {
    std::cout << "[Test] 3x3 Board Graph..." << std::endl;
    Graph g(3);
    assert(g.getSize() == 3);
    assert(g.getTotalCells() == 9);

    // Corner (0,0) -> 3 neighbors
    assert(g.getNeighbors(0, 0).size() == 3);
    // Edge (0,1) -> 5 neighbors
    assert(g.getNeighbors(0, 1).size() == 5);
    // Center (1,1) -> 8 neighbors
    assert(g.getNeighbors(1, 1).size() == 8);
    std::cout << "  Passed!" << std::endl;
}

void test4x4() {
    std::cout << "[Test] 4x4 Classic Board Graph..." << std::endl;
    Graph g(4);
    assert(g.getSize() == 4);
    assert(g.getTotalCells() == 16);

    // 4 corners: 3 neighbors each
    assert(g.getNeighbors(0, 0).size() == 3);
    assert(g.getNeighbors(0, 3).size() == 3);
    assert(g.getNeighbors(3, 0).size() == 3);
    assert(g.getNeighbors(3, 3).size() == 3);

    // Edges: 5 neighbors
    assert(g.getNeighbors(0, 1).size() == 5);
    assert(g.getNeighbors(1, 0).size() == 5);

    // Interior 2x2 cells: 8 neighbors each
    for (int r = 1; r <= 2; ++r) {
        for (int c = 1; c <= 2; ++c) {
            assert(g.getNeighbors(r, c).size() == 8);
        }
    }
    std::cout << "  Passed!" << std::endl;
}

void test5x5() {
    std::cout << "[Test] 5x5 Big Board Graph..." << std::endl;
    Graph g(5);
    assert(g.getSize() == 5);
    assert(g.getTotalCells() == 25);

    // Corner & center
    assert(g.getNeighbors(0, 0).size() == 3);
    assert(g.getNeighbors(2, 2).size() == 8);

    // Index conversion
    assert(g.toIndex(4, 4) == 24);
    auto coords = g.toCoords(24);
    assert(coords.first == 4 && coords.second == 4);
    std::cout << "  Passed!" << std::endl;
}

void testBounds() {
    std::cout << "[Test] Boundaries and Exceptions..." << std::endl;
    Graph g(4);
    assert(!g.isValid(-1, 0));
    assert(!g.isValid(0, 4));
    assert(g.isValid(0, 0));
    assert(g.isValid(3, 3));

    bool caught = false;
    try {
        g.getNeighbors(5, 5);
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);
    std::cout << "  Passed!" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "        RUNNING GRAPH UNIT TESTS        " << std::endl;
    std::cout << "========================================" << std::endl;
    test3x3();
    test4x4();
    test5x5();
    testBounds();
    std::cout << "========================================" << std::endl;
    std::cout << "   ALL GRAPH UNIT TESTS PASSED (100%)   " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
