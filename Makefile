# Makefileを書いたことの無いゆとり世代なのでちょっと適当です。
# とりあえず、ix.exeとxtal.exeが生成されます。
# cygwinでコンパイルできたので、多分Linuxでも出来るんじゃないかと思います。
#

CC = gcc

all: ix xtal

ix: xtal/ixmain.cpp
	$(CC) xtal/ixmain.cpp -o ix -O2 -finline-functions -lstdc++

xtal: xtal/main.cpp
	$(CC) xtal/main.cpp -o xtal -O2 -lstdc++ -pg
