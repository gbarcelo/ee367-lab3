all: bin/client367 bin/server

bin/server: src/server.c
	-gcc src/server.c -o bin/server

run-server: bin/server
	./bin/server

run-server-local: bin/server
	./bin/server loc

server: run-server

serverbg: bin/client367 bin/server
	./bin/server &

serverl: run-server-local

bin/client367: src/client.c
	-gcc src/client.c -o bin/client367

run-client-wiliki: bin/client367
	./bin/client367 wiliki.eng.hawaii.edu

run-client-local: bin/client367
	./bin/client367 127.0.0.1

client: run-client-wiliki

local: run-client-local

clean:
	-rm ./bin/client367
	-rm ./client/*.txt
	-rm ./bin/server
