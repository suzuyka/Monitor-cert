#include <QApplication>
#include "database.h"
#include "loginwindow.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Database db("/app/data/db.sqlite");

    LoginWindow login(&db);
    
    login.setOnLoginSuccess([&](int userId, const QString &username){
        auto *w = new MainWindow(&db, userId, username);
        w->show();
        login.close();
    });

    login.show();
    return app.exec();
}
