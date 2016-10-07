#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

int main (int argc, char **argv)
{
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

	fd = socket(PF_INET, SOCK_STREAM, 0);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(1118);

	bind(fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
	listen(fd, 1);

	while (1) {
		int connection_fd;
		int i, socklen = sizeof(sockaddr);
		char buffer[256];
		connection_fd = accept(fd, (struct sockaddr*)&sockaddr, &socklen);
		i = recv(connection_fd, buffer, sizeof(buffer), 0);
		if (i <= 0)
			return -1;
		printf("Received: %s\n", buffer);
		close(connection_fd);
	}
	
	
		
	
	return 0;
}
