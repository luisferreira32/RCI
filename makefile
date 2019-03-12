############################# Makefile ##########################
all: iamroot #bash
iamroot: iamroot.o startup.o UI.o GUI.o udp_api.o root_api.o
	gcc -o iamroot *.o

# compiling one at the time
startup.o: startup.c startup.h
	gcc -o startup.o -c startup.c -W -Wall -ansi -pedantic
GUI.o: GUI.c GUI.h
	gcc -o GUI.o -c GUI.c -W -Wall -ansi -pedantic
UI.o: UI.c UI.h GUI.h
	gcc -o UI.o -c UI.c -W -Wall -ansi -pedantic
udp_api.o: udp_api.c udp_api.h
	gcc -o udp_api.o -c udp_api.c -W -Wall
root_api.o: root_api.c root_api.h udp_api.h
	gcc -o root_api.o -c root_api.c -W -Wall -ansi -pedantic

iamroot.o: iamroot.c UI.h startup.h
	gcc -o iamroot.o -c iamroot.c -W -Wall -ansi -pedantic

# running bash script -NOT WORKING
#bash:
#	$(shell alias iamroot="./iamroot")

# clean functions
clean:
	rm -rf *.o
mrproper: clean
	rm -rf iamroot
