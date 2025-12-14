#pragma once
#include <string>

// 実行関数
void server();
void client();
// クライアントとの通信を使ってゲームを進めるループ
void printMyIP();
void startGame(int client_fd,bool isFirstGame);
void runGameLoopS(int client_fd);
void runGameLoopC(int sock, const std::string& hand, int firstPlayer);