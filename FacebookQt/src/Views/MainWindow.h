#pragma once

#include <QMainWindow>
#include "../Controllers/BackendService.h"
#include "../Widgets/UserCard.h"

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QFrame;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QSpinBox;
class QStackedWidget;
class QVBoxLayout;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    enum Page {
        DashboardPage,
        FeedPage,
        ProfilePage,
        FriendsPage,
        RequestsPage,
        SearchPage,
        NotificationsPage,
        MessagesPage,
        SettingsPage
    };

    BackendService service_;
    QStackedWidget* rootStack_{};
    QWidget* loginPage_{};
    QWidget* signupPage_{};
    QWidget* shellPage_{};
    QStackedWidget* contentStack_{};
    QLabel* pageTitle_{};
    QLabel* statusLabel_{};
    QButtonGroup* navGroup_{};

    QLineEdit* loginEmail_{};
    QLineEdit* loginPassword_{};
    QLabel* loginError_{};
    QPushButton* showPasswordButton_{};

    QLineEdit* signupId_{};
    QLineEdit* signupName_{};
    QLineEdit* signupEmail_{};
    QLineEdit* signupPassword_{};
    QLineEdit* signupConfirm_{};
    QSpinBox* signupAge_{};
    QComboBox* signupGender_{};
    QComboBox* signupPrivacy_{};
    QLabel* signupError_{};
    QPushButton* signupButton_{};
    QLabel* signupIdStatus_{};
    QLabel* signupNameStatus_{};
    QLabel* signupEmailStatus_{};
    QLabel* signupPasswordStatus_{};
    QLabel* signupConfirmStatus_{};

    QVBoxLayout* dashboardLayout_{};
    QVBoxLayout* feedLayout_{};
    QVBoxLayout* profileLayout_{};
    QVBoxLayout* friendsLayout_{};
    QVBoxLayout* requestsLayout_{};
    QVBoxLayout* searchResultsLayout_{};
    QVBoxLayout* notificationsLayout_{};
    QVBoxLayout* settingsLayout_{};

    QPlainTextEdit* postComposer_{};
    QLabel* postCounter_{};
    QLineEdit* searchBox_{};

    void buildUi();
    QWidget* buildLoginPage();
    QWidget* buildSignupPage();
    QWidget* buildShell();
    QWidget* buildDashboardPage();
    QWidget* buildFeedPage();
    QWidget* buildProfilePage();
    QWidget* buildFriendsPage();
    QWidget* buildRequestsPage();
    QWidget* buildSearchPage();
    QWidget* buildNotificationsPage();
    QWidget* buildMessagesPage();
    QWidget* buildSettingsPage();

    QPushButton* navButton(const QString& text, const QString& iconPath, Page page);
    QScrollArea* scrollPage(QVBoxLayout** contentLayout);
    QFrame* statCard(const QString& value, const QString& label);

    void switchToAuth();
    void switchToShell();
    void setPage(Page page, const QString& title);
    void refreshAll();
    void refreshDashboard();
    void refreshFeed();
    void refreshProfile();
    void refreshFriends();
    void refreshRequests();
    void refreshSearch();
    void refreshNotifications();
    void refreshSettings();
    void addPostCards(QVBoxLayout* layout, const QList<PostView>& posts);
    void addUserCards(QVBoxLayout* layout, const QList<UserView>& users, UserCard::Mode mode);
    void handleError(const QString& message);
    void showInfo(const QString& title, const QString& message);
    bool confirm(const QString& title, const QString& message);
    void updateSignupValidation();
};
