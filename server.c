/* html-ls-server.c
 *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
 * whole or in part in accordance to the General Public License (GPL).
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/

/*****************************************************************************/
/*** html-ls-server.c (multitasking)                                       ***/
/***                                                                       ***/
/*** Multitask directory listing server (HTTP).                          ***/
/*****************************************************************************/

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

#define PORTNUM 9999
#define	MY_PORT		9999
#define MAXHOSTNAME 1000

void PERROR(char* msg);
#define PERROR(msg)	{ perror(msg); abort(); }
#define MAXBUF	1024
char buffer[MAXBUF];
char *Host="127.0.0.1:9999";
void do_something(int);
int establish(unsigned short portnum);

#define MAXPATH	150
/*---------------------------------------------------------------------*/
/*--- strtrim - trim the spaces off the front and back of a string. ---*/
/*---------------------------------------------------------------------*/
char* strtrim(char *str)
{	int tail=strlen(str);
	while ( str[tail] <= ' '  &&  tail > 0 )
		tail--;
	str[tail+1] = 0;
	while ( *str <= ' '  &&  *str != 0 )
		str++;
	return str;
}

/*---------------------------------------------------------------------*/
/*--- panic - report error and die.                                 ---*/
/*---------------------------------------------------------------------*/
void panic(char* msg, ...)
{	va_list ap;

	va_start(ap, msg);
	vprintf(msg, ap);
	va_end(ap);
	exit(0);
}

/*---------------------------------------------------------------------*/
/*--- dir_up - find the parent directory in a path.                 ---*/
/*---------------------------------------------------------------------*/
char* dir_up(char* dirpath)
{   static char Path[MAXPATH];
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

/*---------------------------------------------------------------------*/
/*--- DirListing - read the directory and output an HTML table      ---*/
/*---------------------------------------------------------------------*/
void DirListing(FILE* FP, char* Path)
{	struct dirent *dirent;
	struct stat info;
	char Filename[MAXPATH];
	DIR* dir = opendir(Path);
	fprintf(FP, "<html><head><title>Directory Lister</title></body>"
		"<body><font size=+4>Linux Directory Server</font><br><hr width=\"100%%\"><br><center>"
		"<table border cols=4 width=\"100%%\" bgcolor=\"#33CCFF\">");
	fprintf(FP, "<caption><font size=+3>Directory of %s</font></caption>\n", Path);
	if ( dir == 0 )
	{
		fprintf(FP, "</table><font color=\"CC0000\" size=+2>%s</font></body></html>", strerror(errno));
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
				fprintf(FP, "<td><a href=\"ftp://%s%s\">%s</a></td>", Host, Filename, dirent->d_name);
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

/*---------------------------------------------------------------------*/
/*--- main - set up client and accept connections.                  ---*/
/*---------------------------------------------------------------------*/
 

void  do_something(int client){
	int len;
	struct stat statbuf;

	printf("Connected:");
	if ( (len = recv(client, buffer, MAXBUF, 0)) > 0 )
	{
		FILE* ClientFP = fdopen(client, "w");
		if ( ClientFP == NULL )
			perror("fpopen");
		else
		{	char Req[MAXPATH];
			sscanf(buffer, "GET %s HTTP", Req);
			printf("Request: \"%s\"\n", Req);
			fprintf(ClientFP,"HTTP/1.0 200 OK\n");
			fprintf(ClientFP,"Content-type: text/html\n");
			fprintf(ClientFP,"\n");

			if (lstat(Req, &statbuf) < 0) {
				printf("Stat error.\n");
				return;
			}

			if (S_ISDIR(statbuf.st_mode)) {
				DirListing(ClientFP, Req);
			}

			fclose(ClientFP);
		}
	}
}

/* code to establish a socket */
int establish(unsigned short portnum)
{
        char myname[MAXHOSTNAME+1];
        int s;
        struct sockaddr_in sa;
        struct hostent *hp;

        memset(&sa, 0, sizeof(struct sockaddr));                                                        /* clear our address */
        gethostname(myname, MAXHOSTNAME);                                                                       /* who are we? */
        hp= gethostbyname(myname);                                                                                      /* get our address info */
        if (hp == NULL)return(-1);                                                                                      /* we don't exist !? */

        sa.sin_family= hp->h_addrtype;                                                                          /* this is our host address */
        sa.sin_addr.s_addr = INADDR_ANY;                                                                        /* this is our default IP address */
        sa.sin_port= htons(portnum);                                                                            /* this is our port number */
        if ( ( s = socket(AF_INET, SOCK_STREAM, 0)) < 0) return(-1);                       /* create socket */

        if ( bind( s, (struct sockaddr *) & sa , sizeof(sa) )  < 0)
        {
                close(s);
                return(-1);                                                                                                             /* bind address to socket */
        }
        listen(s, 3);                                                                                                           /* max # of queued connects */
        return(s);

}


main()
{
        int s, t;
        if ((s = establish(PORTNUM)) < 0)
        {                                                                                                                                       /* plug in the phone */
                perror("establish");
                exit(1);
        }
                                                                                                                                                /* how a concurrent server looks like */
        for (;;)
        {                                                                                                                                       /* loop for phone calls */
                if ((t= get_connection(s)) < 0)
                {                                                                                                                               /* get a connection */
                        perror("accept");                                                                                       /* bad */
                        exit(1);
                }
                switch( fork() )
                {                                                                                                                               /* try to handle connection */
                        case -1 :                                                                                                       /* bad news. scream and die */
                                perror("fork");
                                close(s);
                                close(t);
                                exit(1);
                        case 0 :                                                                                                        /* we're the child, do something */
                                close(s);
                                do_something(t);
                                close(t);
                                exit(0);
                        default :                                                                                                       /* we're the parent so look for */
                                close(t);                                                                                               /* another connection */
                                continue;
                }
        }
}


/* wait for a connection to occur on a socket created with establish() */
int get_connection(int s) {
int t; /* socket of connection */
if ((t = accept(s,NULL,NULL)) < 0) /* accept connection if there is one */
 return(-1);
return(t); }
