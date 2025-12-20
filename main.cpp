//g++ main.cpp server.cpp client.cpp game.cpp check.cpp ip.cpp -o main
#include <iostream>
#include <string>
#include "game.hpp"


int main() {
    std::string mode;
    std::cout << "起動モードを入力してください (server / client): ";
    std::getline(std::cin, mode);

    if (mode == "server") {
        server();
    } else if (mode == "client") {
        client();
    } else {
        std::cerr << "不正な入力です。server または client を入力してください。" << std::endl;
        return 1; // エラー終了
    }

    return 0; // 正常終了
}