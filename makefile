TESTDEP=test.cpp testUtils.hpp server/db.hpp server/Properties.hpp
TESTFLE=test.cpp

SERVDEP=server/server.cpp server/db.hpp server/Properties.hpp server/httplib.h
SERVFLE=server/server.cpp

SERVFLG=-std=c++11 -ljsoncpp -lmysqlclient -lpthread
TESTFLG=-std=c++11  -lgtest -lgtest_main -lpthread -lmysqlclient -ljsoncpp

service:$(SERVDEP)
	g++ -o service $(SERVFLE) $(SERVFLG)

test:$(TESTDEP)
	g++ -o test $(TESTFLE) $(TESTFLG)

.PHONY:clean
clean:
	rm test service
