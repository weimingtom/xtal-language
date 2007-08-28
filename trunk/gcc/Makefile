
CC = g++

OBJS = xtal_allocator.o xtal_any.o xtal_array.o xtal_code.o xtal_codebuilder.o xtal_expr.o xtal_frame.o xtal_fun.o xtal_cfun.o xtal_lexer.o xtal_lib.o xtal_map.o xtal_parser.o xtal_string.o xtal_vmachine.o xtal_cast.o xtal_stream.o xtal_stack.o xtal_thread.o xtal_ch.o xtal_base.o xtal_serializer.o xtal_debug.o xtal_format.o xtal_math.o xtal_basic_type.o

OPT_FLAGS = -O3 -DNDEBUG -fno-rtti

all: libxtal ix xtal

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

xtal_cast.o: ../xtal/xtal_cast.cpp
	$(CC) -c ../xtal/xtal_cast.cpp $(OPT_FLAGS)

xtal_stream.o: ../xtal/xtal_stream.cpp
	$(CC) -c ../xtal/xtal_stream.cpp $(OPT_FLAGS)

xtal_stack.o: ../xtal/xtal_stack.cpp
	$(CC) -c ../xtal/xtal_stack.cpp $(OPT_FLAGS)

xtal_thread.o: ../xtal/xtal_thread.cpp
	$(CC) -c ../xtal/xtal_thread.cpp $(OPT_FLAGS)

xtal_ch.o: ../xtal/xtal_ch.cpp
	$(CC) -c ../xtal/xtal_ch.cpp $(OPT_FLAGS)

xtal_base.o: ../xtal/xtal_base.cpp
	$(CC) -c ../xtal/xtal_base.cpp $(OPT_FLAGS)

xtal_serializer.o: ../xtal/xtal_serializer.cpp
	$(CC) -c ../xtal/xtal_serializer.cpp $(OPT_FLAGS)

xtal_debug.o: ../xtal/xtal_debug.cpp
	$(CC) -c ../xtal/xtal_debug.cpp $(OPT_FLAGS)

xtal_format.o: ../xtal/xtal_format.cpp
	$(CC) -c ../xtal/xtal_format.cpp $(OPT_FLAGS)

xtal_math.o: ../xtal/xtal_math.cpp
	$(CC) -c ../xtal/xtal_math.cpp $(OPT_FLAGS)

xtal_basic_type.o: ../xtal/xtal_basic_type.cpp
	$(CC) -c ../xtal/xtal_basic_type.cpp $(OPT_FLAGS)

libxtal: $(OBJS)
	ar r libxtal.a $(OBJS)
	ranlib libxtal.a

xtal: libxtal ../xtal/main.cpp
	$(CC) -o ../bin/xtal.exe ../xtal/main.cpp libxtal.a -lstdc++ $(OPT_FLAGS)
	strip --strip-all ../bin/xtal.exe

ix: libxtal ../xtal/ixmain.cpp
	$(CC) -o ../bin/ix.exe ../xtal/ixmain.cpp libxtal.a -lstdc++ $(OPT_FLAGS)
	strip --strip-all ../bin/ix.exe
	
clean:
	rm *.o *.a ../bin/xtal.exe ../bin/ix.exe

prof: libxtal ../xtal/main.cpp
	$(CC) -o ../bin/prof.exe ../xtal/main.cpp libxtal.a -lstdc++ $(OPT_FLAGS)
	../bin/prof.exe ../bench/array_for.xtal
	gprof ../bin/prof.exe gmon.out -p > prof.txt

run: xtal
	../bin/xtal.exe ../test.xtal
	
