#pragma once
#include <string>
using namespace std;

class Likes {
public:
    int userID;
    string userName;
    Likes* next;

    Likes(int id, const string& name) : userID(id), userName(name), next(nullptr) {}
};
