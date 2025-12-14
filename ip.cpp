#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

void printMyIP() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname)); // ホスト名を取得

    hostent* host_entry = gethostbyname(hostname); // ホスト情報を取得
    if (host_entry == nullptr) {
        std::cerr << "IPアドレスを取得できませんでした。" << std::endl;
        return;
    }

    char* ip = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    std::cout << "このサーバーのIPアドレスは: " << ip << std::endl;
}