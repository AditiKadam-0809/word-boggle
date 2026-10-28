#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <algorithm>
#include <cctype>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "../include/Graph.h"
#include "../include/AVL.h"
#include "../include/Game.h"

// Global game instance
static Game g_game;
static std::vector<std::string> g_currentBoard;
static int g_currentSize = 4;

// Simple JSON extraction helpers
std::string extractJsonString(const std::string& json, const std::string& key) {
    std::string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == std::string::npos) {
        // Try without quotes on key
        pattern = key + ":";
        pos = json.find(pattern);
        if (pos == std::string::npos) return "";
    }

    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";

    pos = json.find('\"', pos);
    if (pos == std::string::npos) return "";

    size_t endPos = json.find('\"', pos + 1);
    if (endPos == std::string::npos) return "";

    return json.substr(pos + 1, endPos - pos - 1);
}

int extractJsonInt(const std::string& json, const std::string& key, int defaultVal = 0) {
    std::string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == std::string::npos) {
        pattern = key + ":";
        pos = json.find(pattern);
        if (pos == std::string::npos) return defaultVal;
    }

    pos = json.find(':', pos);
    if (pos == std::string::npos) return defaultVal;

    while (pos < json.size() && (json[pos] == ':' || json[pos] == ' ' || json[pos] == '\t')) {
        pos++;
    }

    size_t endPos = pos;
    while (endPos < json.size() && (isdigit(json[endPos]) || json[endPos] == '-')) {
        endPos++;
    }

    if (endPos > pos) {
        try {
            return std::stoi(json.substr(pos, endPos - pos));
        } catch (...) {
            return defaultVal;
        }
    }
    return defaultVal;
}

// Standard Boggle dice distribution
std::vector<std::string> generateBoard(int size) {
    static const std::string dice4x4[16] = {
        "RIFOBX", "IFEHEY", "DENOWS", "UTOKND",
        "HMSRAO", "LUPETS", "ACITOA", "YLGKUE",
        "QBMJOA", "EHISPN", "VETIGN", "BALIYT",
        "EZAVND", "RALESC", "UWILRG", "PACEMD"
    };

    static std::mt19937 rng(42);
    std::vector<std::string> board;
    int totalCells = size * size;

    for (int i = 0; i < totalCells; ++i) {
        const std::string& die = dice4x4[i % 16];
        char ch = die[rng() % die.size()];
        board.push_back(std::string(1, ch));
    }

    std::shuffle(board.begin(), board.end(), rng);
    return board;
}

// Build counts JSON: {"3": [found, total], "4": [found, total], ...}
std::string buildCountsJson(const std::map<int, std::pair<int, int>>& counts) {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& pair : counts) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << pair.first << "\":[" << pair.second.first << "," << pair.second.second << "]";
    }
    oss << "}";
    return oss.str();
}

// HTTP Response Helper
std::string createHttpResponse(int statusCode, const std::string& contentType, const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " OK\r\n";
    oss << "Content-Type: " << contentType << "\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Access-Control-Allow-Origin: *\r\n";
    oss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    oss << "Access-Control-Allow-Headers: Content-Type\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << body;
    return oss.str();
}

// Read static file from frontend folder
bool readStaticFile(const std::string& relPath, std::string& content, std::string& mime) {
    std::string safePath = relPath;
    if (safePath.empty() || safePath == "/") safePath = "/index.html";

    if (!safePath.empty() && safePath[0] == '/') {
        safePath = safePath.substr(1);
    }

    if (safePath.find(".html") != std::string::npos) mime = "text/html";
    else if (safePath.find(".css") != std::string::npos) mime = "text/css";
    else if (safePath.find(".js") != std::string::npos) mime = "application/javascript";
    else mime = "text/plain";

    std::string fullPath = "frontend/" + safePath;
    std::ifstream file(fullPath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        fullPath = "../frontend/" + safePath;
        file.open(fullPath.c_str(), std::ios::binary);
    }

    if (!file.is_open()) return false;

    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
    return true;
}

// Handle API and Static requests
std::string handleRequest(const std::string& method, const std::string& path, const std::string& body) {
    if (method == "OPTIONS") {
        return createHttpResponse(200, "text/plain", "");
    }

    // Endpoint 1: GET /api/ping
    if (path == "/api/ping") {
        return createHttpResponse(200, "application/json", "{\"status\":\"ok\",\"server\":\"Word Boggle C++ Backend\"}");
    }

    // Endpoint 2: POST /api/new
    if (path == "/api/new" && method == "POST") {
        int size = extractJsonInt(body, "size", 4);
        if (size < 3 || size > 5) size = 4;
        std::string difficulty = extractJsonString(body, "difficulty");
        if (difficulty.empty()) difficulty = "medium";

        g_currentSize = size;
        g_currentBoard = generateBoard(size);

        std::vector<std::string> validWords = {
            "cat", "cats", "care", "cars", "cart", "cater", "rate", "rats", "rose",
            "star", "stare", "start", "seat", "sate", "east", "eats", "eater", "rest",
            "team", "time", "tine", "site", "side", "line", "lion", "pine", "send",
            "sand", "band", "bland", "blend", "bend", "bone", "cone", "tone", "note",
            "boat", "coat", "goat", "float", "flat", "plat", "plot", "slot", "spot",
            "stop", "post", "host", "most", "cost", "lost", "frost", "forest", "boggling"
        };

        g_game.setValidWords(validWords);

        std::ostringstream boardJson;
        boardJson << "[";
        for (size_t i = 0; i < g_currentBoard.size(); ++i) {
            if (i > 0) boardJson << ",";
            boardJson << "\"" << g_currentBoard[i] << "\"";
        }
        boardJson << "]";

        std::string countsJson = buildCountsJson(g_game.getWordCountsByLength());

        std::ostringstream resp;
        resp << "{"
             << "\"board\":" << boardJson.str() << ","
             << "\"size\":" << g_currentSize << ","
             << "\"difficulty\":\"" << difficulty << "\","
             << "\"counts\":" << countsJson << ","
             << "\"totalWords\":" << g_game.getTotalCount()
             << "}";

        return createHttpResponse(200, "application/json", resp.str());
    }

    // Endpoint 3: POST /api/submit
    if (path == "/api/submit" && method == "POST") {
        std::string word = extractJsonString(body, "word");
        SubmitResponse res = g_game.submitWord(word);

        std::string statusStr;
        switch (res.status) {
            case SubmitStatus::WORD_VALID:     statusStr = "VALID"; break;
            case SubmitStatus::WORD_DUPLICATE: statusStr = "DUPLICATE"; break;
            case SubmitStatus::WORD_INVALID:   statusStr = "INVALID"; break;
        }

        std::string countsJson = buildCountsJson(g_game.getWordCountsByLength());

        std::ostringstream resp;
        resp << "{"
             << "\"status\":\"" << statusStr << "\","
             << "\"word\":\"" << res.word << "\","
             << "\"pointsAwarded\":" << res.pointsAwarded << ","
             << "\"totalScore\":" << res.totalScore << ","
             << "\"foundCount\":" << g_game.getFoundCount() << ","
             << "\"counts\":" << countsJson << ","
             << "\"message\":\"" << res.message << "\""
             << "}";

        return createHttpResponse(200, "application/json", resp.str());
    }

    // Endpoint 4: GET /api/end
    if (path == "/api/end" && method == "GET") {
        std::vector<EndGameWord> words = g_game.getEndGameSortedWordList();

        std::ostringstream wordsJson;
        wordsJson << "[";
        for (size_t i = 0; i < words.size(); ++i) {
            if (i > 0) wordsJson << ",";
            wordsJson << "{"
                      << "\"word\":\"" << words[i].word << "\","
                      << "\"points\":" << words[i].points << ","
                      << "\"found\":" << (words[i].isFound ? "true" : "false")
                      << "}";
        }
        wordsJson << "]";

        std::ostringstream resp;
        resp << "{"
             << "\"totalScore\":" << g_game.getTotalScore() << ","
             << "\"foundCount\":" << g_game.getFoundCount() << ","
             << "\"totalCount\":" << g_game.getTotalCount() << ","
             << "\"words\":" << wordsJson.str()
             << "}";

        return createHttpResponse(200, "application/json", resp.str());
    }

    // Static frontend files
    std::string fileContent, mime;
    if (readStaticFile(path, fileContent, mime)) {
        return createHttpResponse(200, mime, fileContent);
    }

    return createHttpResponse(404, "text/plain", "404 Not Found");
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "     WORD BOGGLE C++ BACKEND SERVER     " << std::endl;
    std::cout << "========================================" << std::endl;

    WSADATA wsaData;
    int wsaRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaRes != 0) {
        std::cerr << "WSAStartup failed with error: " << wsaRes << std::endl;
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        return 1;
    }

    int optval = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[Server] Word Boggle HTTP Server running at http://localhost:8080" << std::endl;
    std::cout << "[Server] Available Endpoints:" << std::endl;
    std::cout << "  - GET  /api/ping" << std::endl;
    std::cout << "  - POST /api/new" << std::endl;
    std::cout << "  - POST /api/submit" << std::endl;
    std::cout << "  - GET  /api/end" << std::endl;
    std::cout << "  - GET  / (Frontend static files)" << std::endl;
    std::cout << "========================================" << std::endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            continue;
        }

        char buffer[4096];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string rawRequest(buffer, bytesReceived);

            std::istringstream reqStream(rawRequest);
            std::string method, path, protocol;
            reqStream >> method >> path >> protocol;

            std::string body = "";
            size_t headerEnd = rawRequest.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                body = rawRequest.substr(headerEnd + 4);
            }

            // Read remaining body if Content-Length specified
            size_t clPos = rawRequest.find("Content-Length:");
            if (clPos == std::string::npos) clPos = rawRequest.find("content-length:");
            if (clPos != std::string::npos) {
                size_t valStart = clPos + 15;
                while (valStart < rawRequest.size() && rawRequest[valStart] == ' ') valStart++;
                int contentLength = std::atoi(rawRequest.c_str() + valStart);

                while (contentLength > 0 && body.size() < static_cast<size_t>(contentLength)) {
                    int more = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                    if (more <= 0) break;
                    body.append(buffer, more);
                }
            }

            std::cout << "[HTTP] " << method << " " << path << std::endl;

            std::string response = handleRequest(method, path, body);
            send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
        }

        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
