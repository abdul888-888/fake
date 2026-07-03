#include "MainWindow.h"

#include "../Utils/UiHelpers.h"
#include "../Widgets/PostCard.h"
#include "../Widgets/UserCard.h"

#include <QApplication>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QIODevice>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1180, 760);
    setMinimumSize(980, 640);
    setWindowTitle("Facebook Desktop");
    setWindowIcon(QIcon(":/Resources/icons/app.svg"));

    QFile style(":/Resources/styles/light.qss");
    if (style.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(QString::fromUtf8(style.readAll()));
    }

    buildUi();
    connect(&service_, &BackendService::authChanged, this, [this] {
        service_.isLoggedIn() ? switchToShell() : switchToAuth();
    });
    connect(&service_, &BackendService::dataChanged, this, &MainWindow::refreshAll);
    connect(&service_, &BackendService::notificationRaised, this, [this](const QString& title, const QString& detail) {
        statusBar()->showMessage(title + ": " + detail, 4000);
    });

    new QShortcut(QKeySequence("Ctrl+N"), this, [this] { setPage(FeedPage, "News Feed"); postComposer_->setFocus(); });
    new QShortcut(QKeySequence("Ctrl+F"), this, [this] { setPage(SearchPage, "Search"); searchBox_->setFocus(); });
    new QShortcut(QKeySequence("Ctrl+L"), this, [this] { service_.logout(); });
}

void MainWindow::buildUi() {
    rootStack_ = new QStackedWidget(this);
    loginPage_ = buildLoginPage();
    signupPage_ = buildSignupPage();
    shellPage_ = buildShell();
    rootStack_->addWidget(loginPage_);
    rootStack_->addWidget(signupPage_);
    rootStack_->addWidget(shellPage_);
    setCentralWidget(rootStack_);
    switchToAuth();
}

QWidget* MainWindow::buildLoginPage() {
    auto* page = new QWidget(this);
    page->setObjectName("Root");
    auto* outer = new QVBoxLayout(page);
    outer->setAlignment(Qt::AlignCenter);

    auto* card = UiHelpers::card(page);
    card->setFixedWidth(420);
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(32, 30, 32, 30);
    layout->setSpacing(14);

    auto* brand = new QLabel("facebook", card);
    brand->setObjectName("BrandTitle");
    brand->setAlignment(Qt::AlignCenter);
    layout->addWidget(brand);
    layout->addWidget(UiHelpers::muted("Connect with friends from a modern desktop app.", card), 0, Qt::AlignCenter);

    loginEmail_ = new QLineEdit(card);
    loginEmail_->setPlaceholderText("Email address or User ID");
    loginPassword_ = new QLineEdit(card);
    loginPassword_->setPlaceholderText("Password");
    loginPassword_->setEchoMode(QLineEdit::Password);
    showPasswordButton_ = UiHelpers::button("Show password", QString(), card);
    auto* remember = new QCheckBox("Remember me", card);
    auto* login = UiHelpers::button("Log In", "PrimaryButton", card);
    auto* create = UiHelpers::button("Create Account", QString(), card);
    auto* forgot = new QLabel("<a href='#'>Forgot password?</a>", card);
    forgot->setAlignment(Qt::AlignCenter);
    loginError_ = new QLabel(card);
    loginError_->setObjectName("Error");
    loginError_->setWordWrap(true);

    layout->addWidget(loginEmail_);
    layout->addWidget(loginPassword_);
    layout->addWidget(showPasswordButton_);
    layout->addWidget(remember);
    layout->addWidget(login);
    layout->addWidget(create);
    layout->addWidget(forgot);
    layout->addWidget(loginError_);
    outer->addWidget(card);

    auto submit = [this] {
        QString error;
        loginError_->clear();
        bool ok = false;
        const int id = loginEmail_->text().trimmed().toInt(&ok);
        const bool loggedIn = ok && !loginEmail_->text().contains('@')
            ? service_.loginById(id, loginPassword_->text(), &error)
            : service_.loginByEmail(loginEmail_->text(), loginPassword_->text(), &error);
        if (!loggedIn) {
            loginError_->setText(error);
            QMessageBox::warning(this, "Invalid login", error);
        }
    };
    connect(login, &QPushButton::clicked, this, submit);
    connect(loginPassword_, &QLineEdit::returnPressed, this, submit);
    connect(loginEmail_, &QLineEdit::returnPressed, this, submit);
    connect(create, &QPushButton::clicked, this, [this] { rootStack_->setCurrentWidget(signupPage_); });
    connect(showPasswordButton_, &QPushButton::clicked, this, [this] {
        const bool hidden = loginPassword_->echoMode() == QLineEdit::Password;
        loginPassword_->setEchoMode(hidden ? QLineEdit::Normal : QLineEdit::Password);
        showPasswordButton_->setText(hidden ? "Hide password" : "Show password");
    });

    return page;
}

QWidget* MainWindow::buildSignupPage() {
    auto* page = new QWidget(this);
    page->setObjectName("Root");
    auto* outer = new QVBoxLayout(page);
    outer->setAlignment(Qt::AlignCenter);

    auto* card = UiHelpers::card(page);
    card->setFixedWidth(520);
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(12);
    layout->addWidget(UiHelpers::title("Create Account", card));

    signupId_ = new QLineEdit(card);
    signupId_->setPlaceholderText("Unique User ID");
    signupName_ = new QLineEdit(card);
    signupName_->setPlaceholderText("Full Name");
    signupEmail_ = new QLineEdit(card);
    signupEmail_->setPlaceholderText("Email");
    signupPassword_ = new QLineEdit(card);
    signupPassword_->setPlaceholderText("Password");
    signupPassword_->setEchoMode(QLineEdit::Password);
    signupConfirm_ = new QLineEdit(card);
    signupConfirm_->setPlaceholderText("Confirm Password");
    signupConfirm_->setEchoMode(QLineEdit::Password);
    signupAge_ = new QSpinBox(card);
    signupAge_->setRange(13, 120);
    signupGender_ = new QComboBox(card);
    signupGender_->addItems({ "Male", "Female", "Other" });
    signupPrivacy_ = new QComboBox(card);
    signupPrivacy_->addItems({ "Public Profile", "Private Profile" });
    signupError_ = new QLabel(card);
    signupError_->setObjectName("Error");
    signupError_->setWordWrap(true);
    signupButton_ = UiHelpers::button("Sign Up", "PrimaryButton", card);
    auto* back = UiHelpers::button("Back to Login", QString(), card);

    auto statusLabel = [](QWidget* parent) {
        auto* label = new QLabel("○", parent);
        label->setFixedWidth(24);
        label->setAlignment(Qt::AlignCenter);
        return label;
    };
    signupIdStatus_ = statusLabel(card);
    signupNameStatus_ = statusLabel(card);
    signupEmailStatus_ = statusLabel(card);
    signupPasswordStatus_ = statusLabel(card);
    signupConfirmStatus_ = statusLabel(card);
    auto fieldRow = [](QWidget* field, QLabel* status) {
        auto* box = new QWidget(field->parentWidget());
        auto* row = new QHBoxLayout(box);
        row->setContentsMargins(0, 0, 0, 0);
        row->addWidget(field);
        row->addWidget(status);
        return box;
    };

    auto* form = new QFormLayout;
    form->setSpacing(10);
    form->addRow("User ID", fieldRow(signupId_, signupIdStatus_));
    form->addRow("Full Name", fieldRow(signupName_, signupNameStatus_));
    form->addRow("Email", fieldRow(signupEmail_, signupEmailStatus_));
    form->addRow("Password", fieldRow(signupPassword_, signupPasswordStatus_));
    form->addRow("Confirm", fieldRow(signupConfirm_, signupConfirmStatus_));
    form->addRow("Age", signupAge_);
    form->addRow("Gender", signupGender_);
    form->addRow("Privacy", signupPrivacy_);
    layout->addLayout(form);
    layout->addWidget(signupError_);
    layout->addWidget(signupButton_);
    layout->addWidget(back);
    outer->addWidget(card);

    const auto validators = { signupId_, signupName_, signupEmail_, signupPassword_, signupConfirm_ };
    for (QLineEdit* edit : validators) connect(edit, &QLineEdit::textChanged, this, &MainWindow::updateSignupValidation);
    connect(signupAge_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::updateSignupValidation);
    updateSignupValidation();

    connect(signupButton_, &QPushButton::clicked, this, [this] {
        QString error;
        const bool isPublic = signupPrivacy_->currentIndex() == 0;
        if (service_.signUp(signupId_->text().toInt(), signupName_->text(), signupEmail_->text(),
            signupPassword_->text(), signupGender_->currentText(), signupAge_->value(), isPublic, &error)) {
            QMessageBox::information(this, "Registration success", "Your account was created. You can log in now.");
            rootStack_->setCurrentWidget(loginPage_);
            loginEmail_->setText(signupEmail_->text());
        }
        else {
            signupError_->setText(error);
        }
    });
    connect(back, &QPushButton::clicked, this, [this] { rootStack_->setCurrentWidget(loginPage_); });
    return page;
}

QWidget* MainWindow::buildShell() {
    auto* page = new QWidget(this);
    page->setObjectName("Root");
    auto* root = new QHBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* sidebar = new QFrame(page);
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(240);
    auto* side = new QVBoxLayout(sidebar);
    side->setContentsMargins(18, 18, 18, 18);
    auto* brand = new QLabel("facebook", sidebar);
    brand->setObjectName("BrandTitle");
    side->addWidget(brand);
    side->addSpacing(12);

    navGroup_ = new QButtonGroup(this);
    navGroup_->setExclusive(true);
    side->addWidget(navButton("Home Dashboard", ":/Resources/icons/home.svg", DashboardPage));
    side->addWidget(navButton("News Feed", ":/Resources/icons/feed.svg", FeedPage));
    side->addWidget(navButton("Profile", ":/Resources/icons/profile.svg", ProfilePage));
    side->addWidget(navButton("Friends", ":/Resources/icons/friends.svg", FriendsPage));
    side->addWidget(navButton("Friend Requests", ":/Resources/icons/bell.svg", RequestsPage));
    side->addWidget(navButton("Search", ":/Resources/icons/search.svg", SearchPage));
    side->addWidget(navButton("Notifications", ":/Resources/icons/bell.svg", NotificationsPage));
    side->addWidget(navButton("Messages", ":/Resources/icons/message.svg", MessagesPage));
    side->addWidget(navButton("Settings", ":/Resources/icons/settings.svg", SettingsPage));
    side->addStretch();
    auto* logout = UiHelpers::button("Log Out", "DangerButton", sidebar);
    side->addWidget(logout);
    connect(logout, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, "Logout confirmation", "Do you want to log out?") == QMessageBox::Yes) {
            service_.logout();
        }
    });
    root->addWidget(sidebar);

    auto* main = new QWidget(page);
    auto* mainLayout = new QVBoxLayout(main);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    auto* topbar = new QFrame(main);
    topbar->setObjectName("Topbar");
    auto* top = new QHBoxLayout(topbar);
    top->setContentsMargins(24, 14, 24, 14);
    pageTitle_ = new QLabel("Home Dashboard", topbar);
    pageTitle_->setObjectName("PageTitle");
    statusLabel_ = UiHelpers::muted("Ready", topbar);
    top->addWidget(pageTitle_);
    top->addStretch();
    top->addWidget(statusLabel_);
    mainLayout->addWidget(topbar);

    contentStack_ = new QStackedWidget(main);
    contentStack_->addWidget(buildDashboardPage());
    contentStack_->addWidget(buildFeedPage());
    contentStack_->addWidget(buildProfilePage());
    contentStack_->addWidget(buildFriendsPage());
    contentStack_->addWidget(buildRequestsPage());
    contentStack_->addWidget(buildSearchPage());
    contentStack_->addWidget(buildNotificationsPage());
    contentStack_->addWidget(buildMessagesPage());
    contentStack_->addWidget(buildSettingsPage());
    mainLayout->addWidget(contentStack_, 1);
    root->addWidget(main, 1);
    return page;
}

QPushButton* MainWindow::navButton(const QString& text, const QString& iconPath, Page page) {
    auto* button = UiHelpers::button(text, "SidebarButton");
    button->setCheckable(true);
    button->setIcon(QIcon(iconPath));
    navGroup_->addButton(button, static_cast<int>(page));
    connect(button, &QPushButton::clicked, this, [this, page, text] { setPage(page, text); });
    return button;
}

QScrollArea* MainWindow::scrollPage(QVBoxLayout** contentLayout) {
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    auto* content = new QWidget(scroll);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);
    layout->addStretch();
    scroll->setWidget(content);
    *contentLayout = layout;
    return scroll;
}

QWidget* MainWindow::buildDashboardPage() { return scrollPage(&dashboardLayout_); }

QWidget* MainWindow::buildFeedPage() {
    auto* page = scrollPage(&feedLayout_);
    auto* composer = UiHelpers::card(page);
    auto* layout = new QVBoxLayout(composer);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->addWidget(new QLabel("Create Post", composer));
    postComposer_ = new QPlainTextEdit(composer);
    postComposer_->setPlaceholderText("What's on your mind?");
    postComposer_->setFixedHeight(100);
    postCounter_ = UiHelpers::muted("0 / 500 characters", composer);
    auto* actions = new QHBoxLayout;
    auto* attach = UiHelpers::button("Attach image", QString(), composer);
    attach->setToolTip("Image attachment placeholder");
    auto* cancel = UiHelpers::button("Cancel", QString(), composer);
    auto* publish = UiHelpers::button("Post", "PrimaryButton", composer);
    actions->addWidget(attach);
    actions->addStretch();
    actions->addWidget(cancel);
    actions->addWidget(publish);
    layout->addWidget(postComposer_);
    layout->addWidget(postCounter_);
    layout->addLayout(actions);
    feedLayout_->insertWidget(0, composer);
    connect(postComposer_, &QPlainTextEdit::textChanged, this, [this] {
        postCounter_->setText(QString("%1 / 500 characters").arg(postComposer_->toPlainText().length()));
    });
    connect(cancel, &QPushButton::clicked, postComposer_, &QPlainTextEdit::clear);
    connect(publish, &QPushButton::clicked, this, [this] {
        QString error;
        if (service_.createPost(postComposer_->toPlainText(), &error)) postComposer_->clear();
        else handleError(error);
    });
    return page;
}

QWidget* MainWindow::buildProfilePage() { return scrollPage(&profileLayout_); }
QWidget* MainWindow::buildFriendsPage() { return scrollPage(&friendsLayout_); }
QWidget* MainWindow::buildRequestsPage() { return scrollPage(&requestsLayout_); }

QWidget* MainWindow::buildSearchPage() {
    auto* page = scrollPage(&searchResultsLayout_);
    searchBox_ = new QLineEdit(page);
    searchBox_->setPlaceholderText("Search by name, email, or user ID...");
    searchResultsLayout_->insertWidget(0, searchBox_);
    connect(searchBox_, &QLineEdit::textChanged, this, &MainWindow::refreshSearch);
    return page;
}

QWidget* MainWindow::buildNotificationsPage() { return scrollPage(&notificationsLayout_); }

QWidget* MainWindow::buildMessagesPage() {
    QVBoxLayout* layout = nullptr;
    auto* page = scrollPage(&layout);
    auto* card = UiHelpers::card(page);
    auto* box = new QVBoxLayout(card);
    box->addWidget(UiHelpers::title("Messages", card));
    box->addWidget(UiHelpers::muted("Messaging is reserved as a placeholder for the next version.", card));
    layout->insertWidget(0, card);
    return page;
}

QWidget* MainWindow::buildSettingsPage() { return scrollPage(&settingsLayout_); }

QFrame* MainWindow::statCard(const QString& value, const QString& label) {
    auto* card = UiHelpers::card(this);
    auto* layout = new QVBoxLayout(card);
    auto* valueLabel = new QLabel(value, card);
    valueLabel->setStyleSheet("font-size:26px;font-weight:850;color:#1877f2;");
    layout->addWidget(valueLabel);
    layout->addWidget(UiHelpers::muted(label, card));
    return card;
}

void MainWindow::switchToAuth() {
    rootStack_->setCurrentWidget(loginPage_);
    loginPassword_->clear();
}

void MainWindow::switchToShell() {
    rootStack_->setCurrentWidget(shellPage_);
    setPage(DashboardPage, "Home Dashboard");
    refreshAll();
}

void MainWindow::setPage(Page page, const QString& title) {
    pageTitle_->setText(title);
    contentStack_->setCurrentIndex(static_cast<int>(page));
    if (QAbstractButton* button = navGroup_->button(static_cast<int>(page))) button->setChecked(true);
    refreshAll();
}

void MainWindow::refreshAll() {
    if (!service_.isLoggedIn()) return;
    refreshDashboard();
    refreshFeed();
    refreshProfile();
    refreshFriends();
    refreshRequests();
    refreshSearch();
    refreshNotifications();
    refreshSettings();
    const UserView me = service_.currentUserView();
    statusLabel_->setText(me.name + "  ·  " + QString::number(service_.friendRequests().size()) + " requests");
}

void MainWindow::refreshDashboard() {
    UiHelpers::clearLayout(dashboardLayout_);
    const UserView me = service_.currentUserView();
    auto* summary = UiHelpers::card(this);
    auto* row = new QHBoxLayout(summary);
    row->addWidget(UiHelpers::avatar(me.name, 72, summary));
    auto* text = new QVBoxLayout;
    text->addWidget(UiHelpers::title("Welcome, " + me.name, summary));
    text->addWidget(UiHelpers::muted(me.email + "  ·  " + (me.isPublic ? "Public profile" : "Private profile"), summary));
    row->addLayout(text, 1);
    dashboardLayout_->addWidget(summary);

    auto* stats = new QHBoxLayout;
    stats->addWidget(statCard(QString::number(me.friends), "Friends"));
    stats->addWidget(statCard(QString::number(me.posts), "Posts"));
    stats->addWidget(statCard(QString::number(me.likesReceived), "Likes received"));
    stats->addWidget(statCard(QString::number(service_.friendRequests().size()), "Friend requests"));
    dashboardLayout_->addLayout(stats);

    dashboardLayout_->addWidget(UiHelpers::title("Friend Suggestions", this));
    addUserCards(dashboardLayout_, service_.friendSuggestions(), UserCard::Suggestion);
    dashboardLayout_->addWidget(UiHelpers::title("Recent Activity", this));
    addPostCards(dashboardLayout_, service_.newsFeed().mid(0, 3));
    dashboardLayout_->addStretch();
}

void MainWindow::refreshFeed() {
    while (feedLayout_->count() > 1) {
        QLayoutItem* item = feedLayout_->takeAt(1);
        if (QWidget* widget = item->widget()) widget->deleteLater();
        delete item;
    }
    addPostCards(feedLayout_, service_.newsFeed());
    feedLayout_->addStretch();
}

void MainWindow::refreshProfile() {
    UiHelpers::clearLayout(profileLayout_);
    const UserView me = service_.currentUserView();
    auto* cover = UiHelpers::card(this);
    cover->setStyleSheet("QFrame{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1877f2,stop:1 #dbeafe);border-radius:14px;}");
    cover->setMinimumHeight(150);
    auto* coverLayout = new QHBoxLayout(cover);
    coverLayout->addWidget(UiHelpers::avatar(me.name, 96, cover));
    auto* info = new QVBoxLayout;
    auto* name = new QLabel(me.name, cover);
    name->setStyleSheet("font-size:30px;font-weight:850;color:white;");
    info->addWidget(name);
    auto* detail = new QLabel(QString("%1 · Age %2 · %3").arg(me.email).arg(me.age).arg(me.gender), cover);
    detail->setStyleSheet("color:white;font-weight:600;");
    info->addWidget(detail);
    coverLayout->addLayout(info, 1);
    profileLayout_->addWidget(cover);

    auto* actions = new QHBoxLayout;
    actions->addWidget(statCard(QString::number(me.friends), "Friends"));
    actions->addWidget(statCard(QString::number(me.posts), "Posts"));
    actions->addWidget(statCard(QString::number(me.likesReceived), "Likes received"));
    profileLayout_->addLayout(actions);
    profileLayout_->addWidget(UiHelpers::title("Posts", this));
    addPostCards(profileLayout_, service_.postsForUser(me.id));
    profileLayout_->addStretch();
}

void MainWindow::refreshFriends() {
    UiHelpers::clearLayout(friendsLayout_);
    friendsLayout_->addWidget(UiHelpers::title("Friends", this));
    addUserCards(friendsLayout_, service_.friends(), UserCard::Friend);
    friendsLayout_->addStretch();
}

void MainWindow::refreshRequests() {
    UiHelpers::clearLayout(requestsLayout_);
    requestsLayout_->addWidget(UiHelpers::title("Friend Requests", this));
    addUserCards(requestsLayout_, service_.friendRequests(), UserCard::Request);
    requestsLayout_->addStretch();
}

void MainWindow::refreshSearch() {
    while (searchResultsLayout_->count() > 1) {
        QLayoutItem* item = searchResultsLayout_->takeAt(1);
        if (QWidget* widget = item->widget()) widget->deleteLater();
        delete item;
    }
    addUserCards(searchResultsLayout_, service_.allUsers(searchBox_->text()), UserCard::SearchResult);
    searchResultsLayout_->addStretch();
}

void MainWindow::refreshNotifications() {
    UiHelpers::clearLayout(notificationsLayout_);
    notificationsLayout_->addWidget(UiHelpers::title("Notifications", this));
    for (const NotificationView& item : service_.notifications()) {
        auto* card = UiHelpers::card(this);
        if (item.unread) card->setStyleSheet("QFrame{background:#eef4ff;border:1px solid #bfdbfe;border-radius:14px;}");
        auto* box = new QVBoxLayout(card);
        box->addWidget(new QLabel("<b>" + item.title.toHtmlEscaped() + "</b>", card));
        box->addWidget(UiHelpers::muted(item.detail, card));
        notificationsLayout_->addWidget(card);
    }
    notificationsLayout_->addStretch();
}

void MainWindow::refreshSettings() {
    UiHelpers::clearLayout(settingsLayout_);
    const UserView me = service_.currentUserView();
    auto* card = UiHelpers::card(this);
    auto* form = new QFormLayout(card);
    auto* name = new QLineEdit(me.name, card);
    auto* email = new QLineEdit(me.email, card);
    auto* password = new QLineEdit(card);
    password->setPlaceholderText("Leave blank to keep current password");
    password->setEchoMode(QLineEdit::Password);
    auto* privacy = new QComboBox(card);
    privacy->addItems({ "Public Profile", "Private Profile" });
    privacy->setCurrentIndex(me.isPublic ? 0 : 1);
    auto* theme = new QComboBox(card);
    theme->addItems({ "Light Mode", "Dark Mode (placeholder)" });
    auto* save = UiHelpers::button("Save Settings", "PrimaryButton", card);
    form->addRow("Name", name);
    form->addRow("Email", email);
    form->addRow("Password", password);
    form->addRow("Privacy", privacy);
    form->addRow("Theme", theme);
    form->addRow(save);
    settingsLayout_->addWidget(card);
    settingsLayout_->addStretch();
    connect(save, &QPushButton::clicked, this, [this, name, email, password, privacy] {
        QString error;
        if (QMessageBox::question(this, "Confirm changes", "Save profile changes?") != QMessageBox::Yes) return;
        if (!service_.updateProfile(name->text(), email->text(), password->text(), privacy->currentIndex() == 0, &error)) {
            handleError(error);
        }
    });
}

void MainWindow::addPostCards(QVBoxLayout* layout, const QList<PostView>& posts) {
    if (posts.isEmpty()) {
        layout->addWidget(UiHelpers::muted("No posts to show yet.", this));
        return;
    }
    for (const PostView& post : posts) {
        auto* card = new PostCard(post, this);
        connect(card, &PostCard::likeRequested, this, [this](int ownerId, int postId) {
            QString error;
            if (!service_.likePost(ownerId, postId, &error)) handleError(error);
        });
        connect(card, &PostCard::unlikeRequested, this, [this](int ownerId, int postId) {
            QString error;
            if (!service_.unlikePost(ownerId, postId, &error)) handleError(error);
        });
        connect(card, &PostCard::commentRequested, this, [this](int ownerId, int postId, const QString& text) {
            QString error;
            if (!service_.addComment(ownerId, postId, text, &error)) handleError(error);
        });
        connect(card, &PostCard::editRequested, this, [this](int postId, const QString& content) {
            QString error;
            if (!service_.editPost(postId, content, &error)) handleError(error);
        });
        connect(card, &PostCard::deleteRequested, this, [this](int postId) {
            if (QMessageBox::question(this, "Delete confirmation", "Delete this post?") != QMessageBox::Yes) return;
            QString error;
            if (!service_.deletePost(postId, &error)) handleError(error);
        });
        layout->addWidget(card);
    }
}

void MainWindow::addUserCards(QVBoxLayout* layout, const QList<UserView>& users, UserCard::Mode mode) {
    if (users.isEmpty()) {
        layout->addWidget(UiHelpers::muted("Nothing to show here yet.", this));
        return;
    }
    for (const UserView& user : users) {
        auto* card = new UserCard(user, mode, this);
        connect(card, &UserCard::viewRequested, this, [this](int userId) {
            const UserView user = service_.userView(userId);
            QMessageBox::information(this, "Profile",
                QString("%1\nEmail: %2\nPrivacy: %3\nFriends: %4")
                    .arg(user.name, user.email, user.isPublic ? "Public" : "Private")
                    .arg(user.friends));
        });
        connect(card, &UserCard::addRequested, this, [this](int userId) {
            QString error;
            if (!service_.sendFriendRequest(userId, &error)) handleError(error);
        });
        connect(card, &UserCard::acceptRequested, this, [this](int userId) {
            QString error;
            if (!service_.acceptFriendRequest(userId, &error)) handleError(error);
        });
        connect(card, &UserCard::rejectRequested, this, [this](int userId) {
            QString error;
            if (!service_.rejectFriendRequest(userId, &error)) handleError(error);
        });
        connect(card, &UserCard::removeRequested, this, [this](int userId) {
            if (QMessageBox::question(this, "Friend removal", "Remove this friend?") != QMessageBox::Yes) return;
            QString error;
            if (!service_.removeFriend(userId, &error)) handleError(error);
        });
        layout->addWidget(card);
    }
}

void MainWindow::handleError(const QString& message) {
    QMessageBox::warning(this, "Validation error", message);
    statusBar()->showMessage(message, 4000);
}

void MainWindow::updateSignupValidation() {
    const bool idOk = signupId_->text().toInt() > 0;
    const bool nameOk = !signupName_->text().trimmed().isEmpty();
    const bool emailOk = signupEmail_->text().contains('@') && signupEmail_->text().contains('.');
    const bool passwordOk = signupPassword_->text().length() >= 8;
    const bool confirmOk = signupPassword_->text() == signupConfirm_->text() && !signupConfirm_->text().isEmpty();
    const bool ok = idOk && nameOk && emailOk && passwordOk && confirmOk;
    auto setStatus = [](QLabel* label, bool valid) {
        label->setText(valid ? "✓" : "○");
        label->setStyleSheet(valid ? "color:#16833a;font-size:18px;font-weight:900;" : "color:#9aa0a6;font-size:18px;");
        label->setToolTip(valid ? "Valid" : "Needs attention");
    };
    setStatus(signupIdStatus_, idOk);
    setStatus(signupNameStatus_, nameOk);
    setStatus(signupEmailStatus_, emailOk);
    setStatus(signupPasswordStatus_, passwordOk);
    setStatus(signupConfirmStatus_, confirmOk);
    signupButton_->setEnabled(ok);
    signupError_->setObjectName(ok ? "Success" : "Error");
    signupError_->style()->unpolish(signupError_);
    signupError_->style()->polish(signupError_);
    signupError_->setText(ok ? "All fields look good." :
        "Use a positive ID, valid email, matching passwords, and at least 8 password characters.");
}
