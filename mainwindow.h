#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "database.h"
#include "profilewidget.h"

#include <QMainWindow>
#include <QLabel>
#include <vector>

class QTableWidget;
class DataFormWidget;
class QPushButton;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(Database *db,
                    int userId,
                    const QString &username,
                    QWidget *parent = nullptr);


private:
    enum class ExportMode { All, Expired, Soon };

    void setupUi();
    void loadTable(bool withHighlight);

    void onAdd();
    void onEdit();
    void onDelete();
    void onSearch();
    void onSaveForm(const Certificate &c);
    void onCancelForm();

    void exportCertificates(ExportMode mode);
    void onImportCertificates();
    void updateStatsLabel();

    void showAbout();
    void onChangePassword();
    void onLogout();

    // ===== поля =====
    Database *m_db;
    int currentUserId;
    QString currentUsername;

    QTableWidget *table;
    DataFormWidget *formWidget;
    QLabel *statsLabel = nullptr;

    ProfileWidget *profileWidget = nullptr;

    QWidget *settingsWidget;
    QLabel *settingsLabel;

    bool isDarkTheme = false;

    // кеш сертификатов текущего пользователя
    std::vector<Certificate> currentCerts;
};

#endif // MAINWINDOW_H
