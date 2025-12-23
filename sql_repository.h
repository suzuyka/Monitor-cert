#pragma once
#include <string>
#include <unordered_map>

class SqlRepository {
public:
    explicit SqlRepository(const std::string &path);

    // выбрасывает исключение, если такого имени нет
    const std::string &get(const std::string &name) const;

private:
    std::unordered_map<std::string, std::string> queries;
};
