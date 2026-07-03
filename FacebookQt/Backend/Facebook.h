#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <cctype>
#include "UserAuthentication.h"
#include "User.h"
#include "Post.h"
#include "Friends.h"
#include "Comment.h"
#include "Liskes.h"
using namespace std;

class Facebook {
private:
    UserAuthentication authSystem;
    User* currentUser;
    int nextPostID;
    int nextCommentID;

    static bool isBlank(const string& value) {
        for (size_t i = 0; i < value.length(); i++) {
            if (!isspace(static_cast<unsigned char>(value[i]))) return false;
        }
        return true;
    }

    static string currentDateTime() {
        time_t now = time(nullptr);
        tm localTime;
#ifdef _WIN32
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif
        stringstream ss;
        ss << put_time(&localTime, "%d %B %Y %I:%M %p");
        return ss.str();
    }

    static string encode(const string& value) {
        string result;
        for (size_t i = 0; i < value.length(); i++) {
            char ch = value[i];
            if (ch == '%') result += "%25";
            else if (ch == '|') result += "%7C";
            else if (ch == '\n') result += "%0A";
            else if (ch == '\r') result += "%0D";
            else result += ch;
        }
        return result;
    }

    static string decode(const string& value) {
        string result;
        for (size_t i = 0; i < value.length(); i++) {
            if (value[i] == '%' && i + 2 < value.length()) {
                string code = value.substr(i, 3);
                if (code == "%25") { result += '%'; i += 2; }
                else if (code == "%7C") { result += '|'; i += 2; }
                else if (code == "%0A") { result += '\n'; i += 2; }
                else if (code == "%0D") { result += '\r'; i += 2; }
                else result += value[i];
            }
            else {
                result += value[i];
            }
        }
        return result;
    }

    static bool nextField(const string& line, size_t& start, string& field) {
        if (start > line.length()) return false;
        size_t end = line.find('|', start);
        if (end == string::npos) {
            field = line.substr(start);
            start = line.length() + 1;
        }
        else {
            field = line.substr(start, end - start);
            start = end + 1;
        }
        return true;
    }

    static int toInt(const string& value, int fallback = 0) {
        try {
            return stoi(value);
        }
        catch (...) {
            return fallback;
        }
    }

    User* findPostOwner(int postID) const {
        User* user = authSystem.GetHead();
        while (user) {
            if (user->findPost(postID)) return user;
            user = user->next;
        }
        return nullptr;
    }

public:
    Facebook() : currentUser(nullptr), nextPostID(1), nextCommentID(1) {
        loadData();
    }

    bool signUp(int id, const string& name, const string& email,
        const string& password, const string& gender, int age, bool isPublic) {
        return authSystem.signUp(id, name, email, password, gender, age, isPublic);
    }

    bool login(int id, const string& password) {
        if (authSystem.login(id, password)) {
            currentUser = authSystem.findUserByID(id);
            return true;
        }
        return false;
    }

    bool login(const string& email, const string& password) {
        if (authSystem.login(email, password)) {
            currentUser = authSystem.findUserByEmail(email);
            return true;
        }
        return false;
    }

    void logout() {
        if (currentUser) cout << "Logged out successfully.\n";
        currentUser = nullptr;
    }

    bool canViewProfile(User* targetUser) const {
        if (!currentUser || !targetUser) return false;
        return targetUser->isPublic || targetUser->userID == currentUser->userID ||
            targetUser->hasFriend(currentUser->userID);
    }

    bool canViewPosts(User* targetUser) const {
        return canViewProfile(targetUser);
    }

    bool canInteractWithPost(User* targetUser) const {
        return canViewProfile(targetUser);
    }

    void sendFriendRequest(int friendID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        if (friendID <= 0) {
            cout << "User ID must be greater than zero.\n";
            return;
        }
        if (friendID == currentUser->userID) {
            cout << "You cannot send a friend request to yourself.\n";
            return;
        }

        User* friendUser = authSystem.findUserByID(friendID);
        if (!friendUser) {
            cout << "User with ID " << friendID << " not found.\n";
            return;
        }
        if (currentUser->hasFriend(friendID)) {
            cout << "You are already friends with this user.\n";
            return;
        }
        if (friendUser->hasFriendRequest(currentUser->userID)) {
            cout << "A friend request is already pending.\n";
            return;
        }
        if (currentUser->hasFriendRequest(friendID)) {
            cout << "This user already sent you a request. Accept or reject it from your requests.\n";
            return;
        }

        friendUser->addFriendRequest(currentUser->userID);
        cout << "Friend request sent to " << friendUser->name << "!\n";
    }

    void acceptFriendRequest(int fromUserID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        if (!currentUser->hasFriendRequest(fromUserID)) {
            cout << "No friend request from user " << fromUserID << ".\n";
            return;
        }

        User* fromUser = authSystem.findUserByID(fromUserID);
        if (!fromUser) {
            currentUser->removeFriendRequest(fromUserID);
            cout << "Request sender no longer exists.\n";
            return;
        }

        currentUser->addFriend(fromUserID);
        fromUser->addFriend(currentUser->userID);
        currentUser->removeFriendRequest(fromUserID);
        cout << "You are now friends with " << fromUser->name << "!\n";
    }

    void rejectFriendRequest(int fromUserID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        if (currentUser->removeFriendRequest(fromUserID)) {
            cout << "Friend request rejected.\n";
        }
        else {
            cout << "No friend request from user " << fromUserID << ".\n";
        }
    }

    void removeFriend(int friendID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        if (!currentUser->hasFriend(friendID)) {
            cout << "You are not friends with this user.\n";
            return;
        }

        currentUser->removeFriend(friendID);
        User* friendUser = authSystem.findUserByID(friendID);
        if (friendUser) friendUser->removeFriend(currentUser->userID);
        cout << "Friend removed successfully.\n";
    }

    void createPost(const string& content) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        if (content.empty() || isBlank(content)) {
            cout << "Post cannot be empty.\n";
            return;
        }

        Post* newPost = new Post(nextPostID++, content, currentDateTime());
        currentUser->addPost(newPost);
        cout << "Post created successfully!\n";
    }

    void editPost(int postID, const string& newContent) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        if (newContent.empty() || isBlank(newContent)) {
            cout << "Post content cannot be empty.\n";
            return;
        }

        Post* post = currentUser->findPost(postID);
        if (!post) {
            cout << "Post not found or you do not own it.\n";
            return;
        }
        post->editContent(newContent);
        cout << "Post updated successfully.\n";
    }

    void deletePost(int postID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        if (currentUser->deletePost(postID)) cout << "Post deleted successfully.\n";
        else cout << "Post not found or you do not own it.\n";
    }

    void likePost(int userID, int postID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        User* targetUser = authSystem.findUserByID(userID);
        if (!targetUser) {
            cout << "User not found.\n";
            return;
        }
        if (!canInteractWithPost(targetUser)) {
            cout << "This account is private.\n";
            return;
        }
        Post* targetPost = targetUser->findPost(postID);
        if (!targetPost) {
            cout << "Post not found.\n";
            return;
        }
        if (!targetPost->addLike(currentUser->userID, currentUser->name)) {
            cout << "You already liked this post.\n";
            return;
        }
        cout << "You liked " << targetUser->name << "'s post!\n";
    }

    void unlikePost(int userID, int postID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        User* targetUser = authSystem.findUserByID(userID);
        if (!targetUser) {
            cout << "User not found.\n";
            return;
        }
        if (!canInteractWithPost(targetUser)) {
            cout << "This account is private.\n";
            return;
        }
        Post* targetPost = targetUser->findPost(postID);
        if (!targetPost) {
            cout << "Post not found.\n";
            return;
        }
        if (targetPost->removeLike(currentUser->userID)) cout << "Like removed.\n";
        else cout << "You have not liked this post.\n";
    }

    void commentOnPost(int userID, int postID, const string& comment) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        if (comment.empty() || isBlank(comment)) {
            cout << "Comment cannot be empty.\n";
            return;
        }
        User* targetUser = authSystem.findUserByID(userID);
        if (!targetUser) {
            cout << "User not found.\n";
            return;
        }
        if (!canInteractWithPost(targetUser)) {
            cout << "This account is private.\n";
            return;
        }
        Post* targetPost = targetUser->findPost(postID);
        if (!targetPost) {
            cout << "Post not found.\n";
            return;
        }
        targetPost->addComment(nextCommentID++, currentUser->userID, currentUser->name, comment, currentDateTime());
        cout << "Comment added successfully!\n";
    }

    void deleteComment(int ownerID, int postID, int commentID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        User* owner = authSystem.findUserByID(ownerID);
        if (!owner) {
            cout << "User not found.\n";
            return;
        }
        Post* post = owner->findPost(postID);
        if (!post) {
            cout << "Post not found.\n";
            return;
        }

        bool deleted = false;
        if (owner->userID == currentUser->userID) deleted = post->deleteCommentAsOwner(commentID);
        else deleted = post->deleteComment(commentID, currentUser->userID);

        if (deleted) cout << "Comment deleted successfully.\n";
        else cout << "Comment not found or you do not have permission to delete it.\n";
    }

    void showMyProfile() {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        currentUser->displayProfile();
    }

    void viewUserProfile(int userID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        User* user = authSystem.findUserByID(userID);
        if (!user) {
            cout << "User not found.\n";
            return;
        }
        if (!canViewProfile(user)) {
            cout << "This account is private.\n";
            return;
        }
        user->displayProfile();
    }

    void showMyFriends() {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        int friendCount = currentUser->countFriends();
        if (friendCount == 0) {
            cout << "You have no friends yet.\n";
            return;
        }

        cout << "Your Friends (" << friendCount << "):\n";
        FriendNode* temp = currentUser->friends;
        while (temp) {
            User* friendUser = authSystem.findUserByID(temp->friendID);
            if (friendUser) cout << "- " << friendUser->name << " (ID: " << temp->friendID << ")\n";
            temp = temp->next;
        }
    }

    void showPostsForUser(int userID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        User* user = authSystem.findUserByID(userID);
        if (!user) {
            cout << "User not found.\n";
            return;
        }
        if (!canViewPosts(user)) {
            cout << "This account is private.\n";
            return;
        }
        if (!user->posts) {
            cout << "No posts to show.\n";
            return;
        }

        cout << user->name << "'s Posts:\n";
        Post* temp = user->posts;
        while (temp) {
            temp->displayFullPost();
            cout << "---\n";
            temp = temp->next;
        }
    }

    void showMyPosts() {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        showPostsForUser(currentUser->userID);
    }

    void showFriendRequests() {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        if (!currentUser->requests) {
            cout << "No pending friend requests.\n";
            return;
        }

        cout << "Pending Friend Requests:\n";
        FriendRequest* temp = currentUser->requests;
        while (temp) {
            User* requester = authSystem.findUserByID(temp->fromUserID);
            if (requester) cout << "- " << requester->name << " (ID: " << temp->fromUserID << ")\n";
            temp = temp->next;
        }
    }

    void showNewsFeed() {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        bool shownAny = false;
        for (int id = nextPostID - 1; id >= 1; id--) {
            User* owner = findPostOwner(id);
            if (owner && currentUser->hasFriend(owner->userID)) {
                Post* post = owner->findPost(id);
                cout << owner->name << " (ID: " << owner->userID << ")\n";
                post->displayFullPost();
                cout << "---\n";
                shownAny = true;
            }
        }
        if (!shownAny) cout << "Your news feed is empty. Add friends or ask them to post.\n";
    }

    void searchByID(int id) {
        User* user = authSystem.findUserByID(id);
        if (user) cout << user->name << " | ID: " << user->userID << " | Email: " << user->email << "\n";
        else cout << "No user found.\n";
    }

    void searchByEmail(const string& email) {
        User* user = authSystem.findUserByEmail(email);
        if (user) cout << user->name << " | ID: " << user->userID << " | Email: " << user->email << "\n";
        else cout << "No user found.\n";
    }

    void searchByName(const string& namePart) {
        if (namePart.empty() || isBlank(namePart)) {
            cout << "Search name cannot be empty.\n";
            return;
        }
        bool found = false;
        User* temp = authSystem.GetHead();
        while (temp) {
            if (temp->name.find(namePart) != string::npos) {
                cout << temp->name << " | ID: " << temp->userID << " | Email: " << temp->email << "\n";
                found = true;
            }
            temp = temp->next;
        }
        if (!found) cout << "No matching users found.\n";
    }

    int countMutualFriends(User* a, User* b) const {
        if (!a || !b) return 0;
        int count = 0;
        FriendNode* temp = a->friends;
        while (temp) {
            if (b->hasFriend(temp->friendID)) count++;
            temp = temp->next;
        }
        return count;
    }

    void showMutualFriends(int otherUserID) {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        User* other = authSystem.findUserByID(otherUserID);
        if (!other) {
            cout << "User not found.\n";
            return;
        }
        bool found = false;
        cout << "Mutual Friends:\n";
        FriendNode* temp = currentUser->friends;
        while (temp) {
            if (other->hasFriend(temp->friendID)) {
                User* mutual = authSystem.findUserByID(temp->friendID);
                if (mutual) {
                    cout << "- " << mutual->name << " (ID: " << mutual->userID << ")\n";
                    found = true;
                }
            }
            temp = temp->next;
        }
        if (!found) cout << "No mutual friends found.\n";
    }

    void showFriendSuggestions() {
        if (!currentUser) {
            cout << "Please login first.\n";
            return;
        }
        bool found = false;
        User* temp = authSystem.GetHead();
        while (temp) {
            if (temp->userID != currentUser->userID && !currentUser->hasFriend(temp->userID)) {
                int mutual = countMutualFriends(currentUser, temp);
                if (mutual > 0) {
                    cout << temp->name << " (ID: " << temp->userID << ") - " << mutual << " mutual friend(s)\n";
                    found = true;
                }
            }
            temp = temp->next;
        }
        if (!found) cout << "No friend suggestions right now.\n";
    }

    void saveData() {
        ofstream users("users.txt");
        User* user = authSystem.GetHead();
        while (user) {
            users << user->userID << "|" << encode(user->name) << "|" << encode(user->email) << "|"
                << encode(user->password) << "|" << encode(user->gender) << "|" << user->age << "|"
                << user->isPublic << "\n";
            user = user->next;
        }
        users.close();

        ofstream friends("friends.txt");
        ofstream requests("requests.txt");
        ofstream posts("posts.txt");
        ofstream likes("likes.txt");
        ofstream comments("comments.txt");

        user = authSystem.GetHead();
        while (user) {
            FriendNode* friendNode = user->friends;
            while (friendNode) {
                friends << user->userID << "|" << friendNode->friendID << "\n";
                friendNode = friendNode->next;
            }

            FriendRequest* request = user->requests;
            while (request) {
                requests << user->userID << "|" << request->fromUserID << "\n";
                request = request->next;
            }

            Post* post = user->posts;
            while (post) {
                posts << user->userID << "|" << post->postID << "|" << encode(post->content) << "|"
                    << encode(post->timeStamp) << "\n";

                Likes* like = post->likeHead;
                while (like) {
                    likes << user->userID << "|" << post->postID << "|" << like->userID << "|"
                        << encode(like->userName) << "\n";
                    like = like->next;
                }

                Comment* comment = post->commentHead;
                while (comment) {
                    comments << user->userID << "|" << post->postID << "|" << comment->commentID << "|"
                        << comment->authorUserID << "|" << encode(comment->authorName) << "|"
                        << encode(comment->text) << "|" << encode(comment->timeStamp) << "\n";
                    comment = comment->next;
                }
                post = post->next;
            }
            user = user->next;
        }
    }

    void loadData() {
        authSystem.clear();
        nextPostID = 1;
        nextCommentID = 1;

        string line, f1, f2, f3, f4, f5, f6, f7;
        ifstream users("users.txt");
        while (getline(users, line)) {
            size_t pos = 0;
            if (!nextField(line, pos, f1) || !nextField(line, pos, f2) || !nextField(line, pos, f3) ||
                !nextField(line, pos, f4) || !nextField(line, pos, f5) || !nextField(line, pos, f6) ||
                !nextField(line, pos, f7)) continue;

            User* user = new User(toInt(f1), decode(f2), decode(f3), decode(f4),
                decode(f5), toInt(f6), toInt(f7) != 0);
            if (!authSystem.addUser(user)) delete user;
        }
        users.close();

        ifstream friends("friends.txt");
        while (getline(friends, line)) {
            size_t pos = 0;
            if (!nextField(line, pos, f1) || !nextField(line, pos, f2)) continue;
            User* user = authSystem.findUserByID(toInt(f1));
            if (user) user->addFriend(toInt(f2));
        }
        friends.close();

        ifstream requests("requests.txt");
        while (getline(requests, line)) {
            size_t pos = 0;
            if (!nextField(line, pos, f1) || !nextField(line, pos, f2)) continue;
            User* user = authSystem.findUserByID(toInt(f1));
            if (user) user->addFriendRequest(toInt(f2));
        }
        requests.close();

        ifstream posts("posts.txt");
        while (getline(posts, line)) {
            size_t pos = 0;
            if (!nextField(line, pos, f1) || !nextField(line, pos, f2) ||
                !nextField(line, pos, f3) || !nextField(line, pos, f4)) continue;
            User* user = authSystem.findUserByID(toInt(f1));
            int postID = toInt(f2);
            if (user && postID > 0) {
                user->addPost(new Post(postID, decode(f3), decode(f4)));
                if (postID >= nextPostID) nextPostID = postID + 1;
            }
        }
        posts.close();

        ifstream likes("likes.txt");
        while (getline(likes, line)) {
            size_t pos = 0;
            if (!nextField(line, pos, f1) || !nextField(line, pos, f2) ||
                !nextField(line, pos, f3) || !nextField(line, pos, f4)) continue;
            User* owner = authSystem.findUserByID(toInt(f1));
            Post* post = owner ? owner->findPost(toInt(f2)) : nullptr;
            if (post) post->addLoadedLike(toInt(f3), decode(f4));
        }
        likes.close();

        ifstream comments("comments.txt");
        while (getline(comments, line)) {
            size_t pos = 0;
            if (!nextField(line, pos, f1) || !nextField(line, pos, f2) || !nextField(line, pos, f3) ||
                !nextField(line, pos, f4) || !nextField(line, pos, f5) || !nextField(line, pos, f6) ||
                !nextField(line, pos, f7)) continue;
            User* owner = authSystem.findUserByID(toInt(f1));
            Post* post = owner ? owner->findPost(toInt(f2)) : nullptr;
            int commentID = toInt(f3);
            if (post && commentID > 0) {
                post->addComment(commentID, toInt(f4), decode(f5), decode(f6), decode(f7));
                if (commentID >= nextCommentID) nextCommentID = commentID + 1;
            }
        }
        comments.close();
    }

    bool isLoggedIn() const {
        return currentUser != nullptr;
    }

    User* getCurrentUser() const {
        return currentUser;
    }

    UserAuthentication* getUsers() {
        return &authSystem;
    }

    UserAuthentication* getUsers() const {
        return const_cast<UserAuthentication*>(&authSystem);
    }

    ~Facebook() {
        saveData();
    }
};
