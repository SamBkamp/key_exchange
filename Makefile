FLAGS := -pthread -Wall -Wextra -ggdb


server.bin:server.c
	gcc server.c ${FLAGS} -o server.bin

host.bin:host.c
	gcc $^ ${FLAGS} -o $@
