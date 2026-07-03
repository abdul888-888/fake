#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include "../../Backend/Facebook.h"

struct CommentView {
    int id{};
    int authorId{};
    QString authorName;
    QString text;
    QString timestamp;
};

struct PostView {
    int ownerId{};
    QString ownerName;
    int id{};
    QString content;
    QString timestamp;
    int likes{};
    int comments{};
    bool likedByCurrentUser{};
    bool ownedByCurrentUser{};
    QList<CommentView> commentList;
};

struct UserView {
    int id{};
    QString name;
    QString email;
    QString gender;
    int age{};
    bool isPublic{};
    bool isFriend{};
    bool isCurrentUser{};
    int friends{};
    int posts{};
    int likesReceived{};
    int mutualFriends{};
};

struct NotificationView {
    QString title;
    QString detail;
    bool unread{};
};

class BackendService : public QObject {
    Q_OBJECT

public:
    explicit BackendService(QObject* parent = nullptr);

    bool loginByEmail(const QString& email, const QString& password, QString* error = nullptr);
    bool loginById(int id, const QString& password, QString* error = nullptr);
    bool signUp(int id, const QString& name, const QString& email, const QString& password,
        const QString& gender, int age, bool isPublic, QString* error = nullptr);
    void logout();
    void save();

    bool isLoggedIn() const;
    UserView currentUserView() const;
    UserView userView(int id) const;
    QList<UserView> allUsers(const QString& filter = QString()) const;
    QList<UserView> friends() const;
    QList<UserView> friendRequests() const;
    QList<UserView> friendSuggestions() const;
    QList<PostView> newsFeed() const;
    QList<PostView> postsForUser(int userId) const;
    QList<NotificationView> notifications() const;

    bool createPost(const QString& content, QString* error = nullptr);
    bool editPost(int postId, const QString& content, QString* error = nullptr);
    bool deletePost(int postId, QString* error = nullptr);
    bool likePost(int ownerId, int postId, QString* error = nullptr);
    bool unlikePost(int ownerId, int postId, QString* error = nullptr);
    bool addComment(int ownerId, int postId, const QString& text, QString* error = nullptr);
    bool deleteComment(int ownerId, int postId, int commentId, QString* error = nullptr);
    bool sendFriendRequest(int userId, QString* error = nullptr);
    bool acceptFriendRequest(int userId, QString* error = nullptr);
    bool rejectFriendRequest(int userId, QString* error = nullptr);
    bool removeFriend(int userId, QString* error = nullptr);
    bool updateProfile(const QString& name, const QString& email, const QString& password,
        bool isPublic, QString* error = nullptr);

signals:
    void authChanged();
    void dataChanged();
    void notificationRaised(const QString& title, const QString& detail);

private:
    Facebook facebook_;

    static QString toQString(const std::string& value);
    static std::string toStdString(const QString& value);
    static bool isBlank(const QString& value);

    User* currentUser() const;
    User* userById(int id) const;
    User* userByEmail(const QString& email) const;
    int postCount(User* user) const;
    int likesReceived(User* user) const;
    bool canView(User* user) const;
    UserView makeUserView(User* user) const;
    PostView makePostView(User* owner, Post* post) const;
};
