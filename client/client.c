/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3601" // the port client will be connecting to (OLD:3490)

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	// Stage 3 -- After server accepts, send and recieve until Q
	char obuf[1024], arg[128];
	int osize;
	int breakflag;

	while (obuf[0]!='q') {
		printf("Command (type `h` for help): ");
		// printf(">> ");
    scanf("%s", obuf);

		breakflag = 0;
		// if (obuf[0]=='q') {break;} // Expand this into switch
		switch(obuf[0]){
			case 'q': {
				breakflag = 1;
				break;
			}

			case 'p':	{	// "find and cat ./server/<filename>" case
				break;
			}

			case 'd':	{	// download (Send?)
				break;
			}

			case 'c': {		//
				scanf("%s", arg);
				char *f;
				printf("%s\n", );("1\n");
				f = strdup(arg);
				printf("2\n");
				// strcpy(f,arg);
				strcat(obuf,arg);
				printf("3\n");
				printf("Checking server . . .");
				osize = send(sockfd, obuf ,sizeof(obuf), 0);
				printf("4\n");
				obuf[osize] = 0;
				printf("5\n");

				if (osize < 0) {
					puts("Send failed");
					return 1;
				}

				printf("6\n");
				//Receive a reply from the server
				if( recv(sockfd , buf , 1024 , 0) < 0) {
					puts("recv failed");
					breakflag = 1;
				}
				printf("7\n");
				if (buf[0]==0) {printf("\nFile `%s` not found\n", f);}
				else {printf("\nFile `%s` exists\n", f);}
				printf("\n");
				free(f);
				break;
			}

			case 'l':{		// Dropdown (for now)
				osize = send(sockfd, obuf ,sizeof(obuf), 0);
				obuf[osize] = 0;

		    if (osize < 0) {
		      puts("Send failed");
		      return 1;
		    }

		    //Receive a reply from the server
		    if( recv(sockfd , buf , 1024 , 0) < 0) {
		      puts("recv failed");
					breakflag = 1;
				}
				puts("List:");
				puts(buf);
				break;
			}

			case 'h': {
				puts("Commands:");
				puts("l : List");
				puts("c : Check <filename>");
				puts("p : Display <filename>");
				puts("d : Download <filename>");
				puts("q : Quit");
				puts("h : Help");
				puts("");
				break;
			}

			default: {
				puts("Error: Command not found.\n");
				// osize = send(sockfd, obuf ,sizeof(obuf), 0);
				// obuf[osize] = 0;
        //
				// if (osize < 0) {
				// 	puts("Send failed");
				// 	return 1;
				// }
        //
				// //Receive a reply from the server
				// if( recv(sockfd , buf , 1024 , 0) < 0) {
				// 	puts("recv failed");
				// 	breakflag = 1;
				// }
			}
		}
		if (breakflag) {break;}
		//Send some data

    //send/recv used to be here

    // puts("Server reply :\n");
    // puts(buf);
	}

	// Commented out for testing Stage 3 Echo code
	// if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	//     perror("recv");
	//     exit(1);
	// }

	// buf[numbytes] = '\0';

	// printf("client: received '%s'\n",buf);

	close(sockfd);

	return 0;
}
