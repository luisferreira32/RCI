############################# Makefile ##########################
all: iamroot #bash
iamroot: iamroot.o startup.o UI.o GUI.o udp_api.o tcp_api.o root_api.o access_server.o stream_api.o
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
tcp_api.o: tcp_api.c tcp_api.h
	gcc -o tcp_api.o -c tcp_api.c -W -Wall
root_api.o: root_api.c root_api.h udp_api.h
	gcc -o root_api.o -c root_api.c -W -Wall -ansi -pedantic
stream_api.o: stream_api.c stream_api.h tcp_api.h
	gcc -o stream_api.o -c stream_api.c -W -Wall -ansi -pedantic
access_server.o: access_server.c access_server.h udp_api.h
	gcc -o access_server.o -c access_server.c -W -Wall -ansi -pedantic

iamroot.o: iamroot.c UI.h startup.h root_api.h structs.h access_server.h stream_api.h
	gcc -o iamroot.o -c iamroot.c -W -Wall -ansi -pedantic

# running bash script -NOT WORKING
#bash:
#	$(shell alias iamroot="./iamroot")

# clean functions
clean:
	rm -rf *.o
mrproper: clean
	rm -rf iamroot
