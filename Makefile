all: main

main: main.cc Schedule.cc Coroutine.cc
	g++ -std=c++11 -g -Wall -o $@ $^

clean:
	-rm main
