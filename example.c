/* C declarations used in actions */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "parse.h"


int head_request(Request * request) {

  if( access( request->http_uri, F_OK ) != -1 ) {
      // file exists
      printf("yep");
  } else {
      // file doesn't exist
      printf("nope");
  }

  return 0;
}

int get_request(Request * request) {
  head_request(request);

  return 0;
}

int post_request(Request * request) {
  head_request(request);

  return 0;
}

int main(int argc, char **argv){
  //Read from the file the sample
  int fd_in = open(argv[1], O_RDONLY);
  int index;
  char buf[8192];
	if(fd_in < 0) {
		printf("Failed to open the file\n");
		return 0;
	}
  int readRet = read(fd_in,buf,8192);
  //Parse the buffer to the parse function. You will need to pass the socket fd and the buffer would need to
  //be read from that fd

  Request *request = parse(buf,readRet,fd_in);

  // GET request
  if (strcmp(request->http_method, "GET") == 0) {
    // printf("GET request");
    get_request(request);
  }

  // HEAD request
  if (strcmp(request->http_method, "HEAD") == 0) {
    // printf("Head request");
    head_request(request);
  }

  // POST request
  if (strcmp(request->http_method, "POST") == 0) {
    // printf("Post request");
    post_request(request);
  }

  // //Just printing everything
  // printf("Http Method %s\n",request->http_method);
  // printf("Http Version %s\n",request->http_version);
  // printf("Http Uri %s\n",request->http_uri);
  for(index = 0;index < request->header_count;index++){
    printf("Request Header\n");
    printf("Header name %s Header Value %s\n",request->headers[index].header_name,request->headers[index].header_value);
  }
  // free(request->headers);
  // free(request);
  return 0;
}
