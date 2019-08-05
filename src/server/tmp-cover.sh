#覆盖率测试  结合postman
g++ -o cov server.cpp -fprofile-arcs -ftest-coverage -std=c++11 -ljsoncpp -lmysqlclient -lpthread;
#这里执行完后才能执行下面的  正常运行一段时间才能生成
./cov&
wait;
gcov server.cpp;
lcov -d . -t 'cover' -o 'cover.info' -b . -c;
#生成图形报告
genhtml -o result cover.info;
rm server.gcno cov server.gcda cover.info;