all: client/a.out server/a.out

server/a.out: server/server.c
	gcc server/server.c -o server/a.out

run-server: server/a.out
	./server/a.out

server: run-server

client/a.out: client/client.c
	gcc client/client.c -o client/a.out

run-client: client/a.out
	./client/a.out

client: run-client

clean:
	rm ./client/a.out
	rm ./server/a.out
