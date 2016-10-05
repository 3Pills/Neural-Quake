/*
 Copyright (C) 1996-2001 Id Software, Inc.
 Copyright (C) 2002-2005 John Fitzgibbons and others
 Copyright (C) 2007-2008 Kristian Duske
 Copyright (C) 2009-2014 Baker and others
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 
 See the GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
 */

// net_udp.c

#include <core.h>
#include "q_stdinc.h"
#include "arch_def.h"
#include "net_sys.h"
#include "quakedef.h"
#include "net_defs.h"

static sys_socket_t net_acceptsocket = INVALID_SOCKET;	// socket for fielding new connections
static sys_socket_t net_controlsocket;
static sys_socket_t net_broadcastsocket = 0;
static struct sockaddr_in broadcastaddr;

static in_addr_t	myAddr;

#include "net_udp.h"

#ifdef _WIN32 // Netdiff
#pragma comment (lib, "wsock32.lib")
int winsock_initialized = 0;
#include "wsaerror.h"
#endif

//=============================================================================

static double	blocktime;


static void UDP_GetLocalAddress (in_addr_t *outAddr, char *buf, size_t bufsize)
{
	struct hostent	*local = NULL;
	char		namebuff[MAXHOSTNAMELEN];
	in_addr_t	addr;
	
	if (myAddr != INADDR_ANY)
		return;
	
	if (gethostname(namebuff, sizeof(namebuff)) == SOCKET_ERROR)
	{
		int err = SOCKETERRNO;
		Con_SafePrintf("UDP_GetLocalAddress: gethostname failed (%s)\n", socketerror(err));
		return;
	}
	
	namebuff[sizeof(namebuff) - 1] = 0;
	blocktime = System_DoubleTime();
	local = gethostbyname(namebuff);
	if (local == NULL)
	{
		int err = SOCKETERRNO;
		Con_SafePrintf ("UDP_GetLocalAddress: gethostbyname failed (%s)\n", socketerror(err));
		return;
	}
	else if (local->h_addrtype != AF_INET)
	{
		Con_SafePrintf("UDP_GetLocalAddress: address from gethostbyname not IPv4\n");
		return;
	}
	
	*outAddr = *(in_addr_t *)local->h_addr_list[0];
	
	addr = ntohl(*outAddr); // Net byte order to host order
	c_snprintfc (buf, bufsize, "%ld.%ld.%ld.%ld", (addr >> 24) & 0xff, (addr >> 16) & 0xff, (addr >> 8) & 0xff, addr & 0xff);
}


static int UDP_Platform_Startup (void)
{
#ifdef _WIN32
	if (winsock_initialized == 0)
	{
		WSADATA	winsockdata;
		int err = WSAStartup(MAKEWORD(1,1), &winsockdata);
		if (err != 0)
		{
			Con_SafePrintf("Winsock initialization failed (%s)\n", socketerror(err));
			return 0;
		}
	}
	winsock_initialized++;
#endif
	
#ifdef PLATFORM_OSX
    // Baker: This tells the sockets to be in non-blocking mode
	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
#endif // PLATFORM_OSX
	return 1;
}

static void UDP_Platform_Cleanup (void)
{
#ifdef _WIN32
	if (--winsock_initialized == 0)
		WSACleanup ();
#endif // _WIN32
}

// Baker: The return value for UDP_Init is not used in the current
// source.  Let's return the socket like the source suggests
// even though I believe the return value from the function looks more
// designed for a boolean like return
sys_socket_t UDP_Init (void)
{
	char	*colon;
	struct qsockaddr	addr;
	int		t;
	
#pragma message ("kill me")
	#pragma message ("kill me")
	#pragma message ("kill me")
	#pragma message ("kill me")
	#pragma message ("kill me")
	#pragma message ("kill me")
#pragma message ("kill me")
	#pragma message ("kill me")
	if (COM_CheckParm ("-noudp"))
{
UDP_Platform_Startup(); // Kill This!!!  It exists because we don't WSAStartup for net_simple
		return INVALID_SOCKET;
}	


	if (!UDP_Platform_Startup())
		return INVALID_SOCKET;
	
	// Set default values
	myAddr = INADDR_ANY;
	c_strlcpy (my_tcpip_address, "INADDR_ANY");
	
	// Fills the my_tcpip_address with the local address
	
	switch ( (t = COM_CheckParm ("-ip")) )
	{
		case 0:  // -ip not specified
			UDP_GetLocalAddress (&myAddr, my_tcpip_address, sizeof(my_tcpip_address));
			break;
			
		default: // -ip was specified
			if ( !(t + 1 < com_argc) )
				System_Error ("NET_Init: you must specify an IP address after -ip");
			
			myAddr = inet_addr (com_argv[t+1] );
			
			if (myAddr == INADDR_NONE)
				System_Error ("%s is not a valid IP address", com_argv[t+1]);
			
			c_strlcpy (my_tcpip_address, com_argv[t+1]);
			
			Con_SafePrintf ("UDP_Init: -ip command line to %s\n", my_tcpip_address);
			break;
	}
	
	if ((net_controlsocket = UDP_OpenSocket(0)) == INVALID_SOCKET)
	{
		Con_SafePrintf("UDP_Init: Unable to open control socket, UDP disabled\n");
		UDP_Platform_Cleanup ();
		return INVALID_SOCKET;
	}
	
	broadcastaddr.sin_family = AF_INET;
	broadcastaddr.sin_addr.s_addr = INADDR_BROADCAST;
	broadcastaddr.sin_port = htons((unsigned short)net_hostport);
	
	// This reconstructs the my_tcpip_address out of the socket, validating what we got.
	UDP_GetSocketAddr (net_controlsocket, &addr);
	c_strlcpy (my_tcpip_address, UDP_AddrToString (&addr));
	colon = strrchr (my_tcpip_address, ':');
	if (colon)
		*colon = 0;
	
	Con_SafePrintf("UDP Initialized: %s\n", my_tcpip_address);
	tcpipAvailable = true;
	
	return net_controlsocket;
}

//=============================================================================

void UDP_Shutdown (void)
{
	UDP_Listen (false);
	UDP_CloseSocket (net_controlsocket);
	UDP_Platform_Cleanup ();
}

//=============================================================================

void UDP_Listen (cbool state)
{
	// enable listening
	if (state)
	{
		if (net_acceptsocket != INVALID_SOCKET)
			return;
		
		if ((net_acceptsocket = UDP_OpenSocket (net_hostport)) == INVALID_SOCKET)
			System_Error ("UDP_Listen: Unable to open accept socket");
		return;
	}
	
	// disable listening
	if (net_acceptsocket == INVALID_SOCKET)
		return;
	UDP_CloseSocket (net_acceptsocket);
	net_acceptsocket = INVALID_SOCKET;
}

//=============================================================================

sys_socket_t UDP_OpenSocket (int port)
{
	sys_socket_t newsocket;
	struct sockaddr_in address;
#ifdef _WIN32
    u_long _true = 1; // Silly.  Win64 is this 64 bits?
#else
	unsigned _true = 1; // u_long for ioctlsocket, unclear for ioctl
#endif
	int err;
	
	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		err = SOCKETERRNO;
		Con_SafePrintf ("UDP_OpenSocket: %s\n", socketerror(err));
		return INVALID_SOCKET;
	}
	
	// Set socket to non-blocking.  fnctl is the POSIX way
	if (ioctlsocket (newsocket, FIONBIO, &_true) == SOCKET_ERROR)
		goto ErrorReturn;
	
	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	
	address.sin_addr.s_addr = myAddr;
	address.sin_port = htons((unsigned short)port);
	
	if (bind (newsocket, (struct sockaddr *)&address, sizeof(address)) == 0)
	{
		int newport = ((struct sockaddr_in*)&address)->sin_port;
		//System_Alert ("Client port on the server is %d\n", newport);
		//System_Alert (UDP_AddrToString (&address));
		return newsocket;
	}
	
	if (tcpipAvailable)
	{
		err = SOCKETERRNO;
		Host_Error ("Unable to bind to %s (%s)", UDP_AddrToString ((struct qsockaddr *) &address), socketerror(err));
	}
	/* else: we are still in init phase, no need to error */
	
ErrorReturn:
	err = SOCKETERRNO;
	Con_SafePrintf("UDP_OpenSocket: %s\n", socketerror(err));
	UDP_CloseSocket (newsocket);
	return INVALID_SOCKET;
}

//=============================================================================

int UDP_CloseSocket (sys_socket_t socketid)
{
	if (socketid == net_broadcastsocket)
		net_broadcastsocket = 0;
	return closesocket (socketid);
}

//=============================================================================

/*
 ============
 PartialIPAddress
 
 this lets you type only as much of the net address as required, using
 the local network components to fill in the rest
 ============
 */
static int PartialIPAddress (const char *in, struct qsockaddr *hostaddr)
{
	char	buff[256];
	char	*b;
	int	addr, mask, num, port, run;
	
	buff[0] = '.';
	b = buff;
	strlcpy (buff + 1, in, sizeof(buff)-1);
	if (buff[1] == '.')
		b++;
	
	addr = 0;
	mask = -1;
	while (*b == '.')
	{
		b++;
		num = 0;
		run = 0;
		while (!( *b < '0' || *b > '9'))
		{
			num = num*10 + *b++ - '0';
			if (++run > 3)
				return -1;
		}
		if ((*b < '0' || *b > '9') && *b != '.' && *b != ':' && *b != 0)
			return -1;
		if (num < 0 || num > 255)
			return -1;
		mask <<= 8;
		addr = (addr<<8) + num;
	}
	
	if (*b++ == ':')
		port = atoi(b);
	else
		port = net_hostport;
	
	hostaddr->qsa_family = AF_INET;
	((struct sockaddr_in *)hostaddr)->sin_port = htons((unsigned short) port);
	((struct sockaddr_in *)hostaddr)->sin_addr.s_addr = (myAddr & htonl(mask)) | htonl(addr);
	
	return 0;
}

//=============================================================================

int UDP_Connect (sys_socket_t socketid, struct qsockaddr *addr)
{
	return 0;
}

//=============================================================================

sys_socket_t UDP_CheckNewConnections (void)
{
	if (net_acceptsocket == INVALID_SOCKET)
		return INVALID_SOCKET;
	
	
	// Baker: Check socket for connection.
	// WinSock vs. BSD have different methods for reading the buffer without
	// removing the read from the buffer (WinSock: recvfrom with  MSG_PEEK vs. BSD where
	// we request ioctl FIONREAD and available returns the count and if the count > 0
	// we have data
	{
#ifdef _WIN32 // Netdiff
		char		buf[4096];
		if (recvfrom (net_acceptsocket, buf, sizeof(buf), MSG_PEEK, NULL, NULL) == SOCKET_ERROR)
			return INVALID_SOCKET;
#else
		int		available;
		if (ioctl (net_acceptsocket, FIONREAD, &available) == -1)
		{
			int err = SOCKETERRNO;
			System_Error ("UDP: ioctlsocket (FIONREAD) failed (%s)", socketerror(err));
		}
		
		if (!available)
		{
			struct sockaddr_in	from;
			socklen_t	fromlen;
			char		buff[1];
			// quietly absorb empty packets
			recvfrom (net_acceptsocket, buff, 0, 0, (struct sockaddr *) &from, &fromlen);
			
			return INVALID_SOCKET;
		}
#endif // end netdiff
	}
	
	return net_acceptsocket;
}

//=============================================================================

// dfunc.Read
int UDP_Read (sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr)
{
	socklen_t addrlen = sizeof(struct qsockaddr);
	int ret;
	
	// BSD recvfrom arg2 buf is type void *, Windows is type char * hence cast for Windows
	ret = recvfrom (socketid, (char *)buf, len, 0, (struct sockaddr *)addr, &addrlen);
	if (ret == SOCKET_ERROR)
	{
		int err = SOCKETERRNO;
		if (err == NET_EWOULDBLOCK || err == NET_ECONNREFUSED)
			return 0;
		Con_SafePrintf ("UDP_Read, recvfrom: %s\n", socketerror(err));
	}
	return ret;
}

//=============================================================================

static int UDP_MakeSocketBroadcastCapable (sys_socket_t socketid)
{
	int	i = 1;
	
	// make this socket broadcast capable
	if (setsockopt(socketid, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) == SOCKET_ERROR)
	{
		int err = SOCKETERRNO;
		Con_SafePrintf ("UDP, setsockopt: %s\n", socketerror(err));
		return -1;
	}
	net_broadcastsocket = socketid;
	
	return 0;
}

//=============================================================================

int UDP_Broadcast (sys_socket_t socketid, byte *buf, int len)
{
	int	ret;
	
	if (socketid != net_broadcastsocket)
	{
		if (net_broadcastsocket != 0)
			System_Error ("Attempted to use multiple broadcasts sockets");
		
		ret = UDP_MakeSocketBroadcastCapable (socketid);
		if (ret == -1)
		{
			Con_Printf("Unable to make socket broadcast capable\n");
			return ret;
		}
	}
	
	return UDP_Write (socketid, buf, len, (struct qsockaddr *)&broadcastaddr);
}

//=============================================================================

int UDP_Write (sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr)
{
	int	ret;
	// BSD sendto arg2 buf is type const void *, Windows is type const char * hence cast for Windows
	ret = sendto (socketid, (const char *)buf, len, 0, (struct sockaddr *)addr, sizeof(struct qsockaddr));
	if (ret == SOCKET_ERROR)
	{
		int err = SOCKETERRNO;
		if (err == NET_EWOULDBLOCK)
			return 0;
		Con_SafePrintf ("UDP_Write, sendto: %s\n", socketerror(err));
	}
	return ret;
}

//=============================================================================

// Baker: In the source called as dfunc.AddrToString.
// Used a lot.  Returns like "192.168.1.2:26001"
const char *UDP_AddrToString (struct qsockaddr *addr)
{
	static char buffer[22]; // 192.168.100.100:26001 is 21 chars
	int		haddr;
	
	haddr = ntohl(((struct sockaddr_in *)addr)->sin_addr.s_addr);
	c_snprintf5 (buffer, "%d.%d.%d.%d:%d", (haddr >> 24) & 0xff,
				 (haddr >> 16) & 0xff, (haddr >> 8) & 0xff, haddr & 0xff,
				 ntohs(((struct sockaddr_in *)addr)->sin_port));
	return buffer;
}

//=============================================================================

int UDP_StringToAddr (const char *string, struct qsockaddr *addr)
{
	int	ha1, ha2, ha3, ha4, hp, ipaddr;
	
	sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;
	
	addr->qsa_family = AF_INET;
	((struct sockaddr_in *)addr)->sin_addr.s_addr = htonl(ipaddr);
	((struct sockaddr_in *)addr)->sin_port = htons((unsigned short)hp);
	return 0;
}

//=============================================================================

// Baker: dfunc.GetSocketAddr returns int
// In the current source, the return value is never used nor checked
// The return value is 0 for success, non-zero for failure.
int UDP_GetSocketAddr (sys_socket_t socketid, struct qsockaddr *addr)
{
	socklen_t addrlen = sizeof(struct qsockaddr);
	in_addr_t a;
	
	memset(addr, 0, sizeof(struct qsockaddr));
	
	// Baker: getsockname returns 0 on success
	if (getsockname(socketid, (struct sockaddr *)addr, &addrlen) != 0)
		return -1;
	
	a = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
	if (a == 0 || a == htonl(INADDR_LOOPBACK))
		((struct sockaddr_in *)addr)->sin_addr.s_addr = myAddr;
	
	return 0;
}

//=============================================================================

int UDP_GetNameFromAddr (struct qsockaddr *addr, char *name, size_t len)
{
	/* From ProQuake: "commented this out because it's slow and completely useless"
	 struct hostent *hostentry;
	 
	 hostentry = gethostbyaddr ((char *)&((struct sockaddr_in *)addr)->sin_addr,
	 sizeof(struct in_addr), AF_INET);
	 if (hostentry)
	 {
	 strncpy (name, (char *)hostentry->h_name, NET_NAMELEN - 1);
	 return 0;
	 }
	 */
	
	strlcpy (name, UDP_AddrToString (addr), len); // Baker: name length is unknown
	return 0;
}

//=============================================================================

int UDP_GetAddrFromName (const char *name, struct qsockaddr *addr)
{
	struct hostent *hostentry;
	
	if (name[0] >= '0' && name[0] <= '9')
		return PartialIPAddress (name, addr);
	
	hostentry = gethostbyname (name);
	if (!hostentry)
		return -1;
	
	addr->qsa_family = AF_INET;
	((struct sockaddr_in *)addr)->sin_port = htons((unsigned short)net_hostport);
	((struct sockaddr_in *)addr)->sin_addr.s_addr = *(in_addr_t *)hostentry->h_addr_list[0];
	
	return 0;
}

//=============================================================================

// Note: This returns 3 distinct values
// -1 is no match, 0 is match, 1 is ip match but not port match
int UDP_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2)
{
	if (addr1->qsa_family != addr2->qsa_family)
		return -1;
	
	if (((struct sockaddr_in *)addr1)->sin_addr.s_addr !=
	    ((struct sockaddr_in *)addr2)->sin_addr.s_addr)
		return -1;
	
	if (((struct sockaddr_in *)addr1)->sin_port !=
	    ((struct sockaddr_in *)addr2)->sin_port)
		return 1;
	
	return 0;
}

//=============================================================================

int UDP_GetSocketPort (struct qsockaddr *addr)
{
	return ntohs(((struct sockaddr_in *)addr)->sin_port);
}


int UDP_SetSocketPort (struct qsockaddr *addr, int port)
{
	((struct sockaddr_in *)addr)->sin_port = htons((unsigned short)port);
	return 0;
}
