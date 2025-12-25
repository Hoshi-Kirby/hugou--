#pragma once
#include <string>
#include <vector>

// グローバル変数
extern int winner;
// 実行関数
void server();
void client();
// クライアントとの通信を使ってゲームを進めるループ
void printMyIP();
void startGame(int client_fd,bool isFirstGame);
void runGameLoopS(int client_fd);
void runGameLoopC(int sock, const std::string& hand, int firstPlayer);

bool excCheck(const std::string& card, const std::vector<std::string>& hand);
void excange(int player, std::vector<std::string>& hand, int socli_fd);
bool cardCheck(std::vector<std::string> tableMs,std::vector<int> tableNs,std::vector<std::string> playMs,std::vector<int> playNs,std::vector<std::string> played,std::vector<std::string> hand,
bool bind,bool stairs,bool revolution);
bool biCh(std::vector<std::string> tableMs,std::vector<int> tableNs,std::vector<std::string> playMs,std::vector<int> playNs,std::vector<std::string> played,std::vector<std::string> hand);
bool stCh(std::vector<std::string> tableMs,std::vector<int> tableNs,std::vector<std::string> playMs,std::vector<int> playNs,std::vector<std::string> played,std::vector<std::string> hand,bool revolution);
bool reCh(std::vector<std::string> tableMs,std::vector<int> tableNs,std::vector<std::string> playMs,std::vector<int> playNs,std::vector<std::string> played,std::vector<std::string> hand);