#pragma once

#include <map>
#include <string>
#include <fstream>
#include <iostream>

class Properties {
public:
    Properties() = delete;

    Properties(const std::string &path){
        std::fstream fs(path, std::fstream::in);
        ParseLine(fs);
    }

    Properties(const Properties &rhs) {
        std::map<std::string, std::string> dup(rhs._kv);
        std::swap(this->_kv, dup);
    }

    ~Properties() = default;

    std::string operator[](const std::string &key) {
        return _kv[key];
    }

    bool hasProp(const std::string &key) {
        return _kv.count(key) > 0;
    }

    void test() {
        for (auto p : _kv) {
            std::cout << p.first << " : " << p.second << std::endl;
        }
    }

private:
    void ParseLine(std::fstream &fs) {
        std::string line;
        while (std::getline(fs, line)) {
            auto pos = line.find(':');
            _kv.insert(
                    std::make_pair(
                            line.substr(0, pos),
                            line.substr(pos+1, line.size()-pos)
                            )
                    );
        }
    }

private:
    std::map<std::string, std::string> _kv;
};