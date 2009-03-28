
CC = g++
OPT = -O2 -DNDEBUG -fno-rtti -fno-exceptions -Wall -DXTAL_USE_PTHREAD_TLS

.SUFFIXES: .cpp .o 

SRCDIR = ../src/xtal
BINDIR = ../bin
LIBDIR = ../lib

SRC = $(shell ls $(SRCDIR)/xtal_*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,%.o,$(SRC))

%.o: $(SRCDIR)/%.cpp
	$(CC) -o $@ -c $< $(OPT)

all: libxtal ix xtal

depend:
	g++ -MM -MG $(SRC) >makefile.depend

libxtal: $(OBJ)
	ar r $(LIBDIR)/libxtal.a $(OBJ)
	ranlib $(LIBDIR)/libxtal.a

xtal: libxtal xtal.cpp
	$(CC) -o $(BINDIR)/xtal.exe ./xtal.cpp $(LIBDIR)/libxtal.a -lstdc++ $(OPT) -lpthread 
	strip --strip-all $(BINDIR)/xtal.exe

ix: libxtal ix.cpp
	$(CC) -o $(BINDIR)/ix.exe ./ix.cpp $(LIBDIR)/libxtal.a -lstdc++ $(OPT) -lpthread 
	strip --strip-all $(BINDIR)/ix.exe

test: libxtal test.cpp
	$(CC) -o $(BINDIR)/test.exe ./test.cpp $(LIBDIR)/libxtal.a -lstdc++ $(OPT) -lpthread 
	strip --strip-all $(BINDIR)/test.exe
	$(BINDIR)/test.exe 
	
.PHONY: clean

clean:
	rm *.o *.a $(BINDIR)/xtal.exe $(BINDIR)/ix.exe $(BINDIR)/test.exe $(BINDIR)/prof.exe $(BINDIR)/all_src.exe

prof: override OPT += -pg
prof: libxtal $(SRCDIR)/main.cpp
	$(CC) -o $(BINDIR)/prof.exe ./xtal.cpp $(LIBDIR)/libxtal.a -lstdc++ $(OPT) -lpthread 
	$(BINDIR)/prof.exe ../bench/ao.xtal
	gprof $(BINDIR)/prof.exe gmon.out -p > prof.txt

all_src: 
	$(CC) -o $(BINDIR)/all_src.exe ../all_src/all_src.cpp -lstdc++ $(OPT)

run: xtal
	$(BINDIR)/xtal.exe 

checkdll:
	$ objdump -p $(BINDIR)/xtal.exe | grep "DLL Name"
	$ objdump -p $(BINDIR)/ix.exe | grep "DLL Name"
	
-include makefile.depend

