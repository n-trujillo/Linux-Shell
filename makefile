# makefile
start: start.o Request.o
	g++ -g -w -std=c++17 start.o Request.o -o start

start.o: start.cpp
	g++ -g -w -std=c++17 -c start.cpp

Request.o: Request.cpp Request.h
	g++ -g -w -std=c++17 -c Request.cpp

clean:
	rm *.o start