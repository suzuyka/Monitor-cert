#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "database.h"

class ProfileWidget : public QWidget
{
public:
    explicit ProfileWidget(Database *db,
                           int userId = -1,
                           const QString &username = QString(),
                           QWidget *parent = nullptr);

    void setUser(int userId, const QString &username);

    QPushButton *changePassButton() const { return changePassBtn; }

private:
    Database *m_db = nullptr;
    int m_userId = -1;
    QString m_username;

    QLabel *userLabel = nullptr;
    QLabel *idLabel = nullptr;
    QLabel *statsLabel = nullptr;
    QPushButton *changePassBtn = nullptr;
    QPushButton *refreshStatsBtn = nullptr;
    QPushButton *changeNameBtn = nullptr;

    void setupUi();
    void loadStats();
    void loadDisplayName();
};

#endif // PROFILEWIDGET_H
