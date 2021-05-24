//
// Created by Jacopo Gasparetto on 21/05/21.
//

#ifndef FORTRESS_ARGPARSE_H
#define FORTRESS_ARGPARSE_H

#include <iostream>
#include <cassert>
#include <string>
#include <memory>
#include <map>


class ArgumentParser {
private:
    std::map<std::string, std::any> m_arguments;
    int m_argc;
    char **m_argv{};

public:

    ArgumentParser(int argc, char *argv[]) : m_argc(argc), m_argv(argv) {}

    template<typename T>
    void addArgument(const std::string &key, T defaultValue = {}) {
        m_arguments[key] = defaultValue;
    }

private:

    void setValue(const std::string &key, bool value) {
        m_arguments[key] = value;
    }

    template<typename T>
    void setValue(const std::string &key, char *value) {
        T v;
        std::stringstream(value) >> v;
        m_arguments[key] = v;
    }

public:
    template<typename T>
    [[nodiscard]] T getValue(const std::string &key) const {
        auto it = m_arguments.find(key);
        if (it == m_arguments.end())
            throw std::domain_error("Element '" + key + "' not found");

        return std::any_cast<T>(it->second);
    }

    [[nodiscard]] std::string_view getValue(const std::string &key) const {
        auto it = m_arguments.find(key);
        if (it == m_arguments.end())
            throw std::domain_error("Element '" + key + "' not found");
        return std::any_cast<std::string_view>(it->second);
    }

    void parseArguments() {

        for (int i = 1; i < m_argc; ++i) {

            if (m_argv[i][1] == '-')
                m_argv[i]++;

            if (m_argv[i][0] == '-') {
                auto currKey = std::string(++m_argv[i]);

                auto it = m_arguments.find(currKey);

                if (it == m_arguments.end())
                    throw std::domain_error("Argument '" + currKey + "' not valid");

                auto value = m_argv[i + 1];

                if (m_argv[i + 1][0] == '-') {
                    setValue(currKey, true);
                    continue;
                }

                switch (*it->second.type().name()) {
                    case 'i':
                        setValue<int>(currKey, value);
                        break;
                    case 'd':
                        setValue<double>(currKey, value);
                        break;
                    default:
                        setValue<std::string>(currKey, value);
                }

            }
        }
    }

};

#endif //FORTRESS_ARGPARSE_H
