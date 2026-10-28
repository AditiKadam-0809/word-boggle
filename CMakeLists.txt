cmake_minimum_required(VERSION 3.10)
project(WordBoggle VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(backend/include)

# Backend Word Boggle Server Executable
add_executable(word_boggle_server
    backend/src/main.cpp
    backend/src/Graph.cpp
    backend/src/AVL.cpp
    backend/src/Trie.cpp
    backend/src/Game.cpp
)

# On Windows, link the Winsock library
if (WIN32)
    target_link_libraries(word_boggle_server ws2_32)
endif()

# Enable Testing
enable_testing()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/backend/tests/test_Graph.cpp")
    add_executable(test_graph backend/src/Graph.cpp backend/tests/test_Graph.cpp)
    add_test(NAME TestGraph COMMAND test_graph)
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/backend/tests/test_Trie.cpp")
    add_executable(test_trie backend/src/Trie.cpp backend/tests/test_Trie.cpp)
    add_test(NAME TestTrie COMMAND test_trie)
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/backend/tests/test_Solver.cpp")
    add_executable(test_solver
        backend/src/Graph.cpp
        backend/src/AVL.cpp
        backend/src/Trie.cpp
        backend/src/Game.cpp
        backend/tests/test_Solver.cpp
    )
    add_test(NAME TestSolver COMMAND test_solver)
endif()

