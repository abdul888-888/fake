#include "BackendService.h"

#include <algorithm>
#include <sstream>

BackendService::BackendService(QObject* parent) : QObject(parent) {}

QString BackendService::toQString(const std::string& value) {
    return QString::fromStdString(value);
}

std::string BackendService::toStdString(const QString& value) {
    return value.trimmed().toStdString();
}

bool BackendService::isBlank(const QString& value) {
    return value.trimmed().isEmpty();
}

User* BackendService::currentUser() const {
    return facebook_.getCurrentUser();
}

User* BackendService::userById(int id) const {
    return facebook_.getUsers()->findUserByID(id);
}

User* BackendService::userByEmail(const QString& email) const {
    return facebook_.getUsers()->findUserByEmail(toStdString(email));
}

bool BackendService::loginByEmail(const QString& email, const QString& password, QString* error) {
    if (facebook_.login(toStdString(email), password.toStdString())) {
        emit authChanged();
        emit dataChanged();
        return true;
    }
    if (error) *error = "Invalid email or password.";
    return false;
}

bool BackendService::loginById(int id, const QString& password, QString* error) {
    if (facebook_.login(id, password.toStdString())) {
        emit authChanged();
        emit dataChanged();
        return true;
    }
    if (error) *error = "Invalid user ID or password.";
    return false;
}

bool BackendService::signUp(int id, const QString& name, const QString& email, const QString& password,
    const QString& gender, int age, bool isPublic, QString* error) {
    if (facebook_.signUp(id, toStdString(name), toStdString(email), password.toStdString(),
        toStdString(gender), age, isPublic)) {
        facebook_.saveData();
        emit notificationRaised("Account created", "Your profile is ready.");
        emit dataChanged();
        return true;
    }
    if (error) *error = "Registration failed. Check the highlighted fields and avoid duplicate IDs or emails.";
    return false;
}

void BackendService::logout() {
    facebook_.logout();
    emit authChanged();
}

void BackendService::save() {
    facebook_.saveData();
}

bool BackendService::isLoggedIn() const {
    return facebook_.isLoggedIn();
}

int BackendService::postCount(User* user) const {
    int count = 0;
    for (Post* post = user ? user->posts : nullptr; post; post = post->next) ++count;
    return count;
}

int BackendService::likesReceived(User* user) const {
    int count = 0;
    for (Post* post = user ? user->posts : nullptr; post; post = post->next) {
        count += post->totalLikes;
    }
    return count;
}

bool BackendService::canView(User* user) const {
    User* viewer = currentUser();
    return viewer && user && (user->isPublic || user->userID == viewer->userID || user->hasFriend(viewer->userID));
}

UserView BackendService::makeUserView(User* user) const {
    UserView view;
    if (!user) return view;
    User* me = currentUser();
    view.id = user->userID;
    view.name = toQString(user->name);
    view.email = toQString(user->email);
    view.gender = toQString(user->gender);
    view.age = user->age;
    view.isPublic = user->isPublic;
    view.isCurrentUser = me && me->userID == user->userID;
    view.isFriend = me && me->hasFriend(user->userID);
    view.friends = user->countFriends();
    view.posts = postCount(user);
    view.likesReceived = likesReceived(user);
    view.mutualFriends = me ? facebook_.countMutualFriends(me, user) : 0;
    return view;
}

PostView BackendService::makePostView(User* owner, Post* post) const {
    PostView view;
    if (!owner || !post) return view;
    User* me = currentUser();
    view.ownerId = owner->userID;
    view.ownerName = toQString(owner->name);
    view.id = post->postID;
    view.content = toQString(post->content);
    view.timestamp = toQString(post->timeStamp);
    view.likes = post->totalLikes;
    view.comments = post->totalComments;
    view.ownedByCurrentUser = me && me->userID == owner->userID;
    view.likedByCurrentUser = me && post->hasLike(me->userID);

    for (Comment* comment = post->commentHead; comment; comment = comment->next) {
        CommentView item;
        item.id = comment->commentID;
        item.authorId = comment->authorUserID;
        item.authorName = toQString(comment->authorName);
        item.text = toQString(comment->text);
        item.timestamp = toQString(comment->timeStamp);
        view.commentList.append(item);
    }
    return view;
}

UserView BackendService::currentUserView() const {
    return makeUserView(currentUser());
}

UserView BackendService::userView(int id) const {
    return makeUserView(userById(id));
}

QList<UserView> BackendService::allUsers(const QString& filter) const {
    QList<UserView> users;
    const QString q = filter.trimmed().toLower();
    for (User* user = facebook_.getUsers()->GetHead(); user; user = user->next) {
        UserView view = makeUserView(user);
        const bool match = q.isEmpty()
            || view.name.toLower().contains(q)
            || view.email.toLower().contains(q)
            || QString::number(view.id).contains(q);
        if (match) users.append(view);
    }
    return users;
}

QList<UserView> BackendService::friends() const {
    QList<UserView> list;
    User* me = currentUser();
    for (FriendNode* node = me ? me->friends : nullptr; node; node = node->next) {
        if (User* friendUser = userById(node->friendID)) list.append(makeUserView(friendUser));
    }
    return list;
}

QList<UserView> BackendService::friendRequests() const {
    QList<UserView> list;
    User* me = currentUser();
    for (FriendRequest* request = me ? me->requests : nullptr; request; request = request->next) {
        if (User* sender = userById(request->fromUserID)) list.append(makeUserView(sender));
    }
    return list;
}

QList<UserView> BackendService::friendSuggestions() const {
    QList<UserView> list;
    User* me = currentUser();
    if (!me) return list;
    for (User* user = facebook_.getUsers()->GetHead(); user; user = user->next) {
        if (user->userID != me->userID && !me->hasFriend(user->userID)) {
            UserView view = makeUserView(user);
            if (view.mutualFriends > 0) list.append(view);
        }
    }
    return list;
}

QList<PostView> BackendService::newsFeed() const {
    QList<PostView> posts;
    User* me = currentUser();
    if (!me) return posts;

    for (User* user = facebook_.getUsers()->GetHead(); user; user = user->next) {
        if (me->hasFriend(user->userID)) {
            for (Post* post = user->posts; post; post = post->next) {
                posts.append(makePostView(user, post));
            }
        }
    }
    std::sort(posts.begin(), posts.end(), [](const PostView& a, const PostView& b) {
        return a.id > b.id;
    });
    return posts;
}

QList<PostView> BackendService::postsForUser(int userId) const {
    QList<PostView> posts;
    User* owner = userById(userId);
    if (!canView(owner)) return posts;
    for (Post* post = owner->posts; post; post = post->next) {
        posts.append(makePostView(owner, post));
    }
    std::sort(posts.begin(), posts.end(), [](const PostView& a, const PostView& b) {
        return a.id > b.id;
    });
    return posts;
}

QList<NotificationView> BackendService::notifications() const {
    QList<NotificationView> list;
    for (const UserView& request : friendRequests()) {
        list.append({ "Friend request", request.name + " wants to connect with you.", true });
    }
    for (const PostView& post : newsFeed()) {
        if (post.likes > 0) list.append({ "Post activity", post.ownerName + "'s post has new likes.", false });
        if (post.comments > 0) list.append({ "New comments", post.ownerName + "'s post has comments.", false });
    }
    if (list.isEmpty()) list.append({ "All caught up", "No new notifications.", false });
    return list;
}

bool BackendService::createPost(const QString& content, QString* error) {
    if (isBlank(content)) {
        if (error) *error = "Post cannot be empty.";
        return false;
    }
    User* me = currentUser();
    const int before = postCount(me);
    facebook_.createPost(toStdString(content));
    const bool ok = postCount(me) > before;
    if (ok) {
        facebook_.saveData();
        emit dataChanged();
        emit notificationRaised("Post published", "Your feed was updated.");
    }
    else if (error) *error = "Could not create the post.";
    return ok;
}

bool BackendService::editPost(int postId, const QString& content, QString* error) {
    User* me = currentUser();
    Post* post = me ? me->findPost(postId) : nullptr;
    if (!post) {
        if (error) *error = "Post not found.";
        return false;
    }
    facebook_.editPost(postId, toStdString(content));
    facebook_.saveData();
    emit dataChanged();
    return true;
}

bool BackendService::deletePost(int postId, QString* error) {
    User* me = currentUser();
    if (!me || !me->findPost(postId)) {
        if (error) *error = "Post not found.";
        return false;
    }
    facebook_.deletePost(postId);
    facebook_.saveData();
    emit dataChanged();
    return true;
}

bool BackendService::likePost(int ownerId, int postId, QString* error) {
    User* owner = userById(ownerId);
    Post* post = owner ? owner->findPost(postId) : nullptr;
    User* me = currentUser();
    if (!owner || !post || !canView(owner)) {
        if (error) *error = "You cannot interact with this post.";
        return false;
    }
    if (me && post->hasLike(me->userID)) {
        if (error) *error = "You already liked this post.";
        return false;
    }
    facebook_.likePost(ownerId, postId);
    facebook_.saveData();
    emit dataChanged();
    return true;
}

bool BackendService::unlikePost(int ownerId, int postId, QString* error) {
    User* owner = userById(ownerId);
    Post* post = owner ? owner->findPost(postId) : nullptr;
    User* me = currentUser();
    if (!owner || !post || !me || !post->hasLike(me->userID)) {
        if (error) *error = "You have not liked this post.";
        return false;
    }
    facebook_.unlikePost(ownerId, postId);
    facebook_.saveData();
    emit dataChanged();
    return true;
}

bool BackendService::addComment(int ownerId, int postId, const QString& text, QString* error) {
    if (isBlank(text)) {
        if (error) *error = "Comment cannot be empty.";
        return false;
    }
    User* owner = userById(ownerId);
    if (!owner || !owner->findPost(postId) || !canView(owner)) {
        if (error) *error = "You cannot comment on this post.";
        return false;
    }
    facebook_.commentOnPost(ownerId, postId, toStdString(text));
    facebook_.saveData();
    emit dataChanged();
    return true;
}

bool BackendService::deleteComment(int ownerId, int postId, int commentId, QString* error) {
    User* owner = userById(ownerId);
    Post* post = owner ? owner->findPost(postId) : nullptr;
    if (!post) {
        if (error) *error = "Comment not found.";
        return false;
    }
    facebook_.deleteComment(ownerId, postId, commentId);
    facebook_.saveData();
    emit dataChanged();
    return true;
}

bool BackendService::sendFriendRequest(int userId, QString* error) {
    User* me = currentUser();
    User* target = userById(userId);
    if (!me) {
        if (error) *error = "Please log in first.";
        return false;
    }
    if (!target) {
        if (error) *error = "User was not found.";
        return false;
    }
    if (me->userID == userId) {
        if (error) *error = "You cannot send a friend request to yourself.";
        return false;
    }
    if (me->hasFriend(userId)) {
        if (error) *error = "You are already friends with this user.";
        return false;
    }
    if (target->hasFriendRequest(me->userID)) {
        if (error) *error = "A friend request is already pending.";
        return false;
    }
    if (me->hasFriendRequest(userId)) {
        if (error) *error = "This user already sent you a request. Open Friend Requests to accept or reject it.";
        return false;
    }
    facebook_.sendFriendRequest(userId);
    facebook_.saveData();
    emit dataChanged();
    return true;
}

bool BackendService::acceptFriendRequest(int userId, QString* error) {
    User* me = currentUser();
    if (!me || !me->hasFriendRequest(userId)) {
        if (error) *error = "Friend request was not found.";
        return false;
    }
    facebook_.acceptFriendRequest(userId);
    facebook_.saveData();
    emit dataChanged();
    emit notificationRaised("Friend added", "Your friend list was updated.");
    return true;
}

bool BackendService::rejectFriendRequest(int userId, QString* error) {
    User* me = currentUser();
    if (!me || !me->hasFriendRequest(userId)) {
        if (error) *error = "Friend request was not found.";
        return false;
    }
    facebook_.rejectFriendRequest(userId);
    facebook_.saveData();
    emit dataChanged();
    return true;
}

bool BackendService::removeFriend(int userId, QString* error) {
    User* me = currentUser();
    if (!me || !me->hasFriend(userId)) {
        if (error) *error = "This user is not in your friends list.";
        return false;
    }
    facebook_.removeFriend(userId);
    facebook_.saveData();
    emit dataChanged();
    return true;
}

bool BackendService::updateProfile(const QString& name, const QString& email, const QString& password,
    bool isPublic, QString* error) {
    User* me = currentUser();
    if (!me) {
        if (error) *error = "Please login first.";
        return false;
    }
    if (!UserAuthentication::isValidName(toStdString(name))) {
        if (error) *error = "Name can contain only letters and spaces.";
        return false;
    }
    User* ownerOfEmail = userByEmail(email);
    if (!UserAuthentication::isValidEmail(toStdString(email)) || (ownerOfEmail && ownerOfEmail != me)) {
        if (error) *error = "Email is invalid or already used.";
        return false;
    }
    if (!password.isEmpty() && !UserAuthentication::isValidPassword(password.toStdString())) {
        if (error) *error = "Password must include uppercase, lowercase, digit, and special character.";
        return false;
    }

    me->name = toStdString(name);
    me->email = toStdString(email);
    if (!password.isEmpty()) me->password = password.toStdString();
    me->isPublic = isPublic;
    facebook_.saveData();
    emit dataChanged();
    return true;
}
