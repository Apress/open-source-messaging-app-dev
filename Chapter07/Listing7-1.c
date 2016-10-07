#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

int main (int argc, char **argv)
{
	struct hostent *hostent;
	int fd;
	struct sockaddr_in sockaddr;

#ifdef _WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		return -1;
	}
	
	/* Check the version */
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
	     HIBYTE( wsaData.wVersion ) != 2 ) {
		WSACleanup( );
		return -1; 
	}
#endif /* _WIN32 */
	
	if (argc != 3)
		return;

	hostent = gethostbyname(argv[1]);
	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (hostent == NULL)
		return;

	memcpy(&sockaddr.sin_addr.s_addr, hostent->h_addr, hostent->h_length);

	sockaddr.sin_port = htons(1118);
	sockaddr.sin_family = hostent->h_addrtype;
	memset(&(sockaddr.sin_zero), 0, sizeof(sockaddr.sin_zero));
	
	if (connect(fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1)
		return -1;
	send(fd, argv[2], strlen(argv[2])+1, 0);
	close(fd);
	
	return 0;
}
