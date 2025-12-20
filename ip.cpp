#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <iostream>

void printMyIP() {
    struct ifaddrs *ifaddr;
    getifaddrs(&ifaddr);

    for (auto ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family == AF_INET &&
            std::string(ifa->ifa_name) == "eth0") {

            char buf[INET_ADDRSTRLEN];
            auto* sa = (struct sockaddr_in*)ifa->ifa_addr;
            inet_ntop(AF_INET, &sa->sin_addr, buf, sizeof(buf));
            std::cout << "このサーバーのIPアドレスは: " << buf << std::endl;
        }
    }

    freeifaddrs(ifaddr);
}