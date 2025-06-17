DSON_DEBUG: DSON.cpp
	mkdir -p build
	g++ -c -o build/DSON.o DSON.cpp -Wall -Wextra -pedantic -ggdb
	ar rcs build/libdson.a build/DSON.o

DSON_RELEASE: DSON.cpp
	mkdir -p build
	g++ -c -o build/DSON.o DSON.cpp -O3
	ar rcs build/libdson.a build/DSON.o

clean:
	rm -f build/*.*

