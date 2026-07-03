#pragma once
#include <iostream>
#include <string>
using namespace std;

class Comment {
public:
    int commentID;
    int postID;
    int authorUserID;
    string authorName;
    string text;
    string timeStamp;
    Comment* next;

    Comment(int cID, int pID, int authorID, const string& author,
        const string& t, const string& time)
        : commentID(cID), postID(pID), authorUserID(authorID), authorName(author),
        text(t), timeStamp(time), next(nullptr) {
    }

    void display() const {
        cout << authorName << " (Comment #" << commentID << ")\n";
        cout << text << "\n";
        cout << timeStamp << "\n";
    }
};
