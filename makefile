insert:main.cpp
	g++ $^ -o $@ -L/usr/lib64/mysql -lmysqlclient -std=c++11