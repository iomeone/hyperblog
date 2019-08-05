#include <string>
#include "md5.hpp"


extern std::map<std::string, std::string> userlist;
// 操作是否是登录用户执行的
//验证cookie
//Cookie: name=7cfe1961a95a2f60553b54af9d238b4f
bool isLogin(const std::string &cookie) {
    std::string cook;
    auto pos = cookie.find("name=");
    cook = cookie.substr(pos+5, 32);
    return userlist.count(cook) > 0;
}


//验证发来的密码MD5是否和数据库中密码计算的md5相同
//使用md5算法验证
bool auth(const std::string &passwd, const std::string &m) {
    return MD5SUM(passwd) == m;
}

std::string CalCookie(const std::string &user, const std::string &passwd, std::string &cookie) {
    std::string val = MD5SUM(user + passwd);
    cookie = val;
    std::string ret = "name=";
    ret.append(val);
    ret.append(";max-age=14400");
    return ret;
}