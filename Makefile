all: bin/client367 bin/server clean

bin/server: src/server.c
	-gcc src/server.c -o bin/server

run-server: bin/server
	./bin/server

run-server-local: bin/server
	./bin/server loc

server: run-server clean

serverl: run-server-local clean

bin/client367: src/client.c
	-gcc src/client.c -o bin/client367

run-client-wiliki: bin/client367
	./bin/client367 wiliki.eng.hawaii.edu

run-client-local: bin/client367
	./bin/client367 127.0.0.1

client: run-client-wiliki clean

local: run-client-local clean

clean:
	-rm ./bin/client367
	-rm ./client/*.txt
	-rm ./bin/server
