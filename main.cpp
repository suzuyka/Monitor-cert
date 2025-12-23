#include <QApplication>
#include "database.h"
#include "loginwindow.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Database db("/app/data/db.sqlite");

    LoginWindow login(&db);

    login.setOnLoginSuccess([&db, &login](int uid) {
        auto *mw = new MainWindow(&db, uid);
        mw->show();
        login.close();
    });

    login.show();
    return app.exec();
}
