#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>
#include "sql_repository.h"

struct Certificate {
    int id;
    std::string name;
    std::string type;
    std::string date;
    std::string description;
    int userId;
};

class Database {
public:
    explicit Database(const std::string &filename);
    ~Database();

    bool isOpen() const;

    bool createUser(const std::string &username, const std::string &password);
    bool checkUser(const std::string &username, const std::string &password);
    bool addCertificate(const Certificate &c, int userId);
    void updateCertificate(const Certificate &c);
    void deleteCertificate(int id);
    std::vector<Certificate> getAllCertificates(int userId);
    int getUserId(const std::string &username, const std::string &password);
    bool changePassword(int userId,
                        const std::string &oldPassword,
                        const std::string &newPassword);
    bool clearCertificatesForUser(int userId);

private:
    sqlite3 *db;
    SqlRepository sqlRepo;

    void createTables();
};
