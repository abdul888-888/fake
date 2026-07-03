#pragma once
#include <iostream>
#include <string>
#include "Friends.h"
#include "Post.h"
using namespace std;

class User {
public:
    int userID;
    string name;
    string email;
    string password;
    string gender;
    int age;
    bool isPublic;

    User* next;
    FriendNode* friends;
    FriendRequest* requests;
    Post* posts;

    User(int id, const string& n, const string& e, const string& p,
        const string& g = "", int a = 0, bool isPub = true)
        : userID(id), name(n), email(e), password(p), gender(g), age(a), isPublic(isPub),
        next(nullptr), friends(nullptr), requests(nullptr), posts(nullptr) {
    }

    bool hasFriend(int id) const {
        FriendNode* temp = friends;
        while (temp) {
            if (temp->friendID == id) return true;
            temp = temp->next;
        }
        return false;
    }

    bool addFriend(int id) {
        if (id <= 0 || id == userID || hasFriend(id)) return false;
        FriendNode* f = new FriendNode(id);
        f->next = friends;
        friends = f;
        return true;
    }

    bool removeFriend(int id) {
        if (!friends) return false;

        if (friends->friendID == id) {
            FriendNode* toDelete = friends;
            friends = friends->next;
            delete toDelete;
            return true;
        }

        FriendNode* current = friends;
        while (current->next && current->next->friendID != id) {
            current = current->next;
        }

        if (current->next) {
            FriendNode* toDelete = current->next;
            current->next = toDelete->next;
            delete toDelete;
            return true;
        }
        return false;
    }

    int countFriends() const {
        int count = 0;
        FriendNode* temp = friends;
        while (temp) {
            count++;
            temp = temp->next;
        }
        return count;
    }

    bool hasFriendRequest(int fromID) const {
        FriendRequest* temp = requests;
        while (temp) {
            if (temp->fromUserID == fromID) return true;
            temp = temp->next;
        }
        return false;
    }

    bool addFriendRequest(int fromID) {
        if (fromID <= 0 || fromID == userID || hasFriendRequest(fromID)) return false;
        FriendRequest* r = new FriendRequest(fromID);
        r->next = requests;
        requests = r;
        return true;
    }

    bool removeFriendRequest(int fromID) {
        if (!requests) return false;

        if (requests->fromUserID == fromID) {
            FriendRequest* toDelete = requests;
            requests = requests->next;
            delete toDelete;
            return true;
        }

        FriendRequest* current = requests;
        while (current->next && current->next->fromUserID != fromID) {
            current = current->next;
        }

        if (current->next) {
            FriendRequest* toDelete = current->next;
            current->next = toDelete->next;
            delete toDelete;
            return true;
        }
        return false;
    }

    void addPost(Post* p) {
        if (!p) return;
        p->next = nullptr;
        if (!posts) {
            posts = p;
            return;
        }
        Post* temp = posts;
        while (temp->next) temp = temp->next;
        temp->next = p;
    }

    Post* findPost(int postID) const {
        Post* temp = posts;
        while (temp) {
            if (temp->getPostID() == postID) return temp;
            temp = temp->next;
        }
        return nullptr;
    }

    bool deletePost(int postID) {
        if (!posts) return false;

        if (posts->postID == postID) {
            Post* toDelete = posts;
            posts = posts->next;
            delete toDelete;
            return true;
        }

        Post* current = posts;
        while (current->next && current->next->postID != postID) {
            current = current->next;
        }

        if (current->next) {
            Post* toDelete = current->next;
            current->next = toDelete->next;
            delete toDelete;
            return true;
        }
        return false;
    }

    void displayProfile() const {
        cout << "=== User Profile ===\n"
            << "ID: " << userID << "\n"
            << "Name: " << name << "\n"
            << "Email: " << email << "\n"
            << "Gender: " << gender << "\n"
            << "Age: " << age << "\n"
            << "Profile: " << (isPublic ? "Public" : "Private") << "\n"
            << "Friends: " << countFriends() << "\n";
    }

    ~User() {
        while (friends) {
            FriendNode* temp = friends;
            friends = friends->next;
            delete temp;
        }

        while (requests) {
            FriendRequest* temp = requests;
            requests = requests->next;
            delete temp;
        }

        while (posts) {
            Post* temp = posts;
            posts = posts->next;
            delete temp;
        }
    }
};
