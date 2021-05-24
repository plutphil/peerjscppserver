#pragma once
#include <vector>
#include <algorithm>
using namespace std;
template<typename T>
void vectorremove(vector<T> v, size_t i) {
    if (i >= 0 && i < v.size())
        v.erase(v.begin() + i);
}
template<typename T>
T vectorfind(vector<T> v, T i) {
    for (auto e : v) {
        if (e == i) {
            return e;
        }
    }
    return NULL;
}
template<typename T>
size_t vectorgeti(vector<T> v, T c) {
    for (size_t i = 0; i < v.size(); i++)
    {
        if (v[i] == c) {
            return i;
        }
    }
    return -1;
}
#include <sstream>
#include <random>
#include <string>

unsigned int random_char() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}
class MyUUID{
public:
    char data[16];
    MyUUID() {
        for (char i = 0; i < 16; i++){
            data[i] = random_char();
        }
    }
    string toString() {
        std::stringstream ss;
        for (char i = 0; i < 16; i++) {
            const auto rc = data[i];
            std::stringstream hexstream;
            hexstream << std::hex << rc;
            auto hex = hexstream.str();
            ss << (hex.length() < 2 ? '0' + hex : hex);
            if (i == 7 || i == 11 || i == 15 || i == 19) {
                ss << "-";
            }
        }
        return ss.str();
    }
};
template <typename T>
class UUIDBinTreeNode {
public:
    T content;
    UUIDBinTreeNode* left;
    UUIDBinTreeNode* right;
};
template <typename T>
class UUIDBinTree {
public:
    UUIDBinTreeNode<T> rootnode;
    UUIDBinTree() {

    }
};

std::string generate_hex(const unsigned int len) {
    std::stringstream ss;
    for (auto i = 0; i < len; i++) {
        const auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << rc;
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}
//8           4      4      4      12
//0123 4567   8901   2345   6789   0123 4567 890
//86f1 6858 - 1a42 - 4c69 - aa4a - caa2 6629 bf1a
string generateuuid() {
    return 
        generate_hex(4) 
        + "-" + generate_hex(2) 
        + "-" + generate_hex(2) 
        + "-" + generate_hex(2) 
        + "-" + generate_hex(6);
}