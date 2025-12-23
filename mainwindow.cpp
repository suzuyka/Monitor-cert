#include "mainwindow.h"
#include "dataformwidget.h"
#include "database.h"

#include <vector>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QDate>
#include <QCheckBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QFrame>
#include <QDir>
#include <QDateTime>

// ===== Item для сортировки по дате =====
class DateItem : public QTableWidgetItem
{
public:
    using QTableWidgetItem::QTableWidgetItem;

    bool operator<(const QTableWidgetItem &other) const override
    {
        QVariant a = data(Qt::UserRole);
        QVariant b = other.data(Qt::UserRole);
        if (a.canConvert<QDate>() && b.canConvert<QDate>())
            return a.toDate() < b.toDate();
        return QTableWidgetItem::operator<(other);
    }
};

// ===== разбор даты dd-MM-yyyy / dd.MM.yyyy =====
static QDate parseDate(const QString &s)
{
    QDate d = QDate::fromString(s, "dd-MM-yyyy");
    if (!d.isValid())
        d = QDate::fromString(s, "dd.MM.yyyy");
    return d;
}

MainWindow::MainWindow(Database *db, int userId, QWidget *parent)
    : QMainWindow(parent),
      m_db(db),
      currentUserId(userId),
      settingsWidget(new QWidget(this)),
      settingsLabel(new QLabel("Раздел настроек", settingsWidget))
{
    setupUi();
    loadTable(false);
}

// ================== UI ==================
void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);

    // верхняя панель
    auto *topLayout  = new QHBoxLayout;
    auto *homeBtn    = new QPushButton("Главная");
    auto *dataBtn    = new QPushButton("Данные");
    auto *reportsBtn = new QPushButton("Отчёты");
    auto *settingsBtn= new QPushButton("Настройки");

    topLayout->addWidget(homeBtn);
    topLayout->addWidget(dataBtn);
    topLayout->addWidget(reportsBtn);
    topLayout->addWidget(settingsBtn);
    topLayout->addStretch();

    // ===== панель фильтров по сроку =====
    auto *filterWidget = new QWidget(this);
    auto *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(0, 0, 0, 0);

    auto *expiredCheck = new QCheckBox("Только просроченные");
    auto *soonCheck    = new QCheckBox("Сгорают ≤ 30 дней");

    filterLayout->addWidget(expiredCheck);
    filterLayout->addWidget(soonCheck);
    filterLayout->addStretch();
    filterWidget->hide(); // по умолчанию не видно

    // центральная область
    auto *centerLayout = new QHBoxLayout;
    table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"№", "Название", "Тип", "Дата", "Действия"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSortingEnabled(true);
    centerLayout->addWidget(table, 2);

    formWidget = new DataFormWidget(this);
    centerLayout->addWidget(formWidget, 1);
    formWidget->hide();

    auto *centerFrame = new QFrame(this);
    centerFrame->setLayout(centerLayout);
    centerFrame->setFrameShape(QFrame::StyledPanel);
    centerFrame->setFrameShadow(QFrame::Raised);
    centerFrame->setLineWidth(1);

    // ===== виджет отчётов =====
    auto *reportsWidget = new QWidget(this);
    auto *reportsOuter  = new QVBoxLayout(reportsWidget);
    reportsOuter->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto *reportsTitle = new QLabel("Отчёты по сертификатам");
    reportsTitle->setAlignment(Qt::AlignCenter);
    reportsOuter->addWidget(reportsTitle);

    statsLabel = new QLabel;
    statsLabel->setAlignment(Qt::AlignCenter);
    reportsOuter->addWidget(statsLabel);

    auto *buttonsRow = new QHBoxLayout;
    buttonsRow->addStretch();

    auto *buttonsCol    = new QVBoxLayout;
    auto *exportAllBtn  = new QPushButton("Скачать все сертификаты");
    auto *exportExpBtn  = new QPushButton("Скачать просроченные");
    auto *exportSoonBtn = new QPushButton("Скачать сгорающие");
    auto *importBtn     = new QPushButton("Импортировать базу из CSV");

    int btnWidth = 280;
    exportAllBtn->setFixedWidth(btnWidth);
    exportExpBtn->setFixedWidth(btnWidth);
    exportSoonBtn->setFixedWidth(btnWidth);

    buttonsCol->addWidget(exportAllBtn);
    buttonsCol->addWidget(exportExpBtn);
    buttonsCol->addWidget(exportSoonBtn);
    buttonsCol->addWidget(importBtn);
    buttonsCol->setAlignment(Qt::AlignHCenter);

    buttonsRow->addLayout(buttonsCol);
    buttonsRow->addStretch();

    reportsOuter->addLayout(buttonsRow);
    reportsOuter->addStretch();

    // ===== виджет настроек =====
    auto *settingsOuter = new QVBoxLayout(settingsWidget);
    settingsOuter->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    settingsOuter->addWidget(settingsLabel);

    auto *changePassBtn = new QPushButton("Сменить пароль");
    auto *aboutBtn      = new QPushButton("О программе");
    auto *logoutBtn     = new QPushButton("Выйти из приложения");

    settingsOuter->addWidget(changePassBtn);
    settingsOuter->addWidget(aboutBtn);
    settingsOuter->addWidget(logoutBtn);
    settingsOuter->addStretch();

    // ===== поиск =====
    auto *searchLayout = new QHBoxLayout;
    auto *searchLabel  = new QLabel("Поиск по названию:");
    auto *searchEdit   = new QLineEdit;
    auto *searchGoBtn  = new QPushButton("Найти");

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchGoBtn);

    searchLabel->hide();
    searchEdit->hide();
    searchGoBtn->hide();

    // ===== нижняя панель =====
    auto *bottomWidget = new QWidget(this);
    auto *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    auto *addBtn    = new QPushButton("Добавить");
    auto *editBtn   = new QPushButton("Редактировать");
    auto *delBtn    = new QPushButton("Удалить");
    auto *searchBtn = new QPushButton("Поиск");

    bottomLayout->addWidget(addBtn);
    bottomLayout->addWidget(editBtn);
    bottomLayout->addWidget(delBtn);
    bottomLayout->addWidget(searchBtn);
    bottomLayout->addStretch();

    // сборка
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(filterWidget);
    mainLayout->addWidget(centerFrame);
    mainLayout->addWidget(reportsWidget);
    mainLayout->addWidget(settingsWidget);
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(bottomWidget);

    // ================== логика UI ==================

    auto hideSearch = [=]()
    {
        searchLabel->hide();
        searchEdit->hide();
        searchGoBtn->hide();
        searchEdit->clear();
        for (int row = 0; row < table->rowCount(); ++row)
            table->setRowHidden(row, false);
    };

    // нижние кнопки
    QObject::connect(addBtn, &QPushButton::clicked, [this]()
    {
        if (formWidget->isVisible()) {
            formWidget->hide();
            return;
        }

        formWidget->clearForm();
        formWidget->setEnabled(true);
        formWidget->show();
    });

    QObject::connect(editBtn, &QPushButton::clicked, [this]()
    {
        int row = table->currentRow();
        if (row < 0)
            return;

        if (formWidget->isVisible()) {
            formWidget->hide();
            return;
        }

        onEdit();
        formWidget->setEnabled(true);
        formWidget->show();
    });

    QObject::connect(delBtn, &QPushButton::clicked, [this]()
    {
        onDelete();
        formWidget->hide();
    });

    // поиск
    QObject::connect(searchBtn, &QPushButton::clicked, [=]()
    {
        if (searchLabel->isVisible())
            hideSearch();
        else {
            searchLabel->show();
            searchEdit->show();
            searchGoBtn->show();
        }
    });

    QObject::connect(searchGoBtn, &QPushButton::clicked, [=]()
    {
        QString text = searchEdit->text();
        for (int row = 0; row < table->rowCount(); ++row)
        {
            bool match = table->item(row, 1)->text()
                             .contains(text, Qt::CaseInsensitive);
            table->setRowHidden(row, !match);
        }
    });

    // вкладка "Главная"
    QObject::connect(homeBtn, &QPushButton::clicked, [=]()
    {
        table->show();
        formWidget->hide();
        reportsWidget->hide();
        settingsWidget->hide();

        filterWidget->hide();
        hideSearch();

        loadTable(false);
        table->sortItems(0, Qt::AscendingOrder);

        for (int row = 0; row < table->rowCount(); ++row)
            table->setRowHidden(row, false);

        bottomWidget->show();
    });

    // вкладка "Данные"
    auto reloadDataTab = [=]()
    {
        table->show();
        formWidget->hide();
        reportsWidget->hide();
        settingsWidget->hide();

        filterWidget->show();
        hideSearch();

        loadTable(true);
        table->sortItems(3, Qt::AscendingOrder);

        QDate today = QDate::currentDate();
        for (int row = 0; row < table->rowCount(); ++row)
        {
            QDate d = parseDate(table->item(row, 3)->text());
            if (!d.isValid())
            {
                table->setRowHidden(row, false);
                continue;
            }
            int days = today.daysTo(d);

            bool show = true;
            if (expiredCheck->isChecked())
                show = (days < 0);
            if (soonCheck->isChecked())
                show = show && (days >= 0 && days <= 30);

            table->setRowHidden(row, !show);
        }

        bottomWidget->hide();
    };

    QObject::connect(dataBtn, &QPushButton::clicked, reloadDataTab);

    QObject::connect(expiredCheck, &QCheckBox::toggled,
                     [=](bool){ reloadDataTab(); });
    QObject::connect(soonCheck, &QCheckBox::toggled,
                     [=](bool){ reloadDataTab(); });

    // вкладка "Отчёты"
    QObject::connect(reportsBtn, &QPushButton::clicked, [=]()
    {
        table->hide();
        formWidget->hide();
        reportsWidget->show();
        settingsWidget->hide();

        filterWidget->hide();
        hideSearch();

        bottomWidget->hide();

        updateStatsLabel();
    });

    QObject::connect(exportAllBtn, &QPushButton::clicked,
                     [this](){ exportCertificates(ExportMode::All); });

    QObject::connect(exportExpBtn, &QPushButton::clicked,
                     [this](){ exportCertificates(ExportMode::Expired); });

    QObject::connect(exportSoonBtn, &QPushButton::clicked,
                     [this](){ exportCertificates(ExportMode::Soon); });

    QObject::connect(importBtn, &QPushButton::clicked,
                     this, &MainWindow::onImportCertificates);

    // вкладка "Настройки"
    QObject::connect(settingsBtn, &QPushButton::clicked, [=]()
    {
        table->hide();
        formWidget->hide();
        reportsWidget->hide();
        settingsWidget->show();

        filterWidget->hide();
        hideSearch();

        bottomWidget->hide();
    });

    QObject::connect(aboutBtn, &QPushButton::clicked,
                     this, &MainWindow::showAbout);
    QObject::connect(changePassBtn, &QPushButton::clicked,
                     this, &MainWindow::onChangePassword);
    QObject::connect(logoutBtn, &QPushButton::clicked,
                     this, &MainWindow::onLogout);

    formWidget->onSave = [this](const Certificate &c)
    { onSaveForm(c); };

    formWidget->onCancel = [this]()
    {
        onCancelForm();
        formWidget->hide();
    };

    reportsWidget->hide();
    settingsWidget->hide();
    filterWidget->hide();
    hideSearch();
    bottomWidget->show();
    formWidget->hide();

    setWindowTitle("Мониторинг срока действий сертификатов ИБ");
    resize(1100, 750);
}

// ================== загрузка таблицы ==================
void MainWindow::loadTable(bool withHighlight)
{
    // кеш на уровне класса: std::vector<Certificate> currentCerts;
    currentCerts = m_db->getAllCertificates(currentUserId);

    bool wasSorting = table->isSortingEnabled();
    table->setSortingEnabled(false);

    table->setRowCount(0);
    QDate today = QDate::currentDate();

    for (int i = 0; i < static_cast<int>(currentCerts.size()); ++i)
    {
        const auto &c = currentCerts[i];

        int row = table->rowCount();
        table->insertRow(row);

        // порядковый номер
        table->setItem(row, 0,
                       new QTableWidgetItem(QString::number(row + 1)));
        table->setItem(row, 1,
                       new QTableWidgetItem(QString::fromStdString(c.name)));
        table->setItem(row, 2,
                       new QTableWidgetItem(QString::fromStdString(c.type)));

        QString dateStr = QString::fromStdString(c.date);
        QDate d = parseDate(dateStr);

        auto *dateItem = new DateItem(dateStr);
        if (d.isValid())
            dateItem->setData(Qt::UserRole, d);
        table->setItem(row, 3, dateItem);

        QString actionText =
            c.description.empty()
                ? "Ред. / Уд."
                : QString::fromStdString(c.description);
        table->setItem(row, 4, new QTableWidgetItem(actionText));

        if (withHighlight && d.isValid())
        {
            int days = today.daysTo(d);
            QColor bg;
            if (days < 0)
                bg = QColor(255, 200, 200);
            else if (days <= 30)
                bg = QColor(255, 240, 200);
            else
                bg = QColor(Qt::white);

            for (int col = 0; col < table->columnCount(); ++col)
            {
                auto *item = table->item(row, col);
                if (!item)
                    continue;
                item->setBackground(bg);
                item->setForeground(Qt::black);
            }
        }
    }

    table->setSortingEnabled(wasSorting);
}

// ================== работа с формой ==================
void MainWindow::onAdd()
{
    formWidget->clearForm();
}

void MainWindow::onEdit()
{
    int row = table->currentRow();
    if (row < 0 || row >= static_cast<int>(currentCerts.size()))
        return;

    const auto &c = currentCerts[row];
    formWidget->setCertificate(c);
}

void MainWindow::onDelete()
{
    int row = table->currentRow();
    if (row < 0 || row >= static_cast<int>(currentCerts.size()))
        return;

    int id = currentCerts[row].id;
    m_db->deleteCertificate(id);
    loadTable(false);
}

void MainWindow::onSaveForm(const Certificate &c)
{
    if (c.id < 0)
        m_db->addCertificate(c, currentUserId);
    else
        m_db->updateCertificate(c);

    loadTable(false);
    formWidget->clearForm();
    formWidget->hide();
}

void MainWindow::onCancelForm()
{
    formWidget->hide();
}

// ================== экспорт / отчёты ==================
void MainWindow::exportCertificates(ExportMode mode)
{
    auto certs = m_db->getAllCertificates(currentUserId);

    QString defaultName;
    switch (mode)
    {
    case ExportMode::All:
        defaultName = "certificates_all.csv";
        break;
    case ExportMode::Expired:
        defaultName = "certificates_expired.csv";
        break;
    case ExportMode::Soon:
        defaultName = "certificates_soon.csv";
        break;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Сохранить отчёт",
        defaultName,
        "CSV файлы (*.csv)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << "ID;Название;Тип;Дата;Описание\n";

    QDate today = QDate::currentDate();

    for (const auto &c : certs)
    {
        QDate d = parseDate(QString::fromStdString(c.date));
        int days = d.isValid() ? today.daysTo(d) : 0;

        bool pass = true;
        if (mode == ExportMode::Expired)
            pass = d.isValid() && days < 0;
        else if (mode == ExportMode::Soon)
            pass = d.isValid() && days >= 0 && days <= 30;

        if (!pass)
            continue;

        out << c.id << ';'
            << QString::fromStdString(c.name) << ';'
            << QString::fromStdString(c.type) << ';'
            << QString::fromStdString(c.date) << ';'
            << QString::fromStdString(c.description) << '\n';
    }
}

void MainWindow::updateStatsLabel()
{
    if (!statsLabel)
        return;

    auto certs = m_db->getAllCertificates(currentUserId);

    int total = 0;
    int expired = 0;
    int soon = 0;
    int active = 0;

    QDate today = QDate::currentDate();

    for (const auto &c : certs)
    {
        ++total;
        QDate d = parseDate(QString::fromStdString(c.date));
        if (!d.isValid())
        {
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
        QString("\nВсего сертификатов: %1\n"
                "Просроченных: %2\n"
                "Сгорают в ближайшие 30 дней: %3\n"
                "Активных (с запасом по сроку): %4\n")
            .arg(total)
            .arg(expired)
            .arg(soon)
            .arg(active));
}

// ================== прочее ==================
void MainWindow::onChangePassword() {
    QDialog dlg(this);
    dlg.setWindowTitle("Смена пароля");

    auto *layout = new QFormLayout(&dlg);

    auto *oldPass = new QLineEdit(&dlg);
    oldPass->setEchoMode(QLineEdit::Password);
    auto *newPass = new QLineEdit(&dlg);
    newPass->setEchoMode(QLineEdit::Password);
    auto *confirmPass = new QLineEdit(&dlg);
    confirmPass->setEchoMode(QLineEdit::Password);

    layout->addRow("Старый пароль:", oldPass);
    layout->addRow("Новый пароль:", newPass);
    layout->addRow("Подтвердите:", confirmPass);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, [&]() {
        QString old = oldPass->text();
        QString new1 = newPass->text();
        QString new2 = confirmPass->text();

        if (old.isEmpty() || new1.isEmpty() || new2.isEmpty()) {
            QMessageBox::warning(&dlg, "Ошибка", "Заполните все поля");
            return;
        }

        if (new1 != new2) {
            QMessageBox::warning(&dlg, "Ошибка", "Пароли не совпадают");
            return;
        }

        if (m_db->changePassword(currentUserId, old.toStdString(), new1.toStdString())) {
            QMessageBox::information(&dlg, "Успех", "Пароль успешно изменён");
            dlg.accept();
        } else {
            QMessageBox::warning(&dlg, "Ошибка", "Неверный старый пароль");
        }
    });

    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    dlg.exec();
}

void MainWindow::onLogout() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Выход",
                                  "Вы действительно хотите выйти из приложения?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        close();
    }
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "О программе",
                       "Система учёта сертификатов\n"
                       "Версия: 1.0\n"
                       "Автор: Suzuyka\n\n"
                       "Программа предназначена для учёта и контроля сроков действия сертификатов.");
}

void MainWindow::onImportCertificates()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Выберите CSV с сертификатами",
        QString(),
        "CSV файлы (*.csv);;Все файлы (*.*)");
    if (fileName.isEmpty())
        return;

    auto oldCerts = m_db->getAllCertificates(currentUserId);

    if (!oldCerts.empty())
    {
        QDir dir;
        if (!dir.exists("сертификаты"))
            dir.mkpath("сертификаты");

        QString timestamp = QDateTime::currentDateTime()
                                .toString("yyyyMMdd_HHmmss");
        QString backupName =
            QString("сертификаты/user_%1_сертификаты_ИБ_%2.csv")
                .arg(currentUserId)
                .arg(timestamp);

        QFile backupFile(backupName);
        if (backupFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&backupFile);
            out.setEncoding(QStringConverter::Utf8);
            out << "ID;Название;Тип;Дата;Описание\n";

            for (const auto &c : oldCerts)
            {
                out << c.id << ';'
                    << QString::fromStdString(c.name) << ';'
                    << QString::fromStdString(c.type) << ';'
                    << QString::fromStdString(c.date) << ';'
                    << QString::fromStdString(c.description) << '\n';
            }

            backupFile.close();
        }
        else
        {
            QMessageBox::warning(this, "Ошибка",
                                 "Не удалось создать файл резервной копии.");
            return;
        }
    }

    if (!m_db->clearCertificatesForUser(currentUserId))
    {
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось очистить старые данные пользователя.");
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось открыть выбранный файл.");
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    if (!in.atEnd())
        in.readLine();

    int imported = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        QStringList parts = line.split(';');
        if (parts.size() < 4)
            continue;

        Certificate c;
        c.id = -1;
        c.name = parts.value(1).trimmed().toStdString();
        c.type = parts.value(2).trimmed().toStdString();
        c.date = parts.value(3).trimmed().toStdString();
        c.description = parts.value(4).trimmed().toStdString();

        m_db->addCertificate(c, currentUserId);
        ++imported;
    }

    file.close();

    loadTable(false);

    QMessageBox::information(
        this,
        "Импорт завершён",
        QString("Импортировано сертификатов: %1.\n"
                "Резервная копия старых данных сохранена (если они были).")
            .arg(imported));
}
