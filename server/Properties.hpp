#pragma once

#include <memory>
#include <string>

#include <unistd.h>
#include <sys/stat.h>

#include <jsoncpp/json/json.h>

class Properties {
public:
    Properties() = delete;

    Properties(const std::string &path){
        int fd = open(path.c_str(), O_RDONLY);
        struct stat st;
        stat(path.c_str(), &st);
        int len = st.st_size;
        std::unique_ptr<char> buf(new char[len]);
        read(fd, buf.get(), len);
        Json::Reader reader;
        reader.parse(buf.get(), buf.get() + len, map);
    }

    ~Properties() = default;

    std::string operator[](const std::string &key) {
        return map[key].asString();
    }

private:
    Json::Value map;
};