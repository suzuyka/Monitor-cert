#include "profilewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

ProfileWidget::ProfileWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QHBoxLayout(this);

    auto *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QPushButton("Личные данные"));
    leftLayout->addWidget(new QPushButton("Мои проекты"));
    leftLayout->addWidget(new QPushButton("Настройки"));
    leftLayout->addWidget(new QPushButton("Выход"));
    leftLayout->addStretch();

    auto *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(new QLabel("Здравствуйте!"));
    rightLayout->addWidget(new QLabel("Добро пожаловать!\nАнализ \"Управление данными\""));
    rightLayout->addWidget(new QPushButton("Просмотреть"));
    rightLayout->addWidget(new QPushButton("Удалить данные"));
    rightLayout->addWidget(new QPushButton("Выйти"));
    rightLayout->addStretch();

    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);
}
