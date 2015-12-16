/*Sources used for server and directory listing: https://www.cs.utah.edu/~swalton/listings/sockets/programs/part2/chap6/html-ls-server.c
 *Source used for sockets: CS 455 Networks Slides Sockets From Matta 2015 Applications Slide 47
 */

#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>

int portNum = 9999;
#define	MY_PORT		9999
#define MAXHOSTNAME 	1000

void PERROR(char* msg);
#define PERROR(msg)	{ perror(msg); abort(); }
#define MAXBUF	1024
char buffer[MAXBUF];
char *Host="127.0.0.1:9999";

char* strtrim(char *str);
void panic(char* msg, ...);
char* dir_up(char* dirpath);
void DirListing(FILE* FP, char* Path);
void do_something(int client);
void sendFileOverSocket(int client, char* filename);
int checkExtension(char *fileName);
int establish(unsigned short portnum);
int get_connection(int s);

#define MAXPATH	150

#define FILE_HTML    1    /*HTML file*/
#define FILE_JPG     2    /*.jpg, or .jpeg file*/
#define FILE_GIF     3    /*.gif file*/
#define FILE_CGI     4    /*.cgi script*/

/*strtrim - trim the spaces off the front and back of a string. Used from source above*/
char* strtrim(char *str) {
	int tail=strlen(str);

	while ( str[tail] <= ' '  &&  tail > 0 )
		tail--;
	str[tail+1] = 0;

	while ( *str <= ' '  &&  *str != 0 )
		str++;
	return str;
}

/*panic - report error and die. Used from source above*/
void panic(char* msg, ...) {
	va_list ap;

	va_start(ap, msg);
	vprintf(msg, ap);
	va_end(ap);
	exit(0);
}

/*dir_up - find the parent directory in a path. Used from source above*/
char* dir_up(char* dirpath) {
	static char Path[MAXPATH];
	int len;

	strcpy(Path, dirpath);
	len = strlen(Path);
	if ( len > 1  &&  Path[len-1] == '/' )
		len--;
	while ( Path[len-1] != '/'  &&  len > 1 )
		len--;
	Path[len] = 0;
	return Path;
}

#define MAXNAME	25

/*DirListing - read the directory and output an HTML table. Used partially from source above*/
void DirListing(FILE* FP, char* Path) {
	struct dirent *dirent;
	struct stat info;
	char Filename[MAXPATH];
	DIR* dir = opendir(Path);
	fprintf(FP, "<html><head><title>Directory Lister</title></body>"
		"<body><font size=+4>CS410 Web Server</font><br><hr width=\"100%%\"><br><center>"
		"<table border cols=4 width=\"100%%\" bgcolor=\"009999\">");
	fprintf(FP, "<caption><font size=+3>Directory of %s</font></caption>\n", Path);
	if ( dir == 0 )
	{
		fprintf(FP, "</table><font color=\"white\" size=+2>%s</font></body></html>", strerror(errno));
		return;
	}
	while ( (dirent = readdir(dir)) != 0 )
	{
		if ( strcmp(Path, "/") == 0 )
			sprintf(Filename, "/%s", dirent->d_name);
		else
			sprintf(Filename, "%s/%s", Path, dirent->d_name);
		fprintf(FP, "<tr>");
		if ( stat(Filename, &info) == 0 )
		{
			fprintf(FP, "<td>%s</td>", dirent->d_name);
			if ( S_ISDIR(info.st_mode) )
			{
				if ( strcmp(dirent->d_name, "..") == 0 )
					fprintf(FP, "<td><a href=\"http://%s%s\">(parent)</a></td>", Host, dir_up(Path));
				else
					fprintf(FP, "<td><a href=\"http://%s%s\">%s</a></td>", Host, Filename, dirent->d_name);
				fprintf(FP, "<td>Directory</td>");
			}
			else if ( S_ISREG(info.st_mode) )
			{
				fprintf(FP, "<td><a href=\"http://%s%s\">%s</a></td>", Host, Filename, dirent->d_name);
				fprintf(FP, "<td>%d</td>", info.st_size);
			}
			else if ( S_ISLNK(info.st_mode) )
				fprintf(FP, "<td>Link</td>");
			else if ( S_ISCHR(info.st_mode) )
				fprintf(FP, "<td>Character Device</td>");
			else if ( S_ISBLK(info.st_mode) )
				fprintf(FP, "<td>Block Device</td>");
			else if ( S_ISFIFO(info.st_mode) )
				fprintf(FP, "<td>FIFO</td>");
			else if ( S_ISSOCK(info.st_mode) )
				fprintf(FP, "<td>Socket</td>");
			else
				fprintf(FP, "<td>(unknown)</td>");
			fprintf(FP, "<td>%s</td>", strtrim(ctime(&info.st_ctime)));
		}
		else
			perror(Path);
		fprintf(FP, "</tr>\n");
	}
	fprintf(FP, "</table></center></body></html>");
}

/*do_something - processes requests from client*/
void  do_something(int client) {
	int len;
	struct stat statbuf;
	int fileType;

	printf("Connected\n");
	if ((len = recv(client, buffer, MAXBUF, 0)) > 0 ) {
		FILE* ClientFP = fdopen(client, "w");

		if ( ClientFP == NULL )
			perror("fpopen");

		else {
			char Req[MAXPATH];
			char *args;
			int hasArgs=0;
			sscanf(buffer, "GET %s HTTP", Req);
			if (strpbrk(Req, "?") != 0) {
				char *Req1;
				printf("has args?\n");
				hasArgs=1;
				Req1 = strtok(Req, "?");
				args = strtok(NULL, "?");
				printf("args: \"%s\"\n", args);
			}

			printf("Request: \"%s\"\n", Req);
			
			if (lstat(Req, &statbuf) < 0) {
				write(client,"HTTP/1.0 404 Not Found\n", 23);
                                write(client,"Content-type: text/html\n", 24);
                                write(client,"\n",1);
                        	sendFileOverSocket(client,"bad.html");
				printf("Stat error.\n");
			}

			else if (S_ISDIR(statbuf.st_mode)) {
				printf("Is Dir\n");
				fprintf(ClientFP,"HTTP/1.0 200 OK\n");
				fprintf(ClientFP,"Content-type: text/html\n");
				fprintf(ClientFP,"\n");
				DirListing(ClientFP, Req);
			}

			else {
				printf("checking file type\n");
				fileType = checkExtension(Req);
				printf("file type is %d\n", fileType);

				if (fileType == 1) {
					printf("Is html file.\n");
					write(client,"HTTP/1.0 200 OK\n",16 );
                                        write(client,"Content-type: text/html\n", 25);
                                        write(client,"\n",1);
                                        sendFileOverSocket(client,Req);
				}

				else if (fileType == 2) {
					printf("Is jpeg file.\n");
					write(client,"HTTP/1.0 200 OK\n",16 );
					write(client,"Content-type: image/jpeg\n", 25);
					write(client,"\n",1);
					sendFileOverSocket(client,Req);
				}

				else if (fileType == 3) {					
					printf("Is gif file.\n");
					write(client,"HTTP/1.0 200 OK\n",16 );
                                        write(client,"Content-type: image/gif\n", 24);
                                        write(client,"\n",1);
                                        sendFileOverSocket(client,Req);
				}

				else if (fileType == 4) {
					printf("is cgi file.\n");
					dup2(client,1);
					write(client,"HTTP/1.0 200 OK\n",16 );
        	                                
					if(hasArgs==0){
						printf("Executing with no args");
						execlp(Req,Req,NULL);
					}
					else{
						execlp(Req,Req,args,NULL);	
					}
					
				}

				else {
					printf("Incorrect file.\n");
					write(client,"HTTP/1.0 501 Not Implemented\n",23);
                                        write(client,"Content-type: text/plain\n", 25);
                                        write(client,"\n",1);
					write(client,"Viewing this file type has not been implemented",47);
					
				}
			}
			printf("about to close FP");
			fclose(ClientFP);
		}
	}
}

/*sendFileOverSocket - sends a file over a socket, hence the name*/
void sendFileOverSocket(int client, char* filename){
    off_t offset = 0;          /* file offset */
    int rc;                    /* holds return code of system calls */
    int fd;                    /* file descriptor for file to send */
    struct stat stat_buf;      /* argument to fstat */
    printf("received request to send file %s\n", filename);

    /* open the file to be sent */
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
      fprintf(stderr, "unable to open '%s': %s\n", filename, strerror(errno));
      exit(1);
    }

    /* get the size of the file to be sent */
    fstat(fd, &stat_buf);

    /* copy file using sendfile */
    offset = 0;
    rc = sendfile (client, fd, &offset, stat_buf.st_size);
    if (rc == -1) {
      fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
      exit(1);
    }
    if (rc != stat_buf.st_size) {
      fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",
              rc,
              (int)stat_buf.st_size);
      exit(1);
    }

    /* close descriptor for file that was sent */
    close(fd);



}

/* Check extension of file */
int checkExtension(char *fileName) {
	char extension1[40];
	char extension2[40];
	int type = NULL;

	strncpy(extension1, fileName + (strlen(fileName) - 4), 4);
	strncpy(extension2, fileName + (strlen(fileName) - 5), 5);
	extension1[4] ='\0';
	extension2[5] ='\0';

	if (strcmp(extension2, ".html") == 0) {
		type = 1;
	}

	else if (strcmp(extension1, ".jpg") == 0 || strcmp(extension2, ".jpeg") == 0) {
		type = 2;
	}

	else if (strcmp(extension1, ".gif") == 0) {
		type = 3;
  	}

	else if (strcmp(extension1, ".cgi") == 0) {
		type = 4;
	}

	else {
		type = -1;
	}

	return type;
}

/* code to establish a socket. Code from source above*/
int establish(unsigned short portnum)
{
        char myname[MAXHOSTNAME+1];
        int s;
        struct sockaddr_in sa;
        struct hostent *hp;

        memset(&sa, 0, sizeof(struct sockaddr));      /* clear our address */
        gethostname(myname, MAXHOSTNAME);             /* who are we? */
        hp= gethostbyname(myname);                    /* get our address info */
        if (hp == NULL)return(-1);                    /* we don't exist !? */

        sa.sin_family= hp->h_addrtype;                /* this is our host address */
        sa.sin_addr.s_addr = INADDR_ANY;              /* this is our default IP address */
        sa.sin_port= htons(portnum);                  /* this is our port number */
        if ( ( s = socket(AF_INET, SOCK_STREAM, 0)) < 0) return(-1);  /* create socket */

        if ( bind( s, (struct sockaddr *) & sa , sizeof(sa) )  < 0)
        {
                close(s);
                return(-1);                           /* bind address to socket */
        }
        listen(s, 3);                                 /* max # of queued connects */
        return(s);

}

/* wait for a connection to occur on a socket created with establish*/
int get_connection(int s) {
	int t; /*socket of connection*/
        if ((t = accept(s, NULL, NULL)) < 0) { /*accept connection if there is one*/
		return(-1);
	}
	return(t);
}

/*Main - establishes connection to server and sets up client. Partially from source above*/
main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Port number not specified.\n");
		exit(0);
	}
	portNum = atoi(argv[1]);
	if (portNum < 5000 || portNum > 65536) {
		printf("Port number out of range.\n");
		exit(0);
	}
        int s, t;
        if ((s = establish(portNum)) < 0)
        {                                    
                perror("establish");
                exit(1);
        }
                                             
        for (;;)
        {                                    
                if ((t= get_connection(s)) < 0)
                {                            /* get a connection */
                        perror("accept");    /* bad */
                        exit(1);
                }
                switch( fork() )
                {                       /* try to handle connection */
                        case -1 :      /* bad news. scream and die */
                                perror("fork");
                                close(s);
                                close(t);
                                exit(1);
                        case 0 :              /* we're the child, do something */
                                close(s);
                                do_something(t);
                                close(t);
                                exit(0);
                        default :              /* we're the parent so look for */
                                close(t);      /* another connection */
                                continue;
                }
        }
}
                

