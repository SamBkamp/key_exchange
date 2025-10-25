FLAGS := -pthread -Wall


server.bin:server.c
	gcc server.c ${FLAGS} -o server.bin
