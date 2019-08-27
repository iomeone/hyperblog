TESTDEP=test/test.cpp test/testUtils.hpp src/server/db.hpp src/server/Properties.hpp
TESTFLE=test/test.cpp

#-L/usr/lib64/mysql
SERVDEP=src/server/server.cpp src/server/db.hpp src/server/Properties.hpp src/server/httplib.h src/server/md5.hpp src/server/authentication.hpp
SERVFLE=src/server/server.cpp

SERVFLG=-std=c++11 -ljsoncpp -lmysqlclient -lpthread  -lcolib -ldl
TESTFLG=-std=c++11  -lgtest -lgtest_main -lpthread -lmysqlclient -ljsoncpp -lcolib -ldl
G=-g

log=-D__LOG__
dbg=-D__DEBUG__

cov=-fprofile-arcs  -ftest-coverage代码覆盖率测试

.PHONY:all
all:servd testd

servd:$(SERVDEP)
	g++ -o servd $(SERVFLE) $(SERVFLG) $(log)

testd:$(TESTDEP)
	g++ -o testd $(TESTFLE) $(TESTFLG)

.PHONY:clean
clean:
	rm testd
	rm servd
