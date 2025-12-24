#include "database.h"
#include <iostream>

#include <QInputDialog>
#include <QMessageBox>

using namespace std;

Database::Database(const string &filename)
    : db(nullptr),
      sqlRepo("queries.sql") //файл с запросами
{
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        cerr << "Не удалось открыть БД: " << sqlite3_errmsg(db) << endl;
        db = nullptr;
    } else {
        createTables();
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Database::isOpen() const {
    return db != nullptr;
}

void Database::createTables() {
    if (!db) return;

    char *err = nullptr;

    try {
        const std::string &usersSql = sqlRepo.get("CREATE_USERS");
        if (sqlite3_exec(db, usersSql.c_str(), nullptr, nullptr, &err) != SQLITE_OK && err) {
            cerr << "Ошибка создания таблицы users: " << err << endl;
            sqlite3_free(err);
        }

        const std::string &certSql = sqlRepo.get("CREATE_CERTS");
        if (sqlite3_exec(db, certSql.c_str(), nullptr, nullptr, &err) != SQLITE_OK && err) {
            cerr << "Ошибка создания таблицы certificates: " << err << endl;
            sqlite3_free(err);
        }
    } catch (const std::exception &ex) {
        cerr << "SQL repo error: " << ex.what() << endl;
    }
}

bool Database::createUser(const string &username, const string &password) {
    if (!db) return false;

    const std::string &sql = sqlRepo.get("INSERT_USER");
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::checkUser(const string &username, const string &password) {
    if (!db) return false;

    const std::string &sql = sqlRepo.get("CHECK_USER");
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::addCertificate(const Certificate &c, int userId) {
    if (!db) return false;

    const std::string &sql = sqlRepo.get("INSERT_CERT");
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, c.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, c.type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, c.date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, c.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, userId);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

void Database::updateCertificate(const Certificate &c) {
    if (!db) return;

    const std::string &sql = sqlRepo.get("UPDATE_CERT");
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return;

    sqlite3_bind_text(stmt, 1, c.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, c.type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, c.date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, c.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, c.id);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void Database::deleteCertificate(int id) {
    if (!db) return;

    const std::string &sql = sqlRepo.get("DELETE_CERT");
    sqlite3_stmt *stmt = nullptr;

    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

vector<Certificate> Database::getAllCertificates(int userId) {
    vector<Certificate> res;
    if (!db) return res;

    const std::string &sql = sqlRepo.get("SELECT_CERTS_BY_USER");
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return res;

    sqlite3_bind_int(stmt, 1, userId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Certificate c;
        c.id   = sqlite3_column_int(stmt, 0);
        c.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        c.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        c.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        c.description =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        c.userId = sqlite3_column_int(stmt, 5);
        res.push_back(c);
    }

    sqlite3_finalize(stmt);
    return res;
}

int Database::getUserId(const std::string &username, const std::string &password) {
    if (!db) return -1;

    const std::string &sql = sqlRepo.get("GET_USER_ID");
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

    int userId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        userId = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return userId;
}

bool Database::changePassword(int userId,
                              const std::string &oldPassword,
                              const std::string &newPassword)
{
    if (!db) return false;

    // 1. Проверяем старый пароль
    const std::string &sqlCheck = sqlRepo.get("CHECK_OLD_PASSWORD");
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sqlCheck.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, oldPassword.c_str(), -1, SQLITE_TRANSIENT);

    bool valid = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    if (!valid) return false;

    // 2. Обновляем пароль
    const std::string &sqlUpdate = sqlRepo.get("UPDATE_PASSWORD");

    if (sqlite3_prepare_v2(db, sqlUpdate.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, newPassword.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, userId);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool Database::clearCertificatesForUser(int userId)
{
    if (!db) return false;

    const std::string &sql = sqlRepo.get("CLEAR_CERTS_BY_USER");
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, userId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::string Database::getDisplayName(int userId)
{
    const std::string &sql = sqlRepo.get("GET_DISPLAY_NAME");

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return {};

    sqlite3_bind_int(stmt, 1, userId);

    std::string result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        if (text)
            result = reinterpret_cast<const char*>(text);
    }

    sqlite3_finalize(stmt);
    return result;
}

bool Database::setDisplayName(int userId, const std::string &name)
{
    const std::string &sql = sqlRepo.get("UPDATE_DISPLAY_NAME");

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, userId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

