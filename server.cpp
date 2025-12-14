//g++ server.cpp game.cpp ip.cpp -o server
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "game.hpp"

void server() {
    // ソケット作成
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // どのIPからでも受け付ける
    address.sin_port = htons(12345);       // ポート番号12345

    // ソケットを住所に結びつけ
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 1); // 接続待ち開始

    printMyIP();
    std::cout << "サーバー待機中..." << std::endl;

    int addrlen = sizeof(address);
    int client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen); // 接続受け入れ
    std::cout << "接続成功！" << std::endl;

    bool isFirstGame=true;
    // ゲームループ呼び出し
    startGame(client_fd,isFirstGame);
    runGameLoopS(client_fd);

    // 終了処理
    close(client_fd);
    close(server_fd);
    std::cout << "接続を終了しました" << std::endl;
}