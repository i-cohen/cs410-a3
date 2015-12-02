#include <errno.h> /* obligatory includes */
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>

/*
BASE CODE COPIED FROM LECTURE SLIDES OF CS:455 Networks w/ Prof. Matta: Week4app
*/


#define PORTNUM 5000									 				/* random port number, we need something */
void do_something(int);
main() 
{
	int s, t;
	if ((s = establish(PORTNUM)) < 0) 
	{													 				/* plug in the phone */
		perror("establish");
		exit(1);
	} 
																		/* how a concurrent server looks like */
	for (;;) 
	{																	/* loop for phone calls */
		if ((t= get_connection(s)) < 0) 
		{																/* get a connection */
			perror("accept"); 											/* bad */
			exit(1);	
		}
		switch( fork() ) 
		{																/* try to handle connection */
			case -1 : 													/* bad news. scream and die */
				perror("fork");
				close(s);
				close(t);
				exit(1);
			case 0 : 													/* we're the child, do something */
				close(s);
				do_something(t);
				close(t);
				exit(0);
			default : 													/* we're the parent so look for */
				close(t); 												/* another connection */
				continue;
		}
	}
}														 				/* end of main */
																		/* this is the function that plays with the socket. It will
																		be called after getting a connection. */
void do_something(int t) 
{																		/* do your thing with the socket here */
} 


/* code to establish a socket */
int establish(unsigned short portnum) 
{
	char myname[MAXHOSTNAME+1];
	int s;
	struct sockaddr_in sa;
	struct hostent *hp;

	memset(&sa, 0, sizeof(struct sockaddr)); 							/* clear our address */
	gethostname(myname, MAXHOSTNAME); 									/* who are we? */
	hp= gethostbyname(myname); 											/* get our address info */
	if (hp == NULL)return(-1); 											/* we don't exist !? */

	sa.sin_family= hp->h_addrtype; 										/* this is our host address */
	sa.sin_addr = htonl(INADDR_ANY); 									/* this is our default IP address */
	sa.sin_port= htons(portnum); 										/* this is our port number */
	if ((s= socket(AF_INET, SOCK_STREAM, 0)) < 0) return(-1);			/* create socket */
		
	if (bind(s,(struct sockaddr *)&sa,sizeof(sa))) < 0) 
	{
		close(s);
		return(-1);														/* bind address to socket */
	}
	listen(s, 3); 														/* max # of queued connects */
	return(s); 

} 
