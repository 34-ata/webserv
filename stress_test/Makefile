all: stressTester
	rm -f stress_test.log
	./stressTester GET 200 Makefile 9090 7090 8080

stressTester: main.cpp
	c++ -Wall -Wextra --std=c++2a main.cpp -o stressTester -lcurl
