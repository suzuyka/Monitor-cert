#include "loginwindow.h"
#include "database.h"
#include "registerwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

LoginWindow::LoginWindow(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("Вход в систему");
    title->setAlignment(Qt::AlignCenter);

    auto *userLayout = new QHBoxLayout;
    userLayout->addWidget(new QLabel("Имя пользователя:"));
    userEdit = new QLineEdit;
    userLayout->addWidget(userEdit);

    auto *passLayout = new QHBoxLayout;
    passLayout->addWidget(new QLabel("Пароль:"));
    passEdit = new QLineEdit;
    passEdit->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(passEdit);

    auto *buttonsLayout = new QHBoxLayout;
    auto *loginBtn = new QPushButton("Войти");
    auto *cancelBtn = new QPushButton("Отмена");
    buttonsLayout->addWidget(loginBtn);
    buttonsLayout->addWidget(cancelBtn);

    auto *registerLink = new QPushButton("Нет аккаунта? Регистрация");
    registerLink->setFlat(true);

    layout->addWidget(title);
    layout->addLayout(userLayout);
    layout->addLayout(passLayout);
    layout->addLayout(buttonsLayout);
    layout->addWidget(registerLink);

    QObject::connect(loginBtn, &QPushButton::clicked, [this]() { onLogin(); });
    QObject::connect(cancelBtn, &QPushButton::clicked, [this]() { close(); });
    QObject::connect(registerLink, &QPushButton::clicked,
                     [this]() { onRegisterLink(); });

    setWindowTitle("Вход");
    resize(800, 500);
}

void LoginWindow::setOnLoginSuccess(const std::function<void(int)> &cb) {
    onLoginSuccess = cb;
}

void LoginWindow::onLogin() {
    std::string u = userEdit->text().toStdString();
    std::string p = passEdit->text().toStdString();

    int uid = m_db->getUserId(u, p);
    if (uid > 0) {
        if (onLoginSuccess) onLoginSuccess(uid);
    } else {
        QMessageBox::warning(this,
                             "Ошибка",
                             "Неверное имя пользователя или пароль");
    }
}

void LoginWindow::onRegisterLink() {
    auto *reg = new RegisterWindow(m_db);
    reg->show();
}
