gcov server.cpp;
lcov -d . -t 'cover' -o 'cover.info' -b . -c;
#生成图形报告
genhtml -o result cover.info;
rm server.gcno cov server.gcda cover.info;