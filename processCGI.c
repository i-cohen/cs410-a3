#include <stdio.h>
#include <string.h>
#include <unistd.h>

void  run_cgi(char* Req);


int main(char argc, char*argv[])
{
	if (argc != 2)
	{
		printf("Usage: ./processCGI cgiFileRequest\n");
		printf("Example: ./processCGI www.abc.com/table.cgi?example=fun&cs410=hard\n");
		printf("CGI file must have proper permissions. IE chmod 755\n");
	}
	else
	{
		char *req = argv[1];
		printf("%s", argv[1]);
		run_cgi(req);
	}
}

void  run_cgi(char* Req) 
{
	char * reqP[15];
	char cgi[30];
	int i = 0;

	reqP[i] = strtok(Req, "/");
	while (reqP[i] != NULL)
	{
		printf("%s\n", reqP[i]);
		i++;
		reqP[i] = strtok(NULL, "/");
		
	}
	i--;

	char* endP = reqP[i];
	printf("REQ: %s\n", endP);
	const char needle[10] = "?";
	char* startArgs = strstr(endP, needle);

	if (startArgs != NULL)
	{
		int cgiInd = startArgs-endP;
		startArgs++;
		
		memcpy(cgi, endP, cgiInd);
		cgi[cgiInd] = '\0';
		printf("FILE %s ARGS: %s\n", cgi, endP+cgiInd+1);
		endP = &cgi[0];
		printf("REQ: %s\n", endP);
	}else
	{
		printf("FILE: %s  NO ARGS \n", endP);
		startArgs = "";
	}
	char cmd[1000];
	strcpy(cmd, "./");
	strcat(cmd, endP);
	printf("FILE: %s  \n", cmd);
	// startArgs++;
	// printf("\nFnArg: %s\n", startArgs);
	// char* args[] = {"./new.cgi", startArgs,(char*)0};

	char* args[] = {cmd, startArgs,(char*)0};
	execvp(cmd, args);
	printf("DONE\n");
}