LFLAG = -L/usr/lib64/mysql -lmysqlclient -ljsoncpp -lpthread -lboost_system
main:main.cpp db.hpp
	g++ -std=c++11 $^ -o $@ $(LFLAG)
