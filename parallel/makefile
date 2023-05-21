all: clean parallel

parallel : readImg.cpp
	g++ -std=c++11 -pthread -w readImg.cpp -o ImageFilters.out

clean:
	rm -f ImageFilters.out
