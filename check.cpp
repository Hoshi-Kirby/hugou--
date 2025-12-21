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

bool checkStairsWithJoker(const std::vector<int>& nums, int jokers) {
    if (nums.empty()) return jokers > 0; // 全部ジョーカーならOK

    for (size_t i = 1; i < nums.size(); ++i) {
        int gap = nums[i] - nums[i-1];
        if (gap == 1) continue; // 連続
        if (gap > 1) {
            jokers -= (gap - 1); // Jokerで埋める
            if (jokers < 0) return false; // Joker不足
        }
    }
    return true; // Jokerで埋め切れた
}

bool cardCheck(std::vector<std::string> tableMs,std::vector<int> tableNs,std::vector<std::string> playMs,std::vector<int> playNs,std::vector<std::string> played,std::vector<std::string> hand,
    bool bind,bool stairs,bool revolution){
    
    for (const auto& card : played) {
        auto it = std::find(hand.begin(), hand.end(), card);
        if (it == hand.end()) {
            return false; // 手札に存在しないカードを出そうとしている
        }
    }
    //カードの数字を変換
    bool spe3=false;
    for (size_t i = 0; i < playMs.size(); ++i) {
        if (playNs[0]==3 && playMs[0]=="s"){spe3=true;}
        if (playNs[i]<=0){
        }else if (playNs[i]<3){
            playNs[i]+=11;
        }else if(playNs[i]<14){
            playNs[i]-=2;
        }else{
        }
        if (revolution){
            if (0<playNs[i] && playNs[i]<14){
                playNs[i]=14-playNs[i];
            }
        }
    }
    for (size_t i = 0; i < tableMs.size(); ++i) {
        if (tableNs[i]<=0){
        }else if (tableNs[i]<3){
            tableNs[i]+=11;
        }else if(tableNs[i]<14){
            tableNs[i]-=2;
        }else{
        }
        if (revolution){
            if (0<tableNs[i] && tableNs[i]<14){
                tableNs[i]=14-tableNs[i];
            }
        }
    }
    //数字の強さ順にソート
    std::sort(playNs.begin(), playNs.end());
    std::sort(tableNs.begin(), tableNs.end());

    int ch1=0,ch2=0;
    
    // 階段判定
        // Joker枚数を数える
    int jokers = std::count(playMs.begin(), playMs.end(), "J");
        // Joker以外の数字だけ抽出してソート
    std::vector<int> pureNums;
    for (size_t i = 0; i < playNs.size(); ++i) {
        if (playMs[i] != "J") pureNums.push_back(playNs[i]);
    }
    std::sort(pureNums.begin(), pureNums.end());
    if (!checkStairsWithJoker(pureNums, jokers)){
        ch1=1; // 階段不成立
    }
    if (playMs.size()<3){
        ch1=1; // 階段不成立
    }
    for (size_t i = 0; i < playMs.size(); ++i) {
        if (i!=0 && (playMs[i-1]!=playMs[i] && playMs[i-1]!="J" && playMs[i]!="J")){
            ch1=1; // 階段不成立
        }
    }

    for (size_t i = 0; i < playMs.size(); ++i) {
        if (i!=0 && (playNs[i-1]!=playNs[i] && playMs[i-1]!="J" && playMs[i]!="J")){
            ch2=1; // 同時出し不成立
        }
    }

    if (ch1==1 and ch2==1){
        std::cout<<"役になっていません"<<std::endl;
        return false; // 役無し
    }
    if (tableNs[0]!=0){//場にカードがあった場合
        if (playMs.size()!=tableMs.size()){
            std::cout<<"出すカードの枚数が一致していません"<<std::endl;
            return false; // 場のカードの枚数と一致していない
        }
        if (stairs){
            if (ch1==1){
                std::cout<<"階段しか出せません"<<std::endl;
                return false; // 階段継続中なのに階段を出していない
            }
        }else{
            if (ch2==1){
                std::cout<<"今は階段は出せません"<<std::endl;
                return false; // 階段中ではないのに階段を出している
            }
        }
        if (!playNs.empty() && !tableNs.empty() && tableNs[0] != 0) {
            if (playNs.front() <= tableNs.front()) {
                // 特殊ルール: ジョーカー vs スペード3
                if (!(tableNs.size()==1 && tableNs[0]==14 && spe3)) {
                    std::cout<<"場より強いカードしか出せません"<<std::endl;
                    return false;// 場のカードより弱い数字を出している
                }
            }
        }
        if (bind){
            for (const auto& m : playMs) {
                if (m != tableMs[0] && m != "J") {
                    std::cout<<"場と同じマークしか出せません"<<std::endl;
                    return false; // 縛り違反
                }
            }
        }

    }
    return true;
}

bool biCh(std::vector<std::string> tableMs,std::vector<int> tableNs,std::vector<std::string> playMs,std::vector<int> playNs,std::vector<std::string> played,std::vector<std::string> hand){
    for (size_t i = 0; i < playMs.size(); ++i) {
        if (playMs[i]!=tableMs[i] || playMs[i]=="J"){
            return false; // 縛り不成立
        }
    }
    return true;
}
bool stCh(std::vector<std::string> tableMs,std::vector<int> tableNs,std::vector<std::string> playMs,std::vector<int> playNs,std::vector<std::string> played,std::vector<std::string> hand,bool revolution){
    //カードの数字を変換
    for (size_t i = 0; i < playMs.size(); ++i) {
        if (playNs[i]<=0){
        }else if (playNs[i]<3){
            playNs[i]+=11;
        }else if(playNs[i]<14){
            playNs[i]-=2;
        }else{
        }
        if (revolution){
            if (0<playNs[i] && playNs[i]<14){
                playNs[i]=14-playNs[i];
            }
        }
    }
    //数字の強さ順にソート
    std::sort(playNs.begin(), playNs.end());

    int ch=0;
    
    // 階段判定
        // Joker枚数を数える
    int jokers = std::count(playMs.begin(), playMs.end(), "J");
        // Joker以外の数字だけ抽出してソート
    std::vector<int> pureNums;
    for (size_t i = 0; i < playNs.size(); ++i) {
        if (playMs[i] != "J") pureNums.push_back(playNs[i]);
    }
    std::sort(pureNums.begin(), pureNums.end());
    if (!checkStairsWithJoker(pureNums, jokers)){
        return false; // 階段不成立
    }
    if (playMs.size()<3){
        return false; // 階段不成立
    }
    for (size_t i = 0; i < playMs.size(); ++i) {
        if (i!=0 && (playMs[i-1]!=playMs[i] && playMs[i-1]!="J" && playMs[i]!="J")){
            return false; // 階段不成立
        }
    }
    return true;
}
bool reCh(std::vector<std::string> tableMs,std::vector<int> tableNs,std::vector<std::string> playMs,std::vector<int> playNs,std::vector<std::string> played,std::vector<std::string> hand){
    if (playMs.size()<4){
        return false; // 革命不成立
    }
    for (size_t i = 0; i < playMs.size(); ++i) {
        if (i!=0 && (playNs[i-1]!=playNs[i] && playMs[i-1]!="J" && playMs[i]!="J")){
            return false; // 革命不成立
        }
    }
    return true;
}