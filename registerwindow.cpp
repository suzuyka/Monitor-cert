#include "registerwindow.h"
#include "database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

RegisterWindow::RegisterWindow(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel("Регистрация");
    title->setAlignment(Qt::AlignCenter);

    auto *userLayout = new QHBoxLayout;
    userLayout->addWidget(new QLabel("Имя для входа (логин):"));
    userEdit = new QLineEdit;
    userLayout->addWidget(userEdit);

    auto *passLayout = new QHBoxLayout;
    passLayout->addWidget(new QLabel("Пароль:"));
    passEdit = new QLineEdit;
    passEdit->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(passEdit);

    auto *pass2Layout = new QHBoxLayout;
    pass2Layout->addWidget(new QLabel("Повтор пароля:"));
    pass2Edit = new QLineEdit;
    pass2Edit->setEchoMode(QLineEdit::Password);
    pass2Layout->addWidget(pass2Edit);

    auto *btnLayout = new QHBoxLayout;
    auto *regBtn = new QPushButton("Зарегистрироваться");
    btnLayout->addWidget(regBtn);

    layout->addWidget(title);
    layout->addLayout(userLayout);
    layout->addLayout(passLayout);
    layout->addLayout(pass2Layout);
    layout->addLayout(btnLayout);

    QObject::connect(regBtn, &QPushButton::clicked, [this]() { onRegister(); });

    setWindowTitle("Регистрация");
    resize(800, 500);
}

void RegisterWindow::onRegister() {
    QString u = userEdit->text();
    QString p1 = passEdit->text();
    QString p2 = pass2Edit->text();

    if (u.isEmpty() || p1.isEmpty() || p2.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля");
        return;
    }
    if (p1 != p2) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают");
        return;
    }

    if (!m_db->createUser(u.toStdString(), p1.toStdString())) {
        QMessageBox::warning(this, "Ошибка", "Такой пользователь уже существует");
        return;
    }

    QMessageBox::information(this, "Готово", "Регистрация успешна");
    close();
}
