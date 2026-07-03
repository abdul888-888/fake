#pragma once

#include <QFrame>
#include "../Controllers/BackendService.h"

class QLabel;
class QTextEdit;

class PostCard : public QFrame {
    Q_OBJECT

public:
    explicit PostCard(const PostView& post, QWidget* parent = nullptr);

signals:
    void likeRequested(int ownerId, int postId);
    void unlikeRequested(int ownerId, int postId);
    void commentRequested(int ownerId, int postId, const QString& text);
    void editRequested(int postId, const QString& content);
    void deleteRequested(int postId);

private:
    PostView post_;
};
