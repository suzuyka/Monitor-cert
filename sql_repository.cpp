#include "sql_repository.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
using namespace std;

SqlRepository::SqlRepository(const string &path)
{
    ifstream in(path);
    if (!in.is_open()) {
        throw runtime_error("Cannot open SQL file: " + path);
    }

    string line;
    string currentName;
    ostringstream currentSql;

    auto flushCurrent = [&]() {
        if (!currentName.empty()) {
            string sql = currentSql.str();

            // trim
            auto start = sql.find_first_not_of(" \n\r\t");
            auto end   = sql.find_last_not_of(" \n\r\t");
            if (start != string::npos) {
                sql = sql.substr(start, end - start + 1);
            }

            queries[currentName] = sql;
            currentName.clear();
            currentSql.str("");
            currentSql.clear();
        }
    };

    while (getline(in, line)) {
        if (line.rfind("-- name:", 0) == 0) {
            // новая секция
            flushCurrent();
            currentName = line.substr(8); // после "-- name:"
            auto pos = currentName.find_first_not_of(" \t");
            if (pos != string::npos)
                currentName = currentName.substr(pos);
        } else {
            currentSql << line << '\n';
        }
    }

    flushCurrent();
}

const string &SqlRepository::get(const string &name) const
{
    auto it = queries.find(name);
    if (it == queries.end()) {
        throw runtime_error("SQL not found: " + name);
    }
    return it->second;
}
