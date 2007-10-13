
CC = g++
OPT = -O3 -DNDEBUG -fno-rtti

.SUFFIXES: .cpp .o 

SRC = $(shell ls ../src/xtal/xtal_*.cpp)
OBJ = $(patsubst ../src/xtal/%.cpp,%.o,$(SRC))

%.o: ../src/xtal/%.cpp
	$(CC) -c $< $(OPT)
	
all: libxtal ix xtal

depend:
	g++ -MM -MG $(SRC) >makefile.depend

libxtal: $(OBJ)
	ar r libxtal.a $(OBJ)
	ranlib libxtal.a

xtal: libxtal ../src/xtal/main.cpp
	$(CC) -o ../bin/xtal.exe ../src/xtal/main.cpp libxtal.a -lstdc++ $(OPT)
	strip --strip-all ../bin/xtal.exe

ix: libxtal ../src/xtal/ixmain.cpp
	$(CC) -o ../bin/ix.exe ../src/xtal/ixmain.cpp libxtal.a -lstdc++ $(OPT)
	strip --strip-all ../bin/ix.exe

.PHONY : clean

clean:
	rm *.o *.a ../bin/xtal.exe ../bin/ix.exe

prof: libxtal ../src/xtal/main.cpp
	$(CC) -o ../bin/prof.exe ../src/xtal/main.cpp libxtal.a -lstdc++ $(OPT)
	../bin/prof.exe ../bench/array_for.xtal
	gprof ../bin/prof.exe gmon.out -p > prof.txt

run: xtal
	../bin/xtal.exe ../test.xtal

-include makefile.depend

