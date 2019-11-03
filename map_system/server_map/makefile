#定义变量
FLAG = -std=c++11 -L/usr/lib64/mysql -lmysqlclient -ljsoncpp -lpthread 

.PHONY:all 
all:test_db server_map 

test_db:db_test.cc db.hpp
	g++ db_test.cc -o test_db $(FLAG)
server_map:server_map.cc 
	g++ server_map.cc -o server_map $(FLAG)

.PHONY:clean
clean:
	rm server_map 
	
