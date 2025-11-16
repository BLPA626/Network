#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#pragma comment(lib, "ws2_32.lib")
namespace fs = std::filesystem;

#define PORT 9000
#define BUFFER_SIZE 1024

// 특정 확장자의 파일 리스트를 구함
std::vector<std::string> getFileList() {
    std::vector<std::string> fileList;
    for (const auto& entry : fs::directory_iterator(".")) {
        if (!entry.is_regular_file()) continue;
        std::string name = entry.path().filename().string();
        if (name.ends_with(".txt") || name.ends_with(".png")) {
            fileList.push_back(name);
        }
    }
    return fileList;
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    bind(listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    listen(listenSock, 1);

    std::cout << "[서버] 클라이언트 연결 대기 중...\n";

    SOCKET clientSock;
    sockaddr_in clientAddr;
    int addrSize = sizeof(clientAddr);
    clientSock = accept(listenSock, (SOCKADDR*)&clientAddr, &addrSize);
    std::cout << "[서버] 클라이언트 연결됨.\n";

    char buffer[BUFFER_SIZE] = {};

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int recvLen = recv(clientSock, buffer, BUFFER_SIZE, 0);
        if (recvLen <= 0) break;

        std::string command(buffer);
        if (command == "list") {
            std::vector<std::string> files = getFileList();
            std::string result;
            for (const auto& file : files)
                result += file + "\n";
            send(clientSock, result.c_str(), result.size(), 0);
        }
        else if (command.starts_with("get ")) {
            std::string filename = command.substr(4);
            std::ifstream file(filename, std::ios::binary);
            if (!file.is_open()) {
                std::string err = "파일을 열 수 없습니다.\n";
                send(clientSock, err.c_str(), err.size(), 0);
                continue;
            }
            while (!file.eof()) {
                file.read(buffer, BUFFER_SIZE);
                int bytes = file.gcount();
                send(clientSock, buffer, bytes, 0);
            }
            std::cout << "[서버] 파일 전송 완료: " << filename << "\n";
            file.close();
        }
    }

    closesocket(clientSock);
    closesocket(listenSock);
    WSACleanup();
    return 0;
}
