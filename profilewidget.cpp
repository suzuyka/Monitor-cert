#include "profilewidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDate>
#include <QInputDialog>
#include <QMessageBox>


ProfileWidget::ProfileWidget(Database *db,
                             int userId,
                             const QString &username,
                             QWidget *parent)
    : QWidget(parent),
      m_db(db),
      m_userId(userId),
      m_username(username)
{
    setupUi();
    loadStats();
}

void ProfileWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);

    auto *title = new QLabel("Личный кабинет пользователя");
    QFont f = title->font();
    f.setPointSize(f.pointSize() + 2);
    f.setBold(true);
    title->setFont(f);
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Блок данных пользователя в "боксе"
    auto *userBox = new QGroupBox("Данные пользователя", this);
    auto *userLayout = new QHBoxLayout(userBox);
    userLayout->setContentsMargins(8, 4, 8, 4);

    userLabel = new QLabel(this);
    userLayout->addWidget(userLabel);

    mainLayout->addWidget(userBox);

    // Блок статистики

    auto *statsBox = new QGroupBox("Статистика по сертификатам", this);
    auto *statsLayout = new QVBoxLayout(statsBox);

    statsLabel = new QLabel(this);
    statsLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    statsLabel->setWordWrap(true);
    statsLayout->addWidget(statsLabel);

    refreshStatsBtn = new QPushButton("Обновить статистику", this);
    statsLayout->addWidget(refreshStatsBtn);

    mainLayout->addWidget(statsBox);

    // Кнопки управления профилем
    auto *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();

    changeNameBtn = new QPushButton("Сменить имя", this);
    changePassBtn = new QPushButton("Сменить пароль", this);

    buttonsLayout->addWidget(changeNameBtn);
    buttonsLayout->addWidget(changePassBtn);

    buttonsLayout->addStretch();
    mainLayout->addLayout(buttonsLayout);

    mainLayout->addStretch();

    // Загрузка отображаемого имени пользователя
    loadDisplayName();

    // Обновление статистики по кнопке
    connect(refreshStatsBtn, &QPushButton::clicked,
            this, &ProfileWidget::loadStats);

    // Смена отображаемого имени
    connect(changeNameBtn, &QPushButton::clicked, [this]() {
        if (!m_db || m_userId < 0)
            return;

        bool ok = false;
        QString currentText = userLabel->text();
        QString initial = currentText.section(": ", 1).trimmed();

        QString newName = QInputDialog::getText(
            this,
            "Смена имени",
            "Новое отображаемое имя:",
            QLineEdit::Normal,
            initial,
            &ok
        );

        newName = newName.trimmed();
        if (!ok || newName.isEmpty())
            return;

        if (m_db->setDisplayName(m_userId, newName.toStdString())) {
            loadDisplayName();
        } else {
            QMessageBox::warning(this, "Ошибка",
                                "Не удалось сохранить новое имя пользователя.");
        }
    });

}

void ProfileWidget::setUser(int userId, const QString &username)
{
    m_userId = userId;
    m_username = username;

    loadDisplayName();
    loadStats();
}


void ProfileWidget::loadStats()
{
    if (!m_db || m_userId < 0) {
        statsLabel->setText("Статистика недоступна: пользователь не задан.");
        return;
    }

    auto certs = m_db->getAllCertificates(m_userId);
    int total = 0;
    int expired = 0;
    int soon = 0;
    int active = 0;

    QDate today = QDate::currentDate();

    for (const auto &c : certs) {
        ++total;
        QString dateStr = QString::fromStdString(c.date);
        QDate d = QDate::fromString(dateStr, "dd-MM-yyyy");
        if (!d.isValid())
            d = QDate::fromString(dateStr, "dd.MM.yyyy");

        if (!d.isValid()) {
            ++active;
            continue;
        }

        int days = today.daysTo(d);
        if (days < 0)
            ++expired;
        else if (days <= 30)
            ++soon;
        else
            ++active;
    }

    statsLabel->setText(
        QString("Всего сертификатов: %1\n"
                "Просроченных: %2\n"
                "Сгорают в ближайшие 30 дней: %3\n"
                "Активных (с запасом по сроку): %4\n")
            .arg(total)
            .arg(expired)
            .arg(soon)
            .arg(active));
}

void ProfileWidget::loadDisplayName()
{
    if (!m_db || m_userId < 0) {
        userLabel->setText("Имя пользователя: <не задано>");
        return;
    }

    std::string dn = m_db->getDisplayName(m_userId);
    QString display = QString::fromStdString(dn);

    if (display.isEmpty())
        display = m_username;

    userLabel->setText("Имя пользователя: " + display);
}

