#pragma once

#include <QFrame>
#include "../Controllers/BackendService.h"

class UserCard : public QFrame {
    Q_OBJECT

public:
    enum Mode { SearchResult, Friend, Request, Suggestion };

    explicit UserCard(const UserView& user, Mode mode, QWidget* parent = nullptr);

signals:
    void viewRequested(int userId);
    void addRequested(int userId);
    void acceptRequested(int userId);
    void rejectRequested(int userId);
    void removeRequested(int userId);

private:
    UserView user_;
};
