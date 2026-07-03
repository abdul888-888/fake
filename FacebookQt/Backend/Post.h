#pragma once
#include <iostream>
#include <string>
#include "Comment.h"
#include "Liskes.h"
using namespace std;

class Post {
public:
    int postID;
    string content;
    string timeStamp;
    Post* next;
    int totalLikes;
    Likes* likeHead;
    int totalComments;
    Comment* commentHead;

    Post(int id, const string& c, const string& time = "")
        : postID(id), content(c), timeStamp(time), next(nullptr),
        totalLikes(0), likeHead(nullptr), totalComments(0), commentHead(nullptr) {
    }

    int getPostID() const { return postID; }
    string getContent() const { return content; }
    int getTotalLikes() const { return totalLikes; }
    int getTotalComments() const { return totalComments; }
    string getTimeStamp() const { return timeStamp; }

    void setTimeStamp(const string& time) { timeStamp = time; }
    void editContent(const string& newContent) { content = newContent; }

    bool hasLike(int userID) const {
        Likes* temp = likeHead;
        while (temp) {
            if (temp->userID == userID) return true;
            temp = temp->next;
        }
        return false;
    }

    bool addLike(int userID, const string& userName) {
        if (hasLike(userID)) return false;

        Likes* newLike = new Likes(userID, userName);
        newLike->next = likeHead;
        likeHead = newLike;
        totalLikes++;
        return true;
    }

    bool removeLike(int userID) {
        if (!likeHead) return false;

        if (likeHead->userID == userID) {
            Likes* toDelete = likeHead;
            likeHead = likeHead->next;
            delete toDelete;
            totalLikes--;
            return true;
        }

        Likes* current = likeHead;
        while (current->next && current->next->userID != userID) {
            current = current->next;
        }

        if (current->next) {
            Likes* toDelete = current->next;
            current->next = toDelete->next;
            delete toDelete;
            totalLikes--;
            return true;
        }
        return false;
    }

    void addLoadedLike(int userID, const string& userName) {
        if (!addLike(userID, userName)) return;
    }

    void addComment(int commentID, int authorID, const string& authorName,
        const string& text, const string& timeStamp) {
        Comment* newComment = new Comment(commentID, postID, authorID, authorName, text, timeStamp);
        newComment->next = commentHead;
        commentHead = newComment;
        totalComments++;
    }

    bool deleteComment(int commentID, int requesterID) {
        if (!commentHead) return false;

        if (commentHead->commentID == commentID) {
            if (commentHead->authorUserID != requesterID) return false;
            Comment* toDelete = commentHead;
            commentHead = commentHead->next;
            delete toDelete;
            totalComments--;
            return true;
        }

        Comment* current = commentHead;
        while (current->next && current->next->commentID != commentID) {
            current = current->next;
        }

        if (current->next) {
            if (current->next->authorUserID != requesterID) return false;
            Comment* toDelete = current->next;
            current->next = toDelete->next;
            delete toDelete;
            totalComments--;
            return true;
        }
        return false;
    }

    bool deleteCommentAsOwner(int commentID) {
        if (!commentHead) return false;

        if (commentHead->commentID == commentID) {
            Comment* toDelete = commentHead;
            commentHead = commentHead->next;
            delete toDelete;
            totalComments--;
            return true;
        }

        Comment* current = commentHead;
        while (current->next && current->next->commentID != commentID) {
            current = current->next;
        }

        if (current->next) {
            Comment* toDelete = current->next;
            current->next = toDelete->next;
            delete toDelete;
            totalComments--;
            return true;
        }
        return false;
    }

    void showComments() const {
        if (!commentHead) {
            cout << "No comments yet.\n";
            return;
        }

        Comment* temp = commentHead;
        while (temp) {
            temp->display();
            if (temp->next) cout << "\n";
            temp = temp->next;
        }
    }

    void showLikes() const {
        if (!likeHead) {
            cout << "No likes yet.\n";
            return;
        }

        cout << "Liked by: ";
        Likes* temp = likeHead;
        while (temp) {
            cout << temp->userName << " (ID: " << temp->userID << ")";
            if (temp->next) cout << ", ";
            temp = temp->next;
        }
        cout << "\n";
    }

    void displayPost() const {
        cout << "Post #" << postID << " [" << timeStamp << "]\n";
        cout << content << "\n";
        cout << "Likes: " << totalLikes << " | Comments: " << totalComments << "\n";
    }

    void displayFullPost() const {
        displayPost();
        showLikes();
        cout << "Comments:\n";
        showComments();
    }

    ~Post() {
        while (likeHead) {
            Likes* temp = likeHead;
            likeHead = likeHead->next;
            delete temp;
        }

        while (commentHead) {
            Comment* temp = commentHead;
            commentHead = commentHead->next;
            delete temp;
        }
    }
};
