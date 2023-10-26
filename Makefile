
main: main.c libfile_utils.a
	gcc -Wall `pkg-config --cflags gtk4` -o main main.c -L. -l:libfile_utils.a -lGL -lGLU `pkg-config --libs gtk4`

libfile_utils.a: file_utils.c
	gcc -Wall -c -o file_utils.o file_utils.c
	ar rcs libfile_utils.a file_utils.o
	rm file_utils.o

debug: main.c file_utils.c
	gcc -Wall `pkg-config --cflags gtk4` -g -o debug main.c -lGL -lGLU `pkg-config --libs gtk4`


