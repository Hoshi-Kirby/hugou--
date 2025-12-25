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
#include <map>

std::vector<std::string> serverHand;
std::vector<std::string> clientHand;
int firstPlayer=0;
std::string command;
int winner=0;

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
    serverHand={};
    serverHand.assign(deck.begin(), deck.begin() + 13);

    // クライアント用の手札13枚
    clientHand={};
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
    if (command.empty()) {
        tableM = "a";
        tableN = 0;
        return;
    }
    tableM = command.substr(0, 1);

    // それ以降が数字や文字
    std::string numStr = command.substr(1);
    if (command.size() > 1) {

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
    }else{
        tableN = 0;
    }
}
void parseCards(const std::string& commandLine,std::vector<std::string>& tableMs,std::vector<int>& tableNs,std::vector<std::string>& played) {
    tableMs.clear(); // 受け取る前に空にしておく
    tableNs.clear();
    played.clear();

    std::istringstream iss(commandLine);
    std::string token;

    while (iss >> token) {
        played.push_back(token);
        std::string m;
        int n;
        parseCard(token, m, n);
        tableMs.push_back(m);
        tableNs.push_back(n);
    }
    // ★ 空入力なら「場が空」を表す 0 を入れる
    if (tableNs.empty()) {
        tableMs.push_back("a");  // マークなし
        tableNs.push_back(0);   // 数字なし（場が空）
        played.push_back("a0");
    }
}

//手札を並べ替える
int cardValue(std::string card, bool revolution){
    std::string numStr = card.substr(1);
    int n;
    if (numStr == "A") n=12;
    else if (numStr == "J") n=9;
    else if (numStr == "2") n=13;
    else if (numStr == "Q") n=10;
    else if (numStr == "K") n=11;
    else if (numStr == "O" || numStr == "o") n=14; // Joker
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
            n=std::stoi(numStr) - 2;
        } else {
            n=0; // 不正
        }
    }
    
    if (revolution){
        if (0<n && n<14){
            n=14-n;
        }
    }
    return n;
}

void sortAK(std::vector<std::string>& hand,bool revolution) {
    std::sort(hand.begin(), hand.end(),
        [revolution](const std::string& a, const std::string& b) {
            return cardValue(a,revolution) < cardValue(b,revolution);
        });
}
void sortKA(std::vector<std::string>& hand,bool revolution) {
    std::sort(hand.begin(), hand.end(),
        [&revolution](const std::string& a, const std::string& b) {
            return cardValue(a,revolution) > cardValue(b,revolution);
        });
}
void sortsuit(std::vector<std::string>& hand, const std::string& command, bool revolution,bool AK) {
    if (command.size() < 8) return;

    std::string orderStr = command.substr(4, 4);

    std::map<char,int> suitOrder;
    for (int i = 0; i < orderStr.size(); i++) {
        suitOrder[orderStr[i]] = i;
    }

    std::sort(hand.begin(), hand.end(),
        [revolution, suitOrder, AK](const std::string& a, const std::string& b) {
            char sa = a[0];
            char sb = b[0];

            // ジョーカー処理
            if (sa == 'J' && sb == 'J') return false; // 両方ジョーカーならそのまま
            if (sa == 'J') {
                return AK ? false : true; // AK=trueなら右端、AK=falseなら左端
            }
            if (sb == 'J') {
                return AK ? true : false; // AK=trueなら右端、AK=falseなら左端
            }

            int va = suitOrder.count(sa) ? suitOrder.at(sa) : 5;
            int vb = suitOrder.count(sb) ? suitOrder.at(sb) : 5;

            if (va == vb) {
                if (AK) {
                    return cardValue(a, revolution) < cardValue(b, revolution); // 弱→強
                } else {
                    return cardValue(a, revolution) > cardValue(b, revolution); // 強→弱
                }
            }
            return va < vb;
        });
}
void sortcustom(std::vector<std::string>& hand) {
    std::cout << "並べ替えるカードを入力してください： ";

    std::string inputOrder;
    std::getline(std::cin, inputOrder);
    std::istringstream iss(inputOrder);
    std::string card;
    std::vector<std::string> ordered;

    // 入力されたカードを順番通りに取り出す
    while (iss >> card) {
        // hand に存在するか確認
        auto it = std::find(hand.begin(), hand.end(), card);
        if (it != hand.end()) {
            ordered.push_back(card);
            hand.erase(it); // hand から削除
        }
    }

    // 最後に順番通り追加
    for (auto& c : ordered) {
        hand.push_back(c);
    }
}



bool isValidSortCommand(const std::string& command) {
    if (command.size() != 8) return false; // "sort" + 4文字 = 8文字

    if (command.substr(0,4) != "sort") return false;

    std::string order = command.substr(4,4); // 後半4文字

    // 必要な文字セット
    std::string required = "scdh";

    // 並べ替えチェック
    std::sort(order.begin(), order.end());
    std::sort(required.begin(), required.end());

    return order == required;
}


void sortCard(std::vector<std::string>& playMs,std::vector<int>& playNs,std::vector<std::string>& played) {

    // 3つをまとめる
    struct Card {
        int n;
        std::string m;
        std::string raw;
    };

    std::vector<Card> cards;
    for (size_t i = 0; i < playNs.size(); ++i) {
        cards.push_back({ playNs[i], playMs[i], played[i] });
    }

    // 数字でソート
    std::sort(cards.begin(), cards.end(),
              [](const Card& a, const Card& b) {
                  return a.n < b.n;
              });

    // ソート結果を展開
    for (size_t i = 0; i < cards.size(); ++i) {
        playNs[i] = cards[i].n;
        playMs[i] = cards[i].m;
        played[i] = cards[i].raw;
    }
}

int winnerDe(std::vector<std::string> table, int player,bool revolution) {
    for (int i = 0; i < table.size(); i++) {
        if (table[i] == "Jo" || table[i] == "JO") {
            std::cout << "反則上がり（ジョーカー）" << std::endl;
            if (player==2){
                std::cout<<"サーバーの勝ち"<<std::endl;
            }else{
                std::cout<<"クライアントの勝ち"<<std::endl;
            }
            return  3 - player;
            break;
        }
    }

    for (const auto& card : table) {
        std::string numPart = card.substr(1);
        if (!revolution && numPart == "2") {
            winner = 3 - player;
            std::cout << "反則上がり（2）" << std::endl;
            if (player==2){
                std::cout<<"サーバーの勝ち"<<std::endl;
            }else{
                std::cout<<"クライアントの勝ち"<<std::endl;
            }
            return  3 - player;
        }
        if (revolution && numPart == "3") {
            winner = 3 - player;
            std::cout << "反則上がり（3）" << std::endl;
            if (player==2){
                std::cout<<"サーバーの勝ち"<<std::endl;
            }else{
                std::cout<<"クライアントの勝ち"<<std::endl;
            }
            return  3 - player;
        }
    }

    if (table.size() == 1 && table[0] == "s3") {
        std::cout << "反則上がり（スペ3）" << std::endl;
        if (player==2){
            std::cout<<"サーバーの勝ち"<<std::endl;
        }else{
            std::cout<<"クライアントの勝ち"<<std::endl;
        }
        return  3 - player;
    }

    if (std::all_of(table.begin(), table.end(),
        [](const std::string& card) {
            return card.size() >= 2 && card[1] == '8';
        })) {
        std::cout << "反則上がり（8切り）" << std::endl;
        if (player==2){
            std::cout<<"サーバーの勝ち"<<std::endl;
        }else{
            std::cout<<"クライアントの勝ち"<<std::endl;
        }
        return  3 - player;
    }

    if (player==1){
        std::cout<<"サーバーの勝ち"<<std::endl;
    }else{
        std::cout<<"クライアントの勝ち"<<std::endl;
    }
    return player;
}


//ゲームループ
void runGameLoopS(int client_fd) {
    sortAK(serverHand,false);
    bool AK=true;
    std::cout << "手札: ";
    for (auto &card : serverHand) std::cout << card << ",";
    std::cout << std::endl;
    if (winner<3){
        excange(1, serverHand, client_fd);
    }
    bool running = true;
    int lenHands1=13;
    int lenHands2=13;
    bool bind=false;
    bool stairs=false;
    bool revolution=false;
    winner=0;
    std::string command;
    std::vector<std::string> tableMs={""};
    std::vector<int> tableNs={0};    
    std::vector<std::string> table={""};
    std::vector<std::string> playMs={""};
    std::vector<int> playNs={0};
    std::vector<std::string> played={""};
    while (running) {
        if (firstPlayer==1){
            std::cout << "手札: ";
            for (auto &card : serverHand) std::cout << card << ",";
            std::cout << std::endl;
            std::cout<<"相手のターン中..."<<std::endl;
            char buffer[1024] = {0};
            int bytes = recv(client_fd, buffer, sizeof(buffer), 0);
            if (bytes <= 0) {
                std::cout << "接続が切れました。" << std::endl;
                break;
            }
            std::string command(buffer);
            
            // "quit" が来たら終了
            if (command == "quit") {
                std::cout<<"クライアントがゲームを終了しました"<<std::endl;
                running = false;
                break;
            }
            parseCards(command, tableMs, tableNs,table);
            if (command=="pass"){
                bind=false;
                stairs=false;
                table={""};
            }else{
                lenHands2-=tableMs.size();
            }

            if (biCh(playMs,playNs,tableMs,tableNs,played,clientHand)){
                bind=true;
                std::cout <<"縛り"<<std::endl;
            }
            if (stCh(playMs,playNs,tableMs,tableNs,played,clientHand,revolution)){
                stairs=true;
                std::cout <<"階段"<<std::endl;
            }
            if (reCh(playMs,playNs,tableMs,tableNs,played,clientHand)){
                revolution=!revolution;
                std::cout <<"革命"<<std::endl;
            }
            //勝敗
            if (lenHands2==0){
                std::cout << "手札: ";
                for (auto &card : serverHand) std::cout << card << ",";
                std::cout << std::endl;
                std::cout << "自分"<< lenHands1<<"枚 相手"<<lenHands2<<"枚" <<std::endl;
                std::cout<<"場: ";
                for (size_t i = 0; i < tableMs.size(); ++i) {
                    std::cout <<table[i]<<" ";
                }
                std::cout<< std::endl;
            
                std::cout<<"ゲーム終了"<<std::endl;
                winner=winnerDe(table,2,revolution);
                firstPlayer=2-winner;
                running = false;
                break;
            }


        }else{
            firstPlayer=1;
        }
        


        command="";
        std::cout << "自分"<< lenHands1<<"枚 相手"<<lenHands2<<"枚" <<std::endl;
        std::cout<<"場: ";
        for (size_t i = 0; i < tableMs.size(); ++i) {
            std::cout <<table[i]<<" ";
        }
        std::cout<< std::endl;
        if (!tableNs.empty() && std::all_of(tableNs.begin(), tableNs.end(),[](int x){ return x == 8; })) {
            command="pass";// 8切り発動
            std::cout<<"8切り"<< std::endl;
        }
        if (tableNs[0]==3 && playNs[0]==14){
            command="pass";// スぺ３発動
            std::cout<<"スペ3返し"<< std::endl;
        }

        while (!cardCheck(tableMs,tableNs,playMs,playNs,played,serverHand,bind,stairs,revolution) && (table[0]=="" || command!="pass") && command!="quit"){
            
            std::cout << "出すカードを入力してください:";
            std::getline(std::cin, command);
            //handだった場合
            if (command=="hand"){
                std::cout << "手札: ";
                for (auto &card : serverHand) std::cout << card << ",";
                std::cout << std::endl;
                std::cout << "自分"<< lenHands1<<"枚 相手"<<lenHands2<<"枚" <<std::endl;
                std::cout<<"場: ";
                for (size_t i = 0; i < tableMs.size(); ++i) {
                    std::cout <<table[i]<<" ";
                }
                std::cout<< std::endl;
            }
            //sort
            if (command=="sortAK") {
                sortAK(serverHand,revolution);
                std::cout << "手札: ";
                for (auto &card : serverHand) std::cout << card << ",";
                std::cout << std::endl;
                AK=true;
            }
            if (command=="sortKA") {
                sortKA(serverHand,revolution);
                std::cout << "手札: ";
                for (auto &card : serverHand) std::cout << card << ",";
                std::cout << std::endl;
                AK=false;
            }
            if (isValidSortCommand(command)){
                sortsuit(serverHand,command,revolution,AK);
                std::cout << "手札: ";
                for (auto &card : serverHand) std::cout << card << ",";
                std::cout << std::endl;
            }
            if (command=="sortcustom") {
                sortcustom(serverHand);
                std::cout << "手札: ";
                for (auto &card : serverHand) std::cout << card << ",";
                std::cout << std::endl;
            }

            parseCards(command, playMs, playNs,played);
            sortCard(playMs,playNs,played);
        }

        if (biCh(tableMs,tableNs,playMs,playNs,played,clientHand)){
            bind=true;
            std::cout <<"縛り"<<std::endl;
        }
        if (stCh(tableMs,tableNs,playMs,playNs,played,clientHand,revolution)){
            stairs=true;
            std::cout <<"階段"<<std::endl;
        }
        if (reCh(tableMs,tableNs,playMs,playNs,played,clientHand)){
            revolution=!revolution;
            std::cout <<"革命"<<std::endl;
        }
        if (!playNs.empty() && std::all_of(playNs.begin(), playNs.end(),[](int x){ return x == 8; })) {
            std::cout<<"8切り"<< std::endl;
        }
        if (tableNs[0]==14 && playNs[0]==3){
            std::cout<<"スペ3返し"<< std::endl;
        }
        if (command=="pass"){
            bind=false;
            stairs=false;
            table={""};
        }else{
            lenHands1-=played.size();
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
        //勝敗
        if (lenHands1==0){
            std::cout<<"ゲーム終了"<<std::endl;
            winner=winnerDe(played,1,revolution);
            firstPlayer=2-winner;
            running = false;
        }
    }
}

void runGameLoopC(int sock,const std::string& hand, int firstPlayer) {
    std::istringstream iss(hand);
    std::string token;
    clientHand={};
    while (getline(iss, token, ',')) {  // 区切り文字をコンマに指定
        clientHand.push_back(token);
    }
    sortAK(clientHand,false);
    bool AK=true;
    std::cout << "手札: ";
    for (auto &card : clientHand) std::cout << card << ",";
    std::cout << std::endl;
    if (winner<3){
        excange(2, clientHand, sock);
    }
    bool running = true;
    int lenHands1=13;
    int lenHands2=13;
    bool bind=false;
    bool stairs=false;
    bool revolution=false;
    winner=0;
    std::string command;
    std::vector<std::string> tableMs={""};
    std::vector<int> tableNs={0};    
    std::vector<std::string> table={""};
    std::vector<std::string> playMs={""};
    std::vector<int> playNs={0};
    std::vector<std::string> played={""};
    while (running) {
        if (firstPlayer==1){
            command="";
            std::cout << "自分"<< lenHands1<<"枚 相手"<<lenHands2<<"枚" <<std::endl;
            std::cout<<"場: ";
            for (size_t i = 0; i < tableMs.size(); ++i) {
                std::cout <<table[i]<<" ";
            }
            std::cout<< std::endl;
            if (!tableNs.empty() && std::all_of(tableNs.begin(), tableNs.end(),[](int x){ return x == 8; })) {
                command="pass";// 8切り発動
                std::cout<<"8切り"<< std::endl;
            }
            if (tableNs[0]==3 && playNs[0]==14){
                command="pass";// スぺ３発動
                std::cout<<"スペ3返し"<< std::endl;
            }
            while (!cardCheck(tableMs,tableNs,playMs,playNs,played,clientHand,bind,stairs,revolution) && (table[0]=="" || command!="pass") && command!="quit"){

                std::cout << "出すカードを入力してください:";
                std::getline(std::cin, command);
                //handだった場合
                if (command=="hand"){
                    std::cout << "手札: ";
                    for (auto &card : clientHand) std::cout << card << ",";
                    std::cout << std::endl;
                    std::cout << "自分"<< lenHands1<<"枚 相手"<<lenHands2<<"枚" <<std::endl;
                    std::cout<<"場: ";
                    for (size_t i = 0; i < tableMs.size(); ++i) {
                        std::cout <<table[i]<<" ";
                    }
                    std::cout<< std::endl;
                }
                //sort
                if (command=="sortAK") {
                    sortAK(clientHand,revolution);
                    std::cout << "手札: ";
                    for (auto &card : clientHand) std::cout << card << ",";
                    std::cout << std::endl;
                    AK=true;
                }
                if (command=="sortKA") {
                    sortKA(clientHand,revolution);
                    std::cout << "手札: ";
                    for (auto &card : clientHand) std::cout << card << ",";
                    std::cout << std::endl;
                    AK=false;
                }
                if (isValidSortCommand(command)){
                    sortsuit(clientHand,command,revolution,AK);
                    std::cout << "手札: ";
                    for (auto &card : clientHand) std::cout << card << ",";
                    std::cout << std::endl;
                }
                if (command=="sortcustom") {
                    sortcustom(clientHand);
                    std::cout << "手札: ";
                    for (auto &card : clientHand) std::cout << card << ",";
                    std::cout << std::endl;
                }

                parseCards(command, playMs, playNs,played);
                sortCard(playMs,playNs,played);
            }

            if (biCh(tableMs,tableNs,playMs,playNs,played,clientHand)){
                bind=true;
                std::cout <<"縛り"<<std::endl;
            }
            if (stCh(tableMs,tableNs,playMs,playNs,played,clientHand,revolution)){
                stairs=true;
                std::cout <<"階段"<<std::endl;
            }
            if (reCh(tableMs,tableNs,playMs,playNs,played,clientHand)){
                revolution=!revolution;
                std::cout <<"革命"<<std::endl;
            }
            if (!playNs.empty() && std::all_of(playNs.begin(), playNs.end(),[](int x){ return x == 8; })) {
                std::cout<<"8切り"<< std::endl;
            }
            if (tableNs[0]==14 && playNs[0]==3){
                std::cout<<"スペ3返し"<< std::endl;
            }
            if (command=="pass"){
                bind=false;
                stairs=false;
                table={""};
            }else{
                lenHands1-=played.size();
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
            if (lenHands1==0){
                std::cout<<"ゲーム終了"<<std::endl;
                winner=winnerDe(played,2,revolution);
                firstPlayer=2-winner;
                running = false;
                break;
            }
        }else{
            firstPlayer=1;
        }

        // サーバーから返事を受信
        std::cout << "手札: ";
        for (auto &card : clientHand) std::cout << card << ",";
        std::cout << std::endl;
        std::cout<<"相手のターン中..."<<std::endl;
        char buffer[1024] = {0};
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            std::cout << "サーバーとの接続が切れました。" << std::endl;
            break;
        }

        std::string command(buffer);

        if (command == "quit") {
            std::cout<<"サーバーがゲームを終了しました"<<std::endl;
            running = false;
        }
        parseCards(command, tableMs, tableNs,table);
        if (command=="pass"){
            bind=false;
            stairs=false;
            table={""};
        }else{
            lenHands2-=tableMs.size();
        }

        if (biCh(playMs,playNs,tableMs,tableNs,played,clientHand)){
            bind=true;
            std::cout <<"縛り"<<std::endl;
        }
        if (stCh(playMs,playNs,tableMs,tableNs,played,clientHand,revolution)){
            stairs=true;
            std::cout <<"階段"<<std::endl;
        }
        if (reCh(playMs,playNs,tableMs,tableNs,played,clientHand)){
            revolution=!revolution;
            std::cout <<"革命"<<std::endl;
        }
        //勝敗
        if (lenHands2==0){
            std::cout << "手札: ";
            for (auto &card : clientHand) std::cout << card << ",";
            std::cout << std::endl;
            std::cout << "自分"<< lenHands1<<"枚 相手"<<lenHands2<<"枚" <<std::endl;
            std::cout<<"場: ";
            for (size_t i = 0; i < tableMs.size(); ++i) {
                std::cout <<table[i]<<" ";
            }
            std::cout<< std::endl;

            std::cout<<"ゲーム終了"<<std::endl;
            winner=winnerDe(table,1,revolution);
            firstPlayer=2-winner;
            running = false;
        }
    }
}