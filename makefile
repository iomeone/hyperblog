TESTDEP=test/test.cpp test/testUtils.hpp src/server/db.hpp src/server/Properties.hpp
TESTFLE=test/test.cpp

SERVDEP=src/server/server.cpp src/server/db.hpp src/server/Properties.hpp src/server/httplib.h
SERVFLE=src/server/server.cpp

SERVFLG=-std=c++11 -ljsoncpp -lmysqlclient -lpthread -D__LOG__
TESTFLG=-std=c++11  -lgtest -lgtest_main -lpthread -lmysqlclient -ljsoncpp

gcov=-fprofile-arcs  -ftest-coverage代码覆盖率测试

.PHONY:all
all:servd testd

servd:$(SERVDEP)
	g++ -o servd $(SERVFLE) $(SERVFLG)

testd:$(TESTDEP)
	g++ -o testd $(TESTFLE) $(TESTFLG)

.PHONY:clean
clean:
	rm testd
	rm servd
