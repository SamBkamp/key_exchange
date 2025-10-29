FLAGS := -pthread -Wall -Wextra -ggdb


server:server.c
	gcc $^ ${FLAGS} -o $@

host:host.c
	gcc $^ ${FLAGS} -o $@

client:client.c
	gcc $^ ${FLAGS} -o $@

all:host server client
	echo "Making all"
