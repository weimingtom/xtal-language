
CC = g++
OPT = -O2 -DNDEBUG -fno-rtti -fno-exceptions -mno-cygwin -Wall

.SUFFIXES: .cpp .o 

SRCDIR = ../src/xtal
BINDIR = ../bin

SRC = $(shell ls $(SRCDIR)/xtal_*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,%.o,$(SRC))

%.o: $(SRCDIR)/%.cpp
	$(CC) -o $@ -c $< $(OPT)

all: libxtal ix xtal

depend:
	g++ -MM -MG $(SRC) >makefile.depend

libxtal: $(OBJ)
	ar r $(TEMPDIR)/libxtal.a $(OBJ)
	ranlib $(TEMPDIR)/libxtal.a

xtal: libxtal $(SRCDIR)/main.cpp
	$(CC) -o $(BINDIR)/xtal.exe $(SRCDIR)/main.cpp $(TEMPDIR)/libxtal.a -lstdc++ $(OPT)
	strip --strip-all $(BINDIR)/xtal.exe

ix: libxtal $(SRCDIR)/ixmain.cpp
	$(CC) -o $(BINDIR)/ix.exe $(SRCDIR)/ixmain.cpp $(TEMPDIR)/libxtal.a -lstdc++ $(OPT)
	strip --strip-all $(BINDIR)/ix.exe

testxtal: libxtal $(SRCDIR)/testmain.cpp
	$(CC) -o $(BINDIR)/test.exe $(SRCDIR)/testmain.cpp $(TEMPDIR)/libxtal.a -lstdc++ $(OPT)
	strip --strip-all $(BINDIR)/test.exe
	
.PHONY: clean

clean:
	rm *.o *.a $(BINDIR)/xtal.exe $(BINDIR)/ix.exe $(BINDIR)/test.exe $(BINDIR)/prof.exe $(BINDIR)/all_src.exe

prof: override OPT += -pg
prof: libxtal $(SRCDIR)/main.cpp
	$(CC) -o $(BINDIR)/prof.exe $(SRCDIR)/main.cpp $(TEMPDIR)/libxtal.a -lstdc++ $(OPT)
	$(BINDIR)/prof.exe ../bench/ao.xtal
	gprof $(BINDIR)/prof.exe gmon.out -p > prof.txt

all_src: 
	$(CC) -o $(BINDIR)/all_src.exe ../all_src/all_src.cpp -lstdc++ $(OPT)

run: xtal
	$(BINDIR)/xtal.exe 

test: testxtal
	$(BINDIR)/test.exe

checkdll:
	$ objdump -p $(BINDIR)/xtal.exe | grep "DLL Name"
	$ objdump -p $(BINDIR)/ix.exe | grep "DLL Name"
	
-include makefile.depend

