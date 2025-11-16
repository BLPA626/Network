#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 9000
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[클라이언트] 연결 실패.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "[클라이언트] 서버에 연결됨.\n";

    while (true) {
        std::string command;
        std::cout << "\n명령 입력 (list 또는 get [파일명], 종료는 quit): ";
        std::getline(std::cin, command);

        if (command == "quit") break;

        send(sock, command.c_str(), command.length(), 0);

        if (command == "list") {
            char buffer[BUFFER_SIZE] = {};
            int recvLen = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            if (recvLen > 0) {
                buffer[recvLen] = '\0';
                std::cout << "[클라이언트] 수신된 파일 목록:\n" << buffer;
            }
        }
        else if (command.starts_with("get ")) {
            std::string filename = command.substr(4);
            std::ofstream file("recv_" + filename, std::ios::binary);
            char buffer[BUFFER_SIZE];
            int bytes;
            std::cout << "[클라이언트] 파일 수신 중...\n";

            // 파일 수신 루프
            while ((bytes = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
                file.write(buffer, bytes);
                if (bytes < BUFFER_SIZE) break; // 마지막 조각일 가능성
            }
            file.close();
            std::cout << "[클라이언트] 파일 저장 완료: recv_" << filename << "\n";
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
