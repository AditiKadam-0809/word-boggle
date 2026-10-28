#include <iostream>
#include <cassert>
#include "graph.h"

void test3x3() {
    std::cout << "[Test] Running 3x3 Board Graph Tests..." << std::endl;
    Graph g(3);

    assert(g.getSize() == 3);
    assert(g.getTotalCells() == 9);

    // Corner (0,0) -> 3 neighbors: (0,1), (1,0), (1,1)
    const auto& c00 = g.getNeighbors(0, 0);
    assert(c00.size() == 3);

    // Edge (0,1) -> 5 neighbors: (0,0), (0,2), (1,0), (1,1), (1,2)
    const auto& e01 = g.getNeighbors(0, 1);
    assert(e01.size() == 5);

    // Center (1,1) -> 8 neighbors: all surrounding cells
    const auto& mid = g.getNeighbors(1, 1);
    assert(mid.size() == 8);

    // Check coordinate conversions
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            int idx = g.toIndex(r, c);
            auto coords = g.toCoords(idx);
            assert(coords.first == r && coords.second == c);
        }
    }
    std::cout << "  -> 3x3 Tests Passed!" << std::endl;
}

void test4x4() {
    std::cout << "[Test] Running 4x4 Classic Board Graph Tests..." << std::endl;
    Graph g(4);

    assert(g.getSize() == 4);
    assert(g.getTotalCells() == 16);

    // 4 corners should all have 3 neighbors
    assert(g.getNeighbors(0, 0).size() == 3);
    assert(g.getNeighbors(0, 3).size() == 3);
    assert(g.getNeighbors(3, 0).size() == 3);
    assert(g.getNeighbors(3, 3).size() == 3);

    // Edge cells (not corners) should have 5 neighbors
    assert(g.getNeighbors(0, 1).size() == 5);
    assert(g.getNeighbors(0, 2).size() == 5);
    assert(g.getNeighbors(1, 0).size() == 5);
    assert(g.getNeighbors(2, 3).size() == 5);

    // Interior cells (2x2 center) should all have 8 neighbors
    for (int r = 1; r <= 2; ++r) {
        for (int c = 1; c <= 2; ++c) {
            assert(g.getNeighbors(r, c).size() == 8);
        }
    }
    std::cout << "  -> 4x4 Tests Passed!" << std::endl;
}

void test5x5() {
    std::cout << "[Test] Running 5x5 Big Board Graph Tests..." << std::endl;
    Graph g(5);

    assert(g.getSize() == 5);
    assert(g.getTotalCells() == 25);

    // Corner
    assert(g.getNeighbors(0, 0).size() == 3);
    // Center cell (2, 2)
    assert(g.getNeighbors(2, 2).size() == 8);

    // Coordinate conversion test
    auto coords = g.toCoords(24); // Bottom right corner (4,4)
    assert(coords.first == 4 && coords.second == 4);
    assert(g.toIndex(4, 4) == 24);

    // Neighbor coords helper test
    auto nCoords = g.getNeighborCoords(0, 0);
    assert(nCoords.size() == 3);

    std::cout << "  -> 5x5 Tests Passed!" << std::endl;
}

void testBounds() {
    std::cout << "[Test] Running Boundary and Exception Tests..." << std::endl;
    Graph g(4);

    assert(!g.isValid(-1, 0));
    assert(!g.isValid(0, -1));
    assert(!g.isValid(4, 0));
    assert(!g.isValid(0, 4));
    assert(g.isValid(0, 0));
    assert(g.isValid(3, 3));

    assert(!g.isValidIndex(-1));
    assert(!g.isValidIndex(16));
    assert(g.isValidIndex(0));
    assert(g.isValidIndex(15));

    bool caught = false;
    try {
        g.getNeighbors(10, 10);
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    std::cout << "  -> Boundary Tests Passed!" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   BOGGLE GRAPH MODULE UNIT TESTS       " << std::endl;
    std::cout << "========================================" << std::endl;

    test3x3();
    test4x4();
    test5x5();
    testBounds();

    std::cout << "========================================" << std::endl;
    std::cout << " ALL GRAPH UNIT TESTS PASSED SUCCESSFULLY! " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
