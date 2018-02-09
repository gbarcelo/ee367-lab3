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

#define MAXDATASIZE 1024 // max number of bytes we can get at once

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
	char obuf[MAXDATASIZE], arg[128];
	int osize;
	int breakflag;
	char ovr[16];
	int isQuit = 0;

	while (isQuit==0) {
		int isDl = 0;
		printf("Command (type `h` for help): ");
		// printf(">> ");
    scanf("%s", obuf);

		breakflag = 0;
		// if (obuf[0]=='q') {break;} // Expand this into switch
		switch(obuf[0]){
			case 'q': {
				isQuit = 1;
				breakflag = 1;
				break;
			}

			case 'd':		// download (Send?)
				isDl = 1;
				FILE *fp;
				char temp[32] = "client/";

			case 'p':	{	// "find and cat ./server/<filename>" case
				scanf("%s", arg);
				char *fname, *vbuf;
				long fsize;
				fname = strdup(arg);
				// strcpy(f,arg);
				strcat(obuf,arg);

				/****Start**********Download Only******************/
				if (isDl){
					fp = fopen(strcat(temp,fname),"rb");
					if (fp) {
						printf("There is currently a file of the same name. Overwrite it? (y/n): ");
						scanf("%s", ovr);
						ovr[1] = 0;
						if (ovr[0] == 'n' || ovr[0] == 'N') {
							printf("Action cancelled.\n\n");
							// breakflag = 1;
							break;
						} else if (ovr[0] != 'y' && ovr[0] != 'Y') {
							printf("Invalid selection, aborting.\n\n");
							break;
						}
					}
				}
				/****End************Download Only******************/

				printf("Checking server . . .\n");
				osize = send(sockfd, obuf ,sizeof(obuf), 0);
				if (osize < 0) {perror("send failed"); return 1;}
				obuf[osize] = 0;

				//Receive a reply from the server
				if( recv(sockfd , buf , MAXDATASIZE , 0) < 0) {
					puts("recv failed");
					breakflag = 1;
					break;
				}

				// in: Recieve a fsize IF NULL, FILE DOES NOT EXIST; LOOPBACK HERE
				if (buf[0] == 0) {printf("File `%s` not found\n\n", fname); break;}
				else {
					fsize = strtol(buf, NULL, 10);
				}
				// puts("in success"); // --debug

				// out: Confirm fsize
				sprintf(obuf, "%ld", fsize);
				osize = send(sockfd, obuf ,MAXDATASIZE, 0);
				if (osize < 0) {perror("send failed"); return 1;}
				obuf[osize] = 0;
				// puts("out success"); // --debug
				// in: if NULL GOTO: Recieve a fsize(loop), else MALLOC and RECV buffer
				if( recv(sockfd , buf , MAXDATASIZE , 0) < 0) {
					puts("recv failed");
					breakflag = 1;
					break;
				}
				if (buf[0] != 0) {
					vbuf = calloc(fsize,sizeof(char*));
					if (vbuf) {
						if( (fsize = recv(sockfd , vbuf , fsize , 0)) < 0) {
							puts("recv failed");
							breakflag = 1;
							break;
							vbuf[fsize] = 0;
						}
						if (isDl == 0) {
							int printed;
							if ((printed = printf("%s\n", vbuf)-1) != fsize) { printf("print failed");}
							// puts("end of vbuf"); // --debug
						} else if (isDl == 1) {
							/****Start**********Download Only******************/
							fp = fopen(temp, "wb");
							fwrite(vbuf, 1, fsize, fp);
							fclose(fp);
							printf("File write complete\n\n");
							/****End************Download Only******************/
						}
						memset(vbuf,0,fsize-1);
						free(vbuf);
					}
				} else {printf("Error: protocal failure\n");}
				/////// end of experiment implementtion; everything //////////////
				/////// below should be commented/deleted ////////////////////////
				free(fname);
				break;
			}


			case 'c': {		//
				scanf("%s", arg);
				char *f;
				f = strdup(arg);
				// strcpy(f,arg);
				strcat(obuf,arg);
				printf("Checking server . . .");
				osize = send(sockfd, obuf ,sizeof(obuf), 0);
				obuf[osize] = 0;

				if (osize < 0) {
					puts("Send failed");
					return 1;
				}

				//Receive a reply from the server
				if( recv(sockfd , buf , MAXDATASIZE , 0) < 0) {
					puts("recv failed");
					breakflag = 1;
				}
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
				memset(buf, 0, MAXDATASIZE-1);
				if( recv(sockfd , buf , MAXDATASIZE , 0) < 0) {
		      puts("recv failed");
					breakflag = 1;
				}
				puts("List:");
				printf("%s",buf);
				printf("\n");
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
