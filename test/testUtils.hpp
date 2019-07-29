#include <string>
#include <ctime>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>

std::string genStr(int len) {
    std::string str;
    for (int i=0; i<len; ++i) {
        str += char((rand()%94)+32);
    }
    return str;
}

int genTagId() {
    return rand()%50;
}

std::string getTime() {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *t = gmtime(&ts.tv_sec);
    char buf[1024] = {0};
    strftime(buf, 1023, "%F", t);
    return std::string(buf);
}