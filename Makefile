# Makefileを書いたことの無いゆとり世代なのでちょっと適当です。
# とりあえず、ix.exeとxtal.exeが生成されます。
# cygwinでコンパイルできたので、多分Linuxでも出来るんじゃないかと思います。
#

CC = g++

all: ix xtal

ix: xtal/ixmain.cpp
	$(CC) -o ix xtal/ixmain.cpp -O2 -lstdc++

xtal: xtal/main.cpp
	$(CC) -o xtal xtal/main.cpp -O2 -lstdc++
