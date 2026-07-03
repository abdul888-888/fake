#pragma once
#include <iostream>


class FriendNode {
public:
    int friendID;
    FriendNode* next;
    FriendNode(int id) : friendID(id), next(nullptr) {}
};

class FriendRequest {
public:
    int fromUserID;
    FriendRequest* next;
    FriendRequest(int f) : fromUserID(f), next(nullptr) {}
};