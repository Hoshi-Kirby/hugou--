#include "game.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>

#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <random>

std::vector<std::string> serverHand;
std::vector<std::string> clientHand;
int firstPlayer=0;
std::string command;

std::vector<std::string> createDeck() {
    std::vector<std::string> deck;
    const char suits[] = {'h','d','c','s'}; // ハート, ダイヤ, クラブ, スペード
    for (char suit : suits) {
        for (int rank = 1; rank <= 13; ++rank) {
            std::string rankStr;
            switch (rank) {
                case 1:  rankStr = "A"; break;
                case 11: rankStr = "J"; break;
                case 12: rankStr = "Q"; break;
                case 13: rankStr = "K"; break;
                default: rankStr = std::to_string(rank); break;
            }
            deck.push_back(std::string(1, suit) + rankStr);
        }
    }    
    deck.push_back("JO");
    deck.push_back("Jo");
    return deck;
}

//開始時
void startGame(int client_fd,bool isFirstGame) {
    // 山札を作成
    std::vector<std::string> deck = createDeck();

    // シャッフル
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(deck.begin(), deck.end(), rng);

    // サーバー用の手札13枚
    serverHand.assign(deck.begin(), deck.begin() + 13);

    // クライアント用の手札13枚
    clientHand.assign(deck.begin()+13, deck.begin() + 26);

    if (isFirstGame) {
        // ダイヤの3を持っているかチェック
        auto hasCard = [](const std::vector<std::string>& hand, const std::string& card) {
            return std::find(hand.begin(), hand.end(), card) != hand.end();
        };

        if (hasCard(serverHand, "d3")) {
            firstPlayer=0; // サーバー先攻
        }else if (hasCard(clientHand, "d3")) {
            firstPlayer=1; // クライアント先攻
        }else{

            // 誰も持っていなければランダム
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(0, 1);
            firstPlayer=dist(gen);
        }

        isFirstGame=false;
    }



    // クライアント用手札を文字列にまとめる
    std::string handStr;
    for (size_t i = 0; i < clientHand.size(); ++i) {
        handStr += clientHand[i];
        if (i < clientHand.size() - 1) handStr += ",";
    }

    // 先攻情報を文字列化して先頭に置く
    std::string msg = std::to_string(firstPlayer) + handStr;
    send(client_fd, msg.c_str(), msg.size(), 0);

    std::cout << "手札を配布しました" << std::endl;

    // サーバー側は serverHand を保持してゲーム進行に使う
    std::cout << "サーバーの手札: ";
    for (auto &card : serverHand) std::cout << card << ",";
    std::cout << std::endl;

}

//受信したカードを変数に
void parseCard(const std::string& command, std::string& tableM, int& tableN) {
    // 先頭文字がマーク
    tableM = command.substr(0, 1);

    // それ以降が数字や文字
    std::string numStr = command.substr(1);

    if (numStr == "A") {
        tableN = 1;
    } else if (numStr == "J") {
        tableN = 11;
    } else if (numStr == "Q") {
        tableN = 12;
    } else if (numStr == "K") {
        tableN = 13;
    } else if (numStr == "O") {
        tableN = 14;
    } else if (numStr == "o") {
        tableN = 14;
    } else {
        bool allDigits = true;
        for (char c : numStr) {
            if (!isdigit(static_cast<unsigned char>(c))) {
                allDigits = false;
                break;
            }
        }
        if (allDigits) {
            tableN = std::stoi(numStr);
        } else {
            tableN = 0; // 数字じゃなければ0にする
        }
    }
}
void parseCards(const std::string& commandLine,std::vector<std::string>& tableMs,std::vector<int>& tableNs) {
    tableMs.clear(); // 受け取る前に空にしておく
    tableNs.clear();

    std::istringstream iss(commandLine);
    std::string token;

    while (iss >> token) {
        std::string m;
        int n;
        parseCard(token, m, n);
        tableMs.push_back(m);
        tableNs.push_back(n);
    }
}

//ゲームループ
void runGameLoopS(int client_fd) {
    bool running = true;
    int lenHands1=13;
    int lenHands2=13;
    std::string command;
    std::vector<std::string> tableMs;
    std::vector<int> tableNs;    
    std::vector<std::string> playMs;
    std::vector<int> playNs;
    std::vector<std::string> played;
    while (running) {
        if (firstPlayer==1){
            std::cout<<"相手のターン中..."<<std::endl;
            char buffer[1024] = {0};
            int bytes = recv(client_fd, buffer, sizeof(buffer), 0);
            if (bytes <= 0) {
                std::cout << "接続が切れました。" << std::endl;
                break;
            }
            std::string command(buffer);
            std::cout << "受信: " << command << std::endl;
            
            // 仮のゲーム処理: "quit" が来たら終了
            if (command == "quit") {
                std::cout<<"クライアントがゲームを終了しました"<<std::endl;
                running = false;
                break;
            }
            parseCards(command, tableMs, tableNs);
            lenHands2-=tableMs.size();

            for (size_t i = 0; i < tableMs.size(); ++i) {
                std::cout << "M=" << tableMs[i] << ", N=" << tableNs[i] << std::endl;
            }

        }else{
            firstPlayer=1;
        }
        



        std::cout << "手札: ";
        for (auto &card : serverHand) std::cout << card << ",";
        std::cout << std::endl;
        std::cout << "自分"<< lenHands1<<"枚 相手"<<lenHands2<<"枚" <<std::endl;
        std::cout << "コマンドを入力してください (quitで終了): ";
        std::getline(std::cin, command);

        parseCards(command, playMs, playNs);
        // 2つの配列を結合して1つの文字列にする
        played.clear();
        for (size_t i = 0; i < playMs.size(); ++i) {
            played.push_back(playMs[i] + std::to_string(playNs[i]));
        }
        
        // クライアントに送信
        send(client_fd, command.c_str(), command.size(), 0);
        if (command == "quit") {
            std::cout<<"ゲーム終了"<<std::endl;
            running = false;
        }

        //手札の削除
        for (auto& card : played) {
            auto it = std::find(serverHand.begin(), serverHand.end(), card);
            if (it != serverHand.end()) {
                serverHand.erase(it);
            }
        }
        lenHands1-=played.size();
    }
}

void runGameLoopC(int sock,const std::string& hand, int firstPlayer) {
    std::istringstream iss(hand);
    std::string token;
    while (getline(iss, token, ',')) {  // 区切り文字をコンマに指定
        clientHand.push_back(token);
    }
    bool running = true;
    int lenHands1=13;
    int lenHands2=13;
    std::string command;
    std::vector<std::string> tableMs;
    std::vector<int> tableNs;
    std::vector<std::string> playMs;
    std::vector<int> playNs;
    std::vector<std::string> played;
    while (running) {
        if (firstPlayer==1){
            std::cout << "手札: ";
            for (auto &card : clientHand) std::cout << card << ",";
            std::cout << std::endl;
            std::cout << "自分"<< lenHands1<<"枚 相手"<<lenHands2<<"枚" <<std::endl;
            std::cout << "コマンドを入力してください (quitで終了): ";
            std::getline(std::cin, command);

            parseCards(command, playMs, playNs);
            // 2つの配列を結合して1つの文字列にする
            played.clear();
            for (size_t i = 0; i < playMs.size(); ++i) {
                played.push_back(playMs[i] + std::to_string(playNs[i]));
            }

            // サーバーに送信
            send(sock, command.c_str(), command.size(), 0);
            if (command == "quit") {
                std::cout<<"ゲームを終了しました"<<std::endl;
                running = false;
                break;
            }

            //手札の削除
            for (auto& card : played) {
                auto it = std::find(clientHand.begin(), clientHand.end(), card);
                if (it != clientHand.end()) {
                    clientHand.erase(it);
                }
            }
            lenHands1-=played.size();
        }else{
            firstPlayer=1;
        }

        // サーバーから返事を受信
        std::cout<<"相手のターン中..."<<std::endl;
        char buffer[1024] = {0};
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            std::cout << "サーバーとの接続が切れました。" << std::endl;
            break;
        }

        std::string command(buffer);
        std::cout << "受信: " << command << std::endl;

        if (command == "quit") {
            std::cout<<"サーバーがゲームを終了しました"<<std::endl;
            running = false;
        }
        parseCards(command, tableMs, tableNs);
        lenHands2-=tableMs.size();

        for (size_t i = 0; i < tableMs.size(); ++i) {
            std::cout << "M=" << tableMs[i] << ", N=" << tableNs[i] << std::endl;
        }

    }
}
