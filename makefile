############################# Makefile ##########################
all: iamroot
iamroot: iamroot.o startup.o UI.o
	gcc -o iamroot iamroot.o startup.o UI.o
#
startup.o: startup.c
	gcc -o startup.o -c startup.c -W -Wall -ansi -pedantic
UI.o: UI.c
	gcc -o UI.o -c UI.c -W -Wall -ansi -pedantic
iamroot.o: iamroot.c UI.h startup.h
	gcc -o iamroot.o -c iamroot.c -W -Wall -ansi -pedantic

clean:
	rm -rf *.o
mrproper: clean
	rm -rf iamroot
