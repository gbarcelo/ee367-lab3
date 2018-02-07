/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3601"  // the port users will be connecting to (OLD:3490)

#define BACKLOG 10	 // how many pending connections queue will hold

#define LOCALPATH "/bin/"
#define WILIKIPATH "/usr/bin/"

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void error(char *s)
{
  perror(s);
  exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	char comm_path[32];

	if (argc != 2) {
		argv = NULL;
		strcpy(comm_path, WILIKIPATH);
	} else {
		strcpy(comm_path, LOCALPATH);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	char ibuf[1024];		// Using as main in-buffer
	char *filename;	// inbound filename arguement
	int isize,fsize; 					// Inbound size
	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		// Stage 3 -- After client connectoin accepted, wait for
		// a message from client
		while( (isize = recv(new_fd , ibuf , 1024 , 0)) > 0 ) {
      //Send the message back to client
			printf("%s\n", ibuf);
			switch(ibuf[0]){
				case 'p':	{	// "find and cat ./server/<filename>" case
					filename = &ibuf[1];
					// fsize = recv(new_fd , filename , 128 , 0);
					// puts(filename);
					if (!fork()) { // this is the child process
						close(sockfd); // child doesn't need the listener
						int pid, n, pipefd[2];
						char buf[1023]; // main out buffer
						// char *buf = NULL;
						if (pipe(pipefd) < 0) error("pipe error");
						if (!fork()) {	// Begin Child Process for sending "find" to buf
							close(1);
							close(0);
							close(2);
							close(pipefd[0]);		// Close read-side
							dup2(pipefd[1],1);	// Duplicates file descriptor
							dup2(pipefd[1],0);
							dup2(pipefd[1],2);
							// puts("child reaches execl");
							char temp[] = "server/";
							// filename = strcat(temp,filename);
							// execl(strcat(comm_path,"cat"), "cat", strcat("server/",filename), (char *)NULL);
							execl("/bin/cat", "cat", strcat(temp,filename), (char *)NULL);
							error("cat failed");
						} else {	// Begin Parent Process for reading buf after ls
							// puts("parant begin");
							close(pipefd[1]);
							n = read(pipefd[0], buf, 1024);
							// puts("pipe read");
							// printf("%d",n);
							buf[n] = 0; // 0 = null char
							// printf("%s\n", strcat("server/",filename));
							// if (n==0) {strcat(buf,"")} // Deal with 0 on client side
							// close(pipefd[1]);
						}

						// Stage 1 attempt: end [PARTIAL SUCCESS]
						// Stage 2 attempt: replace hello world with buf
						if (send(new_fd, buf, sizeof(buf), 0) == -1)
							perror("send");
						// End while loop here? Loop until recv = 0?
						close(new_fd);
						exit(0);
					}
				}
					break;

				case 'd':		// download (Send?)
					break;

				case 'c':		// "find ./server/<filename>" case
					// printf("Case c\n");
					// puts(ibuf);
					filename = &ibuf[1];
					// fsize = recv(new_fd , filename , 128 , 0);
					// puts(filename);
					if (!fork()) { // this is the child process
						close(sockfd); // child doesn't need the listener
						int pid, n, pipefd[2];
						char buf[1023]; // main out buffer
						if (pipe(pipefd) < 0) error("pipe error");
						if (!fork()) {	// Begin Child Process for sending "find" to buf
							close(1);
							close(0);
							close(2);
							close(pipefd[0]);		// Close read-side
							dup2(pipefd[1],1);	// Duplicates file descriptor
							dup2(pipefd[1],0);
							dup2(pipefd[1],2);
							// puts("child reaches execl");
							// execl(strcat(comm_path,"find"), "find", strcat("./server/",filename),(char *)NULL);
							execl(WILIKIPATH"find", "find", "./server", "-name", filename, (char *)NULL);
							error("find failed");
						} else {	// Begin Parent Process for reading buf after ls
							// puts("parant begin");
							close(pipefd[1]);
							n = read(pipefd[0], buf, 1024);
							// puts("pipe read");
							// printf("%d",n);
							buf[n] = 0; // 0 = null char
							// printf("%s\n", filename);
							// if (n==0) {strcat(buf,"")} // Deal with 0 on client side
							// close(pipefd[1]);
						}

						// Stage 1 attempt: end [PARTIAL SUCCESS]
						// Stage 2 attempt: replace hello world with buf
						if (send(new_fd, buf, sizeof(buf), 0) == -1)
							perror("send");
						// End while loop here? Loop until recv = 0?
						close(new_fd);
						exit(0);
					}
					break;

				case 'l':		// "ls" case
					//////////////////////////////////////////////////////////////
					if (!fork()) { // this is the child process
						close(sockfd); // child doesn't need the listener
						// Stage 1 attempt: execl with pipe
						int pid, n, pipefd[2];
						char buf[1023]; // main out buffer
						if (pipe(pipefd) < 0) error("pipe error");
						// Insert "if (recv L)" here?
						if (!fork()) {	// Begin Child Process for sending "ls" to buf
							// close(1);

							close(1);
							close(0);
							close(2);
							close(pipefd[0]);		// Close read-side
							dup2(pipefd[1],1);	// Duplicates file descriptor
							dup2(pipefd[1],0);
							dup2(pipefd[1],2);
							execl(strcat(comm_path,"ls"), "ls", (char *)NULL);
							error("ls failed");
						} else {	// Begin Parent Process for reading buf after ls
							close(pipefd[1]);
							n = read(pipefd[0], buf, 1024);
							buf[n] = 0; // 0 = null char
							// close(pipefd[1]);
						}

						// Stage 1 attempt: end [PARTIAL SUCCESS]
						// Stage 2 attempt: replace hello world with buf
						if (send(new_fd, buf, sizeof(buf), 0) == -1)
							perror("send");
						// End while loop here? Loop until recv = 0?
						close(new_fd);
						exit(0);
					}
					/////////////////////////////////////////////////////////////
					break;

				default:		// Echo code to print echo if first letter is not a case
					isize = 4;
					strcpy(ibuf,"Echo");
					ibuf[isize]=0;
					if (send(new_fd, ibuf, sizeof(ibuf), 0) == -1)
						perror("send");
					ibuf[0]=0;
			}


			// Place arguement reading here

    }

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			// Stage 1 attempt: execl with pipe
			int pid, n, pipefd[2];
			char buf[1023]; // main out buffer
			if (pipe(pipefd) < 0) error("pipe error");
			// Insert "if (recv L)" here?
			if (!fork()) {	// Begin Child Process for sending "ls" to buf
				close(pipefd[0]);		// Close read-side
				dup2(pipefd[1],1);	// Duplicates file descriptor
				execl(strcat(comm_path,"ls"), "ls", (char *)NULL);
				error("ls failed");
			} else {	// Begin Parent Process for reading buf after ls
				close(pipefd[1]);
				n = read(pipefd[0], buf, 1024);
				buf[n] = 0; // 0 = null char
				// close(pipefd[1]);
			}

			// Stage 1 attempt: end [PARTIAL SUCCESS]
			// Stage 2 attempt: replace hello world with buf
			if (send(new_fd, buf, sizeof(buf), 0) == -1)
				perror("send");
			// End while loop here? Loop until recv = 0?
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
