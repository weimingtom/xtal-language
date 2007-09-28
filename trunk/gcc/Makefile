
CC = g++
OPT = -O3 -DNDEBUG -fno-rtti

.SUFFIXES: .cpp .o 

SRC = $(shell ls ../xtal/xtal_*.cpp)
OBJ = $(patsubst ../xtal/%.cpp,%.o,$(SRC))

%.o: ../xtal/%.cpp
	$(CC) -c $< $(OPT)
	
all: libxtal ix xtal

depend:
	g++ -MM -MG $(SRC) >makefile.depend

libxtal: $(OBJ)
	ar r libxtal.a $(OBJ)
	ranlib libxtal.a

xtal: libxtal ../xtal/main.cpp
	$(CC) -o ../bin/xtal.exe ../xtal/main.cpp libxtal.a -lstdc++ $(OPT)
	strip --strip-all ../bin/xtal.exe

ix: libxtal ../xtal/ixmain.cpp
	$(CC) -o ../bin/ix.exe ../xtal/ixmain.cpp libxtal.a -lstdc++ $(OPT)
	strip --strip-all ../bin/ix.exe

.PHONY : clean

clean:
	rm *.o *.a ../bin/xtal.exe ../bin/ix.exe

prof: libxtal ../xtal/main.cpp
	$(CC) -o ../bin/prof.exe ../xtal/main.cpp libxtal.a -lstdc++ $(OPT)
	../bin/prof.exe ../bench/array_for.xtal
	gprof ../bin/prof.exe gmon.out -p > prof.txt

run: xtal
	../bin/xtal.exe ../test.xtal

-include makefile.depend

