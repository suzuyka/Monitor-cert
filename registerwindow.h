#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>

class QLineEdit;
class Database;

class RegisterWindow : public QWidget {
public:
    explicit RegisterWindow(Database *db, QWidget *parent = nullptr);

private:
    void onRegister();

    Database *m_db;
    QLineEdit *userEdit;
    QLineEdit *passEdit;
    QLineEdit *pass2Edit;
};

#endif // REGISTERWINDOW_H
