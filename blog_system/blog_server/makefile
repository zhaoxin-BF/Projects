FLAG=-std=c++11 -L/usr/lib64/mysql -lmysqlclient -ljsoncpp -lpthread

.PHONY:all 
all:db_test server_blog 

db_test:db_test.cc db.hpp
	g++ db_test.cc -o db_test $(FLAG)
server_blog:blog_server.cc 
	g++ blog_server.cc -o server_blog $(FLAG)

.PHONY:clean
clean:
	rm db_test server_blog 
	
