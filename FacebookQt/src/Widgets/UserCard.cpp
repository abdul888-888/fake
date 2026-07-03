#include "UserCard.h"
#include "../Utils/UiHelpers.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

UserCard::UserCard(const UserView& user, Mode mode, QWidget* parent)
    : QFrame(parent), user_(user) {
    setProperty("class", "Card");

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(16, 14, 16, 14);
    root->setSpacing(14);
    root->addWidget(UiHelpers::avatar(user.name, 48, this));

    auto* info = new QVBoxLayout;
    auto* name = new QLabel(user.name, this);
    name->setStyleSheet("font-weight:800;font-size:15px;");
    info->addWidget(name);
    info->addWidget(UiHelpers::muted(QString("ID %1  ·  %2  ·  %3 mutual")
        .arg(user.id)
        .arg(user.isPublic ? "Public" : "Private")
        .arg(user.mutualFriends), this));
    root->addLayout(info, 1);

    auto* view = UiHelpers::button("View Profile", QString(), this);
    root->addWidget(view);
    connect(view, &QPushButton::clicked, this, [this] { emit viewRequested(user_.id); });

    if (mode == SearchResult || mode == Suggestion) {
        if (!user.isCurrentUser && !user.isFriend) {
            auto* add = UiHelpers::button("Add Friend", "PrimaryButton", this);
            root->addWidget(add);
            connect(add, &QPushButton::clicked, this, [this] { emit addRequested(user_.id); });
        }
    }
    else if (mode == Friend) {
        auto* remove = UiHelpers::button("Remove", "DangerButton", this);
        root->addWidget(remove);
        connect(remove, &QPushButton::clicked, this, [this] { emit removeRequested(user_.id); });
    }
    else if (mode == Request) {
        auto* accept = UiHelpers::button("Accept", "PrimaryButton", this);
        auto* reject = UiHelpers::button("Reject", "DangerButton", this);
        root->addWidget(accept);
        root->addWidget(reject);
        connect(accept, &QPushButton::clicked, this, [this] { emit acceptRequested(user_.id); });
        connect(reject, &QPushButton::clicked, this, [this] { emit rejectRequested(user_.id); });
    }
}
