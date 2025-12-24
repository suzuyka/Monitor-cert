#include <QWidget>
#include <functional>

class QLineEdit;
class Database;

class LoginWindow : public QWidget {
public:
    explicit LoginWindow(Database *db, QWidget *parent = nullptr);

    void setOnLoginSuccess(const std::function<void(int, const QString&)> &cb);

private:
    void onLogin();
    void onRegisterLink();

    Database *m_db;
    QLineEdit *userEdit;
    QLineEdit *passEdit;

    std::function<void(int, const QString&)> onLoginSuccess;
};
