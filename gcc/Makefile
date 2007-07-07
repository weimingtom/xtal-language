
CC = g++

OBJS = xtal_allocator.o xtal_any.o xtal_array.o xtal_code.o xtal_codebuilder.o xtal_expr.o xtal_frame.o xtal_fun.o xtal_cfun.o xtal_lexer.o xtal_lib.o xtal_map.o xtal_parser.o xtal_string.o xtal_vmachine.o xtal_vmachineimpl.o xtal_cast.o xtal_marshal.o xtal_stream.o xtal_stack.o xtal_thread.o xtal_ch.o 

OPT_FLAGS = -O3 -DNDEBUG -g
#OPT_FLAGS = -O3 -DNDEBUG -pg
#OPT_FLAGS = -O3 -DNDEBUG -pg

all: xtallib ix xtal

xtal_allocator.o: ../xtal/xtal_allocator.cpp
	$(CC) -c ../xtal/xtal_allocator.cpp $(OPT_FLAGS)

xtal_any.o: ../xtal/xtal_any.cpp
	$(CC) -c ../xtal/xtal_any.cpp $(OPT_FLAGS)

xtal_array.o: ../xtal/xtal_array.cpp
	$(CC) -c ../xtal/xtal_array.cpp $(OPT_FLAGS)

xtal_code.o: ../xtal/xtal_code.cpp
	$(CC) -c ../xtal/xtal_code.cpp $(OPT_FLAGS)

xtal_codebuilder.o: ../xtal/xtal_codebuilder.cpp
	$(CC) -c ../xtal/xtal_codebuilder.cpp $(OPT_FLAGS)

xtal_expr.o: ../xtal/xtal_expr.cpp
	$(CC) -c ../xtal/xtal_expr.cpp $(OPT_FLAGS)

xtal_frame.o: ../xtal/xtal_frame.cpp
	$(CC) -c ../xtal/xtal_frame.cpp $(OPT_FLAGS)

xtal_fun.o: ../xtal/xtal_fun.cpp
	$(CC) -c ../xtal/xtal_fun.cpp $(OPT_FLAGS)

xtal_cfun.o: ../xtal/xtal_cfun.cpp
	$(CC) -c ../xtal/xtal_cfun.cpp $(OPT_FLAGS)

xtal_lexer.o: ../xtal/xtal_lexer.cpp
	$(CC) -c ../xtal/xtal_lexer.cpp $(OPT_FLAGS)

xtal_lib.o: ../xtal/xtal_lib.cpp
	$(CC) -c ../xtal/xtal_lib.cpp $(OPT_FLAGS)

xtal_map.o: ../xtal/xtal_map.cpp
	$(CC) -c ../xtal/xtal_map.cpp $(OPT_FLAGS)

xtal_parser.o: ../xtal/xtal_parser.cpp
	$(CC) -c ../xtal/xtal_parser.cpp $(OPT_FLAGS)

xtal_string.o: ../xtal/xtal_string.cpp
	$(CC) -c ../xtal/xtal_string.cpp $(OPT_FLAGS)

xtal_vmachine.o: ../xtal/xtal_vmachine.cpp
	$(CC) -c ../xtal/xtal_vmachine.cpp $(OPT_FLAGS)

xtal_vmachineimpl.o: ../xtal/xtal_vmachineimpl.cpp
	$(CC) -c ../xtal/xtal_vmachineimpl.cpp $(OPT_FLAGS)

xtal_cast.o: ../xtal/xtal_cast.cpp
	$(CC) -c ../xtal/xtal_cast.cpp $(OPT_FLAGS)

xtal_marshal.o: ../xtal/xtal_marshal.cpp
	$(CC) -c ../xtal/xtal_marshal.cpp $(OPT_FLAGS)

xtal_stream.o: ../xtal/xtal_stream.cpp
	$(CC) -c ../xtal/xtal_stream.cpp $(OPT_FLAGS)

xtal_stack.o: ../xtal/xtal_stack.cpp
	$(CC) -c ../xtal/xtal_stack.cpp $(OPT_FLAGS)

xtal_thread.o: ../xtal/xtal_thread.cpp
	$(CC) -c ../xtal/xtal_thread.cpp $(OPT_FLAGS)

xtal_ch.o: ../xtal/xtal_ch.cpp
	$(CC) -c ../xtal/xtal_ch.cpp $(OPT_FLAGS)

		
xtallib: $(OBJS)
	ar r xtallib.a $(OBJS)
	ranlib xtallib.a

xtal: xtallib ../xtal/main.cpp
	$(CC) -o ../bin/xtal.exe ../xtal/main.cpp xtallib.a -lstdc++ $(OPT_FLAGS)
	#strip --strip-all ../bin/xtal.exe

ix: xtallib ../xtal/ixmain.cpp
	$(CC) -o ../bin/ixmain.exe ../xtal/ixmain.cpp xtallib.a -lstdc++ $(OPT_FLAGS)
	#strip --strip-all ../bin/ixmain.exe
	
clean:
	rm *.o *.a ../bin/xtal.exe ../bin/ix.exe

prof: xtal
	../bin/xtal.exe ../bench/inst.xtal
	gprof ../bin/xtal.exe gmon.out -p > prof.txt

run: xtal
	../xtal.exe ../test.xtal
	
