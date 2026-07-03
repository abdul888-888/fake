#include <QApplication>
#include "Views/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Facebook Desktop");
    QApplication::setOrganizationName("DSA Facebook");

    MainWindow window;
    window.show();
    return app.exec();
}
