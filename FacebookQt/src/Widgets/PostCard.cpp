#include "PostCard.h"
#include "../Utils/UiHelpers.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

PostCard::PostCard(const PostView& post, QWidget* parent)
    : QFrame(parent), post_(post) {
    setProperty("class", "Card");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 16, 18, 16);
    root->setSpacing(12);

    auto* header = new QHBoxLayout;
    header->addWidget(UiHelpers::avatar(post.ownerName, 44, this));

    auto* meta = new QVBoxLayout;
    auto* name = new QLabel(post.ownerName, this);
    name->setStyleSheet("font-weight:800;font-size:15px;");
    meta->addWidget(name);
    meta->addWidget(UiHelpers::muted(post.timestamp, this));
    header->addLayout(meta);
    header->addStretch();

    if (post.ownedByCurrentUser) {
        auto* edit = UiHelpers::button("Edit", QString(), this);
        auto* del = UiHelpers::button("Delete", "DangerButton", this);
        connect(edit, &QPushButton::clicked, this, [this] {
            bool ok = false;
            const QString value = QInputDialog::getMultiLineText(this, "Edit post",
                "Post content", post_.content, &ok);
            if (ok && !value.trimmed().isEmpty()) emit editRequested(post_.id, value);
        });
        connect(del, &QPushButton::clicked, this, [this] { emit deleteRequested(post_.id); });
        header->addWidget(edit);
        header->addWidget(del);
    }
    root->addLayout(header);

    auto* content = new QLabel(post.content, this);
    content->setWordWrap(true);
    content->setTextInteractionFlags(Qt::TextSelectableByMouse);
    content->setStyleSheet("font-size:15px;line-height:145%;");
    root->addWidget(content);

    root->addWidget(UiHelpers::muted(QString("%1 likes  ·  %2 comments")
        .arg(post.likes)
        .arg(post.comments), this));

    if (!post.commentList.isEmpty()) {
        auto* comments = UiHelpers::card(this);
        comments->setGraphicsEffect(nullptr);
        comments->setStyleSheet("QFrame{background:#f8fafc;border:1px solid #e4e6eb;border-radius:10px;}");
        auto* commentLayout = new QVBoxLayout(comments);
        commentLayout->setContentsMargins(12, 10, 12, 10);
        for (const CommentView& comment : post.commentList) {
            auto* label = new QLabel(QString("<b>%1</b><br>%2<br><span style='color:#65676b'>%3</span>")
                .arg(comment.authorName.toHtmlEscaped(), comment.text.toHtmlEscaped(), comment.timestamp.toHtmlEscaped()), comments);
            label->setWordWrap(true);
            commentLayout->addWidget(label);
        }
        root->addWidget(comments);
    }

    auto* actions = new QHBoxLayout;
    auto* like = UiHelpers::button(post.likedByCurrentUser ? "Unlike" : "Like", QString(), this);
    auto* comment = UiHelpers::button("Comment", QString(), this);
    auto* share = UiHelpers::button("Share", QString(), this);
    share->setToolTip("Placeholder action");
    actions->addWidget(like);
    actions->addWidget(comment);
    actions->addWidget(share);
    root->addLayout(actions);

    auto* composer = new QPlainTextEdit(this);
    composer->setPlaceholderText("Write a comment...");
    composer->setFixedHeight(62);
    auto* sendComment = UiHelpers::button("Post comment", "PrimaryButton", this);
    root->addWidget(composer);
    root->addWidget(sendComment, 0, Qt::AlignRight);

    connect(like, &QPushButton::clicked, this, [this] {
        if (post_.likedByCurrentUser) emit unlikeRequested(post_.ownerId, post_.id);
        else emit likeRequested(post_.ownerId, post_.id);
    });
    connect(comment, &QPushButton::clicked, composer, qOverload<>(&QWidget::setFocus));
    connect(sendComment, &QPushButton::clicked, this, [this, composer] {
        const QString text = composer->toPlainText().trimmed();
        if (!text.isEmpty()) emit commentRequested(post_.ownerId, post_.id, text);
    });
}
