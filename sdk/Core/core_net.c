#if 0
#include "core.h"
#include "corex.h"
#include <winsock.h>
#pragma comment (lib, "wsock32.lib")

#include "core_windows.h"

//this stuff isn't needed use errno and strerror


static const char *WSAE_StrError (int err)
{
	switch (err)
	{
	case 0:			return "No error";
	case WSAEINTR:		return "Interrupted system call";		/* 10004 */
	case WSAEBADF:		return "Bad file number";			/* 10009 */
	case WSAEACCES:		return "Permission denied";			/* 10013 */
	case WSAEFAULT:		return "Bad address";				/* 10014 */
	case WSAEINVAL:		return "Invalid argument (not bind)";		/* 10022 */
	case WSAEMFILE:		return "Too many open files";			/* 10024 */
	case WSAEWOULDBLOCK:	return "Operation would block";			/* 10035 */
	case WSAEINPROGRESS:	return "Operation now in progress";		/* 10036 */
	case WSAEALREADY:	return "Operation already in progress";		/* 10037 */
	case WSAENOTSOCK:	return "Socket operation on non-socket";	/* 10038 */
	case WSAEDESTADDRREQ:	return "Destination address required";		/* 10039 */
	case WSAEMSGSIZE:	return "Message too long";			/* 10040 */
	case WSAEPROTOTYPE:	return "Protocol wrong type for socket";	/* 10041 */
	case WSAENOPROTOOPT:	return "Bad protocol option";			/* 10042 */
	case WSAEPROTONOSUPPORT: return "Protocol not supported";		/* 10043 */
	case WSAESOCKTNOSUPPORT: return "Socket type not supported";		/* 10044 */
	case WSAEOPNOTSUPP:	return "Operation not supported on socket";	/* 10045 */
	case WSAEPFNOSUPPORT:	return "Protocol family not supported";		/* 10046 */
	case WSAEAFNOSUPPORT:	return "Address family not supported by protocol family"; /* 10047 */
	case WSAEADDRINUSE:	return "Address already in use";		/* 10048 */
	case WSAEADDRNOTAVAIL:	return "Can't assign requested address";	/* 10049 */
	case WSAENETDOWN:	return "Network is down";			/* 10050 */
	case WSAENETUNREACH:	return "Network is unreachable";		/* 10051 */
	case WSAENETRESET:	return "Net dropped connection or reset";	/* 10052 */
	case WSAECONNABORTED:	return "Software caused connection abort";	/* 10053 */
	case WSAECONNRESET:	return "Connection reset by peer";		/* 10054 */
	case WSAENOBUFS:	return "No buffer space available";		/* 10055 */
	case WSAEISCONN:	return "Socket is already connected";		/* 10056 */
	case WSAENOTCONN:	return "Socket is not connected";		/* 10057 */
	case WSAESHUTDOWN:	return "Can't send after socket shutdown";	/* 10058 */
	case WSAETOOMANYREFS:	return "Too many references, can't splice";	/* 10059 */
	case WSAETIMEDOUT:	return "Connection timed out";			/* 10060 */
	case WSAECONNREFUSED:	return "Connection refused";			/* 10061 */
	case WSAELOOP:		return "Too many levels of symbolic links";	/* 10062 */
	case WSAENAMETOOLONG:	return "File name too long";			/* 10063 */
	case WSAEHOSTDOWN:	return "Host is down";				/* 10064 */
	case WSAEHOSTUNREACH:	return "No Route to Host";			/* 10065 */
	case WSAENOTEMPTY:	return "Directory not empty";			/* 10066 */
	case WSAEPROCLIM:	return "Too many processes";			/* 10067 */
	case WSAEUSERS:		return "Too many users";			/* 10068 */
	case WSAEDQUOT:		return "Disc Quota Exceeded";			/* 10069 */
	case WSAESTALE:		return "Stale NFS file handle";			/* 10070 */
	case WSAEREMOTE:	return "Too many levels of remote in path";	/* 10071 */
	case WSAEDISCON:	return "Graceful shutdown in progress";		/* 10101 */

	case WSASYSNOTREADY:	return "Network SubSystem is unavailable";			/* 10091 */
	case WSAVERNOTSUPPORTED: return "WINSOCK DLL Version out of range";			/* 10092 */
	case WSANOTINITIALISED:	return "Successful WSASTARTUP not yet performed";		/* 10093 */
	case WSAHOST_NOT_FOUND:	return "Authoritative answer: Host not found";			/* 11001 */
	case WSATRY_AGAIN:	return "Non-Authoritative: Host not found or SERVERFAIL";	/* 11002 */
	case WSANO_RECOVERY:	return "Non-Recoverable errors, FORMERR, REFUSED, NOTIMP";	/* 11003 */
	case WSANO_DATA:	return "Valid name, no data record of requested type";		/* 11004 */
#ifndef _WIN32 // Baker: These are undeclared in real Windows?
	case WSAENOMORE:		return "10102: No more results";			/* 10102 */
	case WSAECANCELLED:		return "10103: Call has been canceled";			/* 10103 */
	case WSAEINVALIDPROCTABLE:	return "Procedure call table is invalid";		/* 10104 */
	case WSAEINVALIDPROVIDER:	return "Service provider is invalid";			/* 10105 */
	case WSAEPROVIDERFAILEDINIT:	return "Service provider failed to initialize";		/* 10106 */
	case WSASYSCALLFAILURE:		return "System call failure";				/* 10107 */
	case WSASERVICE_NOT_FOUND:	return "Service not found";				/* 10108 */
	case WSATYPE_NOT_FOUND:		return "Class type not found";				/* 10109 */
	case WSA_E_NO_MORE:		return "10110: No more results";			/* 10110 */
	case WSA_E_CANCELLED:		return "10111: Call was canceled";			/* 10111 */
	case WSAEREFUSED:		return "Database query was refused";			/* 10112 */
#endif
	default:
		{
			static char _err_unknown[64];
			sprintf(_err_unknown, "Unknown WSAE error (%d)", err);
			return  _err_unknown;
		}
	}
}


// 
void Server_Close (nserver_t *me)
{
	if (me->active_remotesock) 	closesocket(me->remote_sock);
	if (me->active_listensock) 	closesocket(me->listen_sock);
	if (me->active_wsa)			WSACleanup ();
//	memset (me, 0, sizeof(*me)); NO!!!  
}

// Calls server shutdown, returns false
cbool Server_Error (nserver_t *me, const char *msg)
{
#define PRINTERROR(s)	\
		fprintf(stderr,"\nERROR  %s\n%: %d : %s\n", s, WSAGetLastError(), WSAE_StrError(WSAGetLastError()))
	PRINTERROR(msg);
	Server_Close (me);
	return false;
}


// Listen for connection, blocking
cbool Server_Listen (nserver_t *me)
{
	int n;
	
	if (!me->active_listensock) 
		return Server_Error (me, "No active listen socket");

	// #5 listen -  function places a socket in a state in which it is listening for an incoming connection
	//  If set to SOMAXCONN, the provider responsible for socket s will set the backlog to a maximum reasonable value	
	n = listen (me->listen_sock, SOMAXCONN);
	if (n == SOCKET_ERROR) 
		return Server_Error (me, "listen()");

	// #6 accept - function permits an incoming connection attempt on a socket.	
	// BLOCKING!!

	printf ("Listening on port %i\n", me->nport);

	me->remote_sock = accept(me->listen_sock, (struct sockaddr *)&me->client_addr, &me->addr_size );
	if (me->remote_sock == INVALID_SOCKET) 
		return Server_Error (me, "accept()");
	
	me->active_remotesock = true;
	c_snprintf5 (me->client_string, "%d.%d.%d.%d:port %i\n",
  					(int)(me->client_addr.sin_addr.s_addr&0xFF),
				    (int)((me->client_addr.sin_addr.s_addr&0xFF00)>>8),
  					(int)((me->client_addr.sin_addr.s_addr&0xFF0000)>>16),
  					(int)((me->client_addr.sin_addr.s_addr&0xFF000000)>>24),
  					(int)me->client_addr.sin_port);
	
	printf ("Connection accepted from %s\n", me->client_string);

	// #7 ioctlsocket - set non-blocking mode
	{
		u_long iMode = 1;
		ioctlsocket (me->listen_sock, FIONBIO, &iMode);
	}
	
	// Now we send data to listen_sock and receive from remote_sock, right?
	me->connected = true;
	
	return true;
}


// Connected, non-block.  Returns false if connection lost.
cbool Server_Connected_Frame (nserver_t *me)
{
	int n;
	
	if (!me->active_remotesock) 
		return Server_Error (me, "No active remote socket");
	
	memset (me->recvbuffer, 0, sizeof(me->recvbuffer));
	
	// #8 recv - recv function receives data from a connected socket or a bound connectionless socket.
	// Details: http://msdn.microsoft.com/en-us/library/windows/desktop/ms740121%28v=vs.85%29.aspx
	// connect client, receive buffer, length of buffer, flags (peek and out-of-band - which is emergency channel like CTRL-C)
	n = recv (me->remote_sock, me->recvbuffer, sizeof(me->recvbuffer), 0 /* flags */);
	if (n ==  SOCKET_ERROR) 
		return Server_Error (me, "recv()");

	me->recvbuffer[n]=0;

	printf ("%04i: Data received: %s\n", me->recvcount++, me->recvbuffer);
	
	// Reply back acknowledging
	c_snprintf (me->sendbuffer, "Received %s", me->recvbuffer);
	
	// #9 send - function sends data on a connected socket
	n = send (me->remote_sock, me->sendbuffer, strlen(me->sendbuffer)+1, 0 /* flags */);
	if (n ==  SOCKET_ERROR) 
		return Server_Error (me, "send()");
	
	// Print
	printf ("%04i: sent: %s\n", me->sendcount++, me->sendbuffer);
	return true;
}

nserver_t *Server_Shutdown (nserver_t *me)
{
	Server_Close (me);
	free (me);
	return NULL;
}


// This functions gets us listening, but we aren't accepting
//#define VERSION_1_1 = (WORD)MAKEWORD(1,1); // Version 1.1
nserver_t *Server_Instance (short port)
{
	WORD VERSION_1_1 = MAKEWORD(1,1); // Version 1.1

	nserver_t *me = calloc (sizeof(nserver_t), 1);
	
	int n;
	
	me->nport = port;
	me->start_time = time (NULL);
	me->addr_size = sizeof(me->client_addr);
	me->saServer.sin_family = AF_INET;			 // IP4
	me->saServer.sin_addr.s_addr = INADDR_ANY;	 // Let WinSock supply address
	me->saServer.sin_port = htons(me->nport);		 // Use port from command line

	// #1 WSAStartup, indicating version.
	
	n = WSAStartup (VERSION_1_1, &me->wsa_data);
	if (me->wsa_data.wVersion != VERSION_1_1)
	{	
		fprintf (stderr,"\n Wrong version\n");
		return NULL;
	} else me->active_wsa = true;
	
	// #2 socket - creates a socket that is bound to a specific transport service provider
	// Address family (AF_INET = ip4, Socket type (SOCK_STREAM for TCP/IP, SOCK_DGRM for UDP), Protocol
	me->listen_sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (me->listen_sock == INVALID_SOCKET) return (nserver_t *)Server_Error (me, "socket()");
	else me->active_listensock = true;
	
	// #3 bind - associates a local address with a socket  
	// socket, our address and size of structure
	n = bind (me->listen_sock, (LPSOCKADDR)&me->saServer, sizeof(me->saServer));
	if (n == SOCKET_ERROR) return (nserver_t *)Server_Error (me, "bind()");

	// #4 gethostname - retrieves host information corresponding to a host name from a host database
	// gethostname can only return IPv4 addresses for the name parameter
	n = gethostname(me->hostname, sizeof(me->hostname));
	if (n == SOCKET_ERROR) return (nserver_t *)Server_Error (me, "gethostname()");

	return me;
}


#endif