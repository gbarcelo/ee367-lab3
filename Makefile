all: client/a.out server/a.out clean

server/a.out: server/server.c
	-gcc server/server.c -o server/a.out

run-server: server/a.out
	./server/a.out

run-server-local: server/a.out
	./server/a.out loc

server: run-server clean

serverl: run-server-local clean

client/a.out: client/client.c
	-gcc client/client.c -o client/a.out

run-client-wiliki: client/a.out
	./client/a.out wiliki.eng.hawaii.edu

run-client-local: client/a.out
	./client/a.out 127.0.0.1

client: run-client-wiliki clean

local: run-client-local clean

clean:
	-rm ./client/a.out
	-rm ./client/*.txt
	-rm ./server/a.out
