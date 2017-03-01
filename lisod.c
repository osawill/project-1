/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

/******************************************************************************
* echo_client.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo client.  The  *
*              client connects to an arbitrary <host,port> and sends input    *
*              from stdin.                                                    *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "parse.h"
#include <time.h>

#define PORT "9034"   // port we're listening on
#define ROOT_DIR "www/"
#define BUF_SIZE 1024

// Get file extension
const char *get_filename_ext(const char *filename) {
	const char *dot = strrchr(filename, '.');
	if(!dot || dot == filename) return "";
	return dot + 1;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void head_request(Request * request, char * e_buf) {

	char resp[10000];
//	char * filePath = (char*)malloc(strlen(request->http_uri) + strlen(ROOT_DIR) + 1);
//	strcpy(filePath, ROOT_DIR);
//	strcat(filePath, request->http_uri);
	// char * e_buf = (char*) malloc(60);
	if( access( request->http_uri, F_OK ) != -1 ) {
		// file exists
		printf("FILE EXISTS");

		// Server Version
		strcpy(resp,"HTTP/1.1 200 OK\n");

		// Server type
		strcat(resp, "Server: lisod/1.0\n");

		// Connection
		strcat(resp, "Connection: keep-alive\r\n");

		// Get file length
		FILE * fp = fopen(request->http_uri, "rb");
		int prev=ftell(fp);
		fseek(fp, 0L, SEEK_END);
		int sz=ftell(fp);
		fseek(fp,prev,SEEK_SET);
		char length[200];
		sprintf(length, "%d", sz);
		strcat(resp, "Content-length: ");
		strcat(resp, length);
		strcat(resp,"\n");

		// Content type
		char buff_CT[1000];
		const char * extName = get_filename_ext(request->http_uri);

		if (strcmp(extName, "html") == 0) {
			strcpy(buff_CT, "text/html");
		}
		else if (strcmp(extName, "css") == 0) {
			strcpy(buff_CT, "text/css");
		}
		else if (strcmp(extName, "png") == 0) {
			strcpy(buff_CT, "image/png");
		}
		else if (strcmp(extName, "jpeg") == 0) {
			strcpy(buff_CT, "image/jpeg");
		}
		else if (strcmp(extName, "gif") == 0) {
			strcpy(buff_CT, "image/gif");
		}
		else if (strcmp(extName, "txt") == 0) {
			strcpy(buff_CT, "text/plain");
		}
		else { // default
			strcpy(buff_CT, "application/octet-stream");
		}

		strcat(resp, "Content-Type: ");
		strcat(resp, buff_CT);
		strcat(resp,"\n");

		// Get Date
		char buf_t[500];
		time_t now = time(0);
		struct tm tm = *gmtime(&now);
		strftime(buf_t, sizeof buf_t, "%a, %d %b %Y %H:%M:%S %Z", &tm);
		strcat(resp, "Date: ");
		strcat(resp, buf_t);
		strcat(resp, "\n");

		// Get Last date modified of file
		struct stat attr;
		stat(request->http_uri, &attr);
		//char buf_LM[500];
		// NEED TO FORMAT THIS STRING LIKE ABOVE ONE
		// sprintf(buf_LM, "%a, %d %b %Y %H:%M:%S %Z", ctime(&attr.st_mtime));
		strcat(resp, "Last-Modified: ");
		strcat(resp, ctime(&attr.st_mtime));
		strcat(resp, "\n");

	} else { // File doesn't exist: Respond with an error
		strcpy(resp, ""); // ERROR 404
	}

	strcat(resp, "\r\n");
	strcpy(e_buf, resp);
	//Free memory
}

void get_request(Request * request, char * e_buf, int sock) {

	//Send HEAD request internally
	/*head_request(request, e_buf);

	if (send(sock, e_buf, strlen(e_buf), 0) == -1) {
		perror("Sending HEAD response: ");
		return;
	} else {
		printf("%s", e_buf);
	}*/

	char * file_name = (char *)malloc(strlen(request->http_uri) + 5); //Adding room for root directory
	strcpy(file_name, ROOT_DIR);
	strcat(file_name, request->http_uri);

	int fl;
	if ( (fl = open(file_name, O_RDONLY))!=-1 )    //FILE FOUND
	{
		int bytes_read;
		char buffer[BUF_SIZE];
		//send(sock, "HTTP/1.0 200 OK\n\n", 17, 0);
		while ( (bytes_read=read(fl, buffer, BUF_SIZE))>0 )
			write (sock, buffer, bytes_read);
	}
	else    write(sock, "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND


}

void post_request(Request * request, char * e_buf) {

	char respPOST[10000];

	// Server Version
	strcpy(respPOST,"HTTP/1.1 200 OK\n");
	printf("%s", respPOST);

	strcat(respPOST, "\r\n");
	strcpy(e_buf, respPOST);
}

int main(void)
{
	fd_set master;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	int fdmax;        // maximum file descriptor number

	int listener;     // listening socket descriptor
	int newfd;        // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;

	char buf[8192];    // buffer for client data
	int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

	int yes=1;        // for setsockopt() SO_REUSEADDR, below
	int i, rv;

	struct addrinfo hints, *ai, *p;

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	for(p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			continue;
		}

		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

	// listen
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(3);
	}

	// add the listener to the master set
	FD_SET(listener, &master);

	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	// main loop
	for(;;) {
		read_fds = master; // copy it
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		// run through the existing connections looking for data to read
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // we got one!!
				if (i == listener) {
					// handle new connections
					addrlen = sizeof remoteaddr;
					newfd = accept(listener,
						(struct sockaddr *)&remoteaddr,
						&addrlen);

					if (newfd == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on "
							"socket %d\n",
							inet_ntop(remoteaddr.ss_family,
								get_in_addr((struct sockaddr*)&remoteaddr),
								remoteIP, INET6_ADDRSTRLEN),
							newfd);
					}
				} else {
					// handle data from a client
					if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
						// got error or connection closed by client
						if (nbytes == 0) {
							// connection closed
							printf("selectserver: socket %d hung up\n", i);
						} else {
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // remove from master set
					} else {

						// Parse the request
						Request * request = parse(buf, sizeof(buf), i);

						char * send_buf = malloc(10000);



						// HEAD request
						if ((strcmp(request->http_method, "HEAD") == 0) || (strcmp(request->http_method, "GET") == 0)) {
						// printf("Head request");
						head_request(request, send_buf);
						printf("%s", send_buf);
						}

						// POST request
						// CHARLIE: DETERMINE IF GET/HEAD before parsing. Can't parse the POST
						// BEFORE POST.
						else if (strcmp(request->http_method, "POST") == 0) {
						// printf("Post request");
						post_request(request, send_buf);
						}


						if (send(i, send_buf, strlen(send_buf), 0) == -1) {
							perror("send");
						} else {
						//printf("%s", buf);
							//	 GET request
							if (strcmp(request->http_method, "GET") == 0) {
							printf("GET request");
							get_request(request, send_buf, i);
							printf("%s", send_buf);
							}
						}
						free(send_buf);
					}
				} // END handle data from client
			} // END got new incoming connection
		} // END looping through file descriptors
	} // END for(;;)--and you thought it would never end!

	return 0;
}
