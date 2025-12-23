//g++ client.cpp game.cpp -o client
#include <iostream>
#include <string>
#include <arpa/inet.h> // inet_addr, sockaddr_in
#include <unistd.h>    // close
#include <sys/socket.h>
#include "game.hpp"

void client() {
    std::string ip;
    std::cout << "接続先のIPアドレスを入力してください: ";
    std::getline(std::cin, ip);


    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345); // サーバーのポート番号
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "接続に失敗しました" << std::endl;
        return;
    }

    std::cout << "接続成功！" << std::endl;

    winner=1;
    int score[]={0,0,0};
    while(winner>0){
        // サーバーから手札を受け取る
        char buffer[1024] = {0};
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        std::string received(buffer, bytes);

        // 先頭1文字が先攻情報
        int firstPlayer = received[0] - '0';

        // 残りが手札文字列
        std::string hand = received.substr(1);


        if (bytes > 0) {
            std::cout << "手札が配布されました: " << hand << std::endl;
        }
        // ゲームループ
        runGameLoopC(sock, hand, firstPlayer);
        score[winner]++;
        std::cout<<"サーバー "<<score[1]<<"-"<<score[2]<<" クライアント"<<std::endl;
    }

    

    close(sock);
    std::cout << "接続を終了しました" << std::endl;
}