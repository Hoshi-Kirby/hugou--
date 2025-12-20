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
            std::cout<<"手札にないカードです"<<std::endl;
        }
    }
    //カードの数字を変換
    for (size_t i = 0; i < playMs.size(); ++i) {
        if (playNs[i]==0){
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

    for (size_t i = 0; i < playMs.size(); ++i) {
        if (i!=0 && (playNs[i-1]!=playNs[i] && playMs[i-1]!="J" && playMs[i]!="J")){
            ch2=1; // 同時出し不成立
        }
    }

    if (ch1==1 and ch2==1){
        return false; // 役無し
        std::cout<<"役になっていません"<<std::endl;
    }
    if (tableNs[0]!=0){//場にカードがあった場合
        if (playMs.size()!=tableMs.size()){
            return false; // 場のカードの枚数と一致していない
            std::cout<<"出すカードの枚数が一致していません"<<std::endl;
        }
        if (stairs){
            if (ch1==1){
                return false; // 階段継続中なのに階段を出していない
                std::cout<<"階段しか出せません"<<std::endl;
            }
        }else{
            if (ch2==1){
                return false; // 階段中ではないのに階段を出している
                std::cout<<"今は階段は出せません"<<std::endl;
            }
        }
        if (!pureNums.empty() && !tableNs.empty() && tableNs[0] != 0) {
            if (pureNums.front() <= tableNs.front()) {
                // 特殊ルール: ジョーカー vs スペード3
                if (!(tableNs.size()==1 && tableNs[0]==14 && pureNums.front()==3 && playMs[0]=="s")) {
                    return false;// 場のカードより弱い数字を出している
                    std::cout<<"場より強いカードしか出せません"<<std::endl;
                }
            }
        }
        if (bind){
            for (const auto& m : playMs) {
                if (m != tableMs[0] && m != "J") {
                    return false; // 縛り違反
                    std::cout<<"場と同じマークしか出せません"<<std::endl;
                }
            }
        }

    }
    return true;
}