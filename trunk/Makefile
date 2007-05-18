# Makefileを書いたことの無いゆとり世代なのでちょっと適当です。
# とりあえず、ix.exeとxtal.exeが生成されます。
# cygwinでコンパイルできたので、多分Linuxでも出来るんじゃないかと思います。
#

CC = gcc

n: ix.exe xtal.exe

ix.exe: xtal/ixmain.cpp
	$(CC) -o ix.exe xtal/ixmain.cpp -O2 -lstdc++

xtal.exe: xtal/main.cpp
	$(CC) -o xtal.exe xtal/main.cpp -O2 -lstdc++
