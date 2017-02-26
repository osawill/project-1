#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "logger.h"

FILE * fp;

//Create/open text file
void init() {
	if ((fp = fopen("./logfile.txt","a")) == NULL){
	   perror("Error opening file: ");
   }
};

//Close out text file
void end() {
	if(fclose(fp) != 0) {
		perror("Error closing file: ");
	}
};

char * getTime() {
	char * val =(char *)malloc(6);
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	strftime(val, sizeof(val)-1, "%H:%M", t);

	return val;
};

//Write to text file/print to console
void write(int level, char * body) {
	//Assign status level
	char * status = (char*)malloc(10);
	switch(level) {
		case LOG_INFO:
			strcpy(status, "[INFO]");
			break;
		case LOG_DEBUG:
			strcpy(status, "[DEBUG]");
			break;
		case LOG_ALERT:
			strcpy(status, "[ALERT]");
			break;
		case LOG_WARN:
			strcpy(status, "[WARN!]");
			break;
	}
	//Current time
	char * time = getTime();

	//Compile log entry
	int strSize = strlen(body) + 25;

	char * line = (char*)malloc(strSize);
	sprintf(line, "%s%s - %s\r\n", status, time, body);

	//Export line to console & file
	fprintf(fp, "%s", line);
	printf("%s", line);

	//Free the memory
	free(status);
	free(line);
	free(time);
};

Log_class const Log = {init, write, end};
/*int main() {
	Log.init();
	Log.write(1, "We are good");
	Log.end();
	return 0;
}*/

