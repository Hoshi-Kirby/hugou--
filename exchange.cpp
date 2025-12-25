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

int cardValue(std::string card){
    std::string numStr = card.substr(1);

    if (numStr == "A") return 12;
    else if (numStr == "J") return 9;
    else if (numStr == "2") return 13;
    else if (numStr == "Q") return 10;
    else if (numStr == "K") return 11;
    else if (numStr == "O" || numStr == "o") return 14; // Joker
    else {
        // 数字なら stoi して -2
        bool allDigits = true;
        for (char c : numStr) {
            if (!isdigit(static_cast<unsigned char>(c))) {
                allDigits = false;
                break;
            }
        }
        if (allDigits) {
            return std::stoi(numStr) - 2;
        } else {
            return 0; // 不正
        }
    }

}

std::vector<std::string> strongestCards(const std::vector<std::string>& hand) {
    int maxVal = -1;
    for (const auto& c : hand) {
        maxVal = std::max(maxVal, cardValue(c));
    }

    std::vector<std::string> strongest;
    for (const auto& c : hand) {
        if (cardValue(c) == maxVal) {
            strongest.push_back(c);
        }
    }
    return strongest;
}

void excange(int player, std::vector<std::string>& hand, int socli_fd){
    std::string card="a0";
    if (winner==player){
        while (!excCheck(card,hand)){
            std::cout<<"相手に渡すカードを１枚選択してください:";
            std::getline(std::cin, card);
        }
        //手札の削除
        auto it = std::find(hand.begin(), hand.end(), card);
        if (it != hand.end()) {
            hand.erase(it);
        }

        send(socli_fd, card.c_str(), card.size(), 0);

        std::cout<<"通信待機中..."<<std::endl;

        char buffer[1024] = {0};
        int bytes = recv(socli_fd, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            std::string command(buffer, bytes);
            hand.push_back(command);
        }
    }else{
        auto strongest = strongestCards(hand);

        if (strongest.size() == 1) {
            // 最強カードが1枚しかない → 自動で選択
            card = strongest[0];
            std::cout << "最も強いカード " << card << " を渡します。" << std::endl;
        } else {
            // 最強カードが複数 → 入力させる
            while (!excCheck(card, hand) || std::find(strongest.begin(), strongest.end(), card) == strongest.end()) {
                std::cout << "相手に渡すカードを選んでください(候補 ";
                for (auto& c : strongest) std::cout << c << " ";
                std::cout << "):" ;

                std::getline(std::cin, card);
            }
        }
        //手札の削除
        auto it = std::find(hand.begin(), hand.end(), card);
        if (it != hand.end()) {
            hand.erase(it);
        }

        std::cout<<"通信待機中..."<<std::endl;

        char buffer[1024] = {0};
        int bytes = recv(socli_fd, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            std::string command(buffer, bytes);
            hand.push_back(command);
        }

        
        send(socli_fd, card.c_str(), card.size(), 0);
    }
}