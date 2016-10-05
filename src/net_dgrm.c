/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
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
// net_dgrm.c

// This is enables a simple IP banning mechanism
#define BAN_TEST

#include <core.h>
#include "q_stdinc.h"
#include "arch_def.h"
#include "net_sys.h"
#include "quakedef.h"
#include "net_defs.h"
#include "net_dgrm.h"

// these two macros are to make the code more readable
#define sfunc	net_landrivers[sock->landriver]
#define dfunc	net_landrivers[net_landriverlevel]

static int net_landriverlevel;

/* statistic counters */
static int	packetsSent = 0;
static int	packetsReSent = 0;
static int packetsReceived = 0;
static int receivedDuplicateCount = 0;
static int shortPacketCount = 0;
static int droppedDatagrams;

static struct
{
	unsigned int	length;
	unsigned int	sequence;
	byte			data[MAX_MARK_V_DATAGRAM];
} packetBuffer;

static int myDriverLevel;

extern cbool m_return_onerror;
extern char m_return_reason[32];


static char *StrAddr (struct qsockaddr *addr)
{
	static char buf[34];
	byte *p = (byte *)addr;
	int n;

	for (n = 0; n < 16; n++)
		sprintf (buf + n * 2, "%02x", *p++);
	return buf;
}


#ifdef BAN_TEST

static struct in_addr	banAddr;
static struct in_addr	banMask;

// This will be going away.
void NET_Ban_f (lparse_t *line)
{
	char	addrStr [32];
	char	maskStr [32];
	int	    (*print_fn)(const char *fmt, ...)
				__fp_attribute__((__format__(__printf__,1,2)));

	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer (line);
			return;
		}
		print_fn = Con_Printf;
	}
	else
	{
		if (pr_global_struct->deathmatch && !host_client->privileged)
			return;
		print_fn = SV_ClientPrintf;
	}

// This function is just a gatekeeper now.
//	Ban_f (line);
	switch (line->count)
	{
		case 1:
		if (banAddr.s_addr != INADDR_ANY)
			{
			c_strlcpy (addrStr, inet_ntoa(banAddr));
			c_strlcpy (maskStr, inet_ntoa(banMask));
				print_fn("Banning %s [%s]\n", addrStr, maskStr);
			}
			else
				print_fn("Banning not active\n");
			break;

		case 2:
			if (strcasecmp(line->args[1], "off") == 0)
				banAddr.s_addr = INADDR_ANY;
			else
				banAddr.s_addr = inet_addr(line->args[1]);
			banMask.s_addr = INADDR_NONE;
			break;

		case 3:
			banAddr.s_addr = inet_addr(line->args[1]);
			banMask.s_addr = inet_addr(line->args[2]);
			break;

		default:
			print_fn("BAN ip_address [mask]\n");
			break;
	}
}
#endif	// BAN_TEST


int Datagram_SendMessage (qsocket_t *sock, sizebuf_t *data)
{
	unsigned int	packetLen;
	unsigned int	dataLen;
	unsigned int	eom;

#ifdef DEBUG
	if (data->cursize == 0)
		System_Error("Datagram_SendMessage: zero length message");

	if (data->cursize > NET_FITZQUAKE_MAXMESSAGE)
		System_Error("Datagram_SendMessage: message too big %u\n", data->cursize);

	if (sock->canSend == false)
		System_Error("SendMessage: called with canSend == false");
#endif

//	Con_Printf ("Sending data to %s with length of %i (max: %i) with maxsize of %i\n", sock->address,  data->cursize, data->maxsize, sv.datagram.maxsize);

	memcpy (sock->sendMessage, data->data, data->cursize);
	sock->sendMessageLength = data->cursize;

#ifdef SUPPORTS_SERVER_PROTOCOL_15
	if (data->cursize <= host_protocol_datagram_maxsize)
#endif // SUPPORTS_SERVER_PROTOCOL_15
	{
		dataLen = data->cursize;
		eom = NETFLAG_EOM;
	}
	else
	{
#ifdef SUPPORTS_SERVER_PROTOCOL_15
		dataLen = host_protocol_datagram_maxsize;
#endif // SUPPORTS_SERVER_PROTOCOL_15
		eom = 0;
	}
	packetLen = NET_HEADERSIZE + dataLen;

	packetBuffer.length = BigLong(packetLen | (NETFLAG_DATA | eom));
	packetBuffer.sequence = BigLong(sock->sendSequence++);
	memcpy (packetBuffer.data, sock->sendMessage, dataLen);

	sock->canSend = false;

//	Con_Printf ("Datagram sent with size %d, maxsize should be %d\n", packetLen, host_protocol_datagram_maxsize);

	if (sfunc.Write (sock->socket, (byte *)&packetBuffer, packetLen, &sock->addr) == -1)
		return -1;

	sock->lastSendTime = net_time;
	packetsSent++;

	return 1;
}


static int SendMessageNext (qsocket_t *sock)
{
	unsigned int	packetLen;
	unsigned int	dataLen;
	unsigned int	eom;

#ifdef SUPPORTS_SERVER_PROTOCOL_15
	if (sock->sendMessageLength <= host_protocol_datagram_maxsize)
#endif // SUPPORTS_SERVER_PROTOCOL_15
	{
		dataLen = sock->sendMessageLength;
		eom = NETFLAG_EOM;
	}
	else
	{
#ifdef SUPPORTS_SERVER_PROTOCOL_15
		dataLen = host_protocol_datagram_maxsize;
#endif // SUPPORTS_SERVER_PROTOCOL_15
		eom = 0;
	}
	packetLen = NET_HEADERSIZE + dataLen;

	packetBuffer.length = BigLong(packetLen | (NETFLAG_DATA | eom));
	packetBuffer.sequence = BigLong(sock->sendSequence++);
	memcpy (packetBuffer.data, sock->sendMessage, dataLen);

	sock->sendNext = false;

	if (sfunc.Write (sock->socket, (byte *)&packetBuffer, packetLen, &sock->addr) == -1)
		return -1;

	sock->lastSendTime = net_time;
	packetsSent++;

	return 1;
}


static int ReSendMessage (qsocket_t *sock)
{
	unsigned int	packetLen;
	unsigned int	dataLen;
	unsigned int	eom;

#ifdef SUPPORTS_SERVER_PROTOCOL_15
	if (sock->sendMessageLength <= host_protocol_datagram_maxsize)
#endif // SUPPORTS_SERVER_PROTOCOL_15
	{
		dataLen = sock->sendMessageLength;
		eom = NETFLAG_EOM;
	}
	else
	{
#ifdef SUPPORTS_SERVER_PROTOCOL_15
		dataLen = host_protocol_datagram_maxsize;
#endif // SUPPORTS_SERVER_PROTOCOL_15
		eom = 0;
	}
	packetLen = NET_HEADERSIZE + dataLen;

	packetBuffer.length = BigLong(packetLen | (NETFLAG_DATA | eom));
	packetBuffer.sequence = BigLong(sock->sendSequence - 1);
	memcpy (packetBuffer.data, sock->sendMessage, dataLen);

	sock->sendNext = false;

	if (sfunc.Write (sock->socket, (byte *)&packetBuffer, packetLen, &sock->addr) == -1)
		return -1;

	sock->lastSendTime = net_time;
	packetsReSent++;

	return 1;
}


cbool Datagram_CanSendMessage (qsocket_t *sock)
{
	if (sock->sendNext)
		SendMessageNext (sock);

	return sock->canSend;
}


cbool Datagram_CanSendUnreliableMessage (qsocket_t *sock)
{
	return true;
}


int Datagram_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data)
{
	int 	packetLen;

#ifdef DEBUG
	if (data->cursize == 0)
		System_Error("Datagram_SendUnreliableMessage: zero length message");

	if (data->cursize > MAX_MARK_V_DATAGRAM)
		System_Error("Datagram_SendUnreliableMessage: message too big %u", data->cursize);
#endif

	packetLen = NET_HEADERSIZE + data->cursize;

	packetBuffer.length = BigLong(packetLen | NETFLAG_UNRELIABLE);
	packetBuffer.sequence = BigLong(sock->unreliableSendSequence++);
	memcpy (packetBuffer.data, data->data, data->cursize);

	if (sfunc.Write (sock->socket, (byte *)&packetBuffer, packetLen, &sock->addr) == -1)
		return -1;

	packetsSent++;
	return 1;
}


int	Datagram_GetMessage (qsocket_t *sock)
{
	unsigned int	length;
	unsigned int	flags;
	int				ret = 0;
	struct qsockaddr readaddr;
	unsigned int	sequence;
	unsigned int	count;

	if (!sock->canSend)
		if ((net_time - sock->lastSendTime) > 1.0)
			ReSendMessage (sock);

	while(1)
	{
		length = (unsigned int)sfunc.Read (sock->socket, (byte *)&packetBuffer,
#ifdef SUPPORTS_SERVER_PROTOCOL_15
			host_protocol_datagram_maxsize, &readaddr);
#endif // SUPPORTS_SERVER_PROTOCOL_15

//	if ((rand() & 255) > 220)
//		continue;

		if (length == 0)
			break;

		if (length == (unsigned int)-1)
		{
			Con_Printf ("Datagram_GetMessage: Read error\n");
			return -1;
		}

		// ProQuake opens a new sock so the address changes
		if (sfunc.AddrCompare(&readaddr, &sock->addr) != 0)
		{
			Con_Printf("Forged packet received\n");
			Con_Printf("Expected: %s\n", StrAddr (&sock->addr));
			Con_Printf("Received: %s\n", StrAddr (&readaddr));
			continue;
		}

		if (length < NET_HEADERSIZE)
		{
			shortPacketCount++;
			continue;
		}

		length = BigLong(packetBuffer.length);
		flags = length & (~NETFLAG_LENGTH_MASK);
		length &= NETFLAG_LENGTH_MASK;

//#ifdef SUPPORTS_NETWORK_FIX // Baker change +
		// From ProQuake:  fix for attack that crashes server
		if (length > NET_MARK_V_DATAGRAMSIZE)
		{
			Con_Printf ("Datagram_GetMessage: Invalid length\n");
			return -1;
		}
//#endif // Baker change +

		if (flags & NETFLAG_CTL)
			continue;

		sequence = BigLong(packetBuffer.sequence);
		packetsReceived++;

		if (flags & NETFLAG_UNRELIABLE)
		{
			if (sequence < sock->unreliableReceiveSequence)
			{
				Con_DPrintf("Got a stale datagram\n");
				ret = 0;
				break;
			}
			if (sequence != sock->unreliableReceiveSequence)
			{
				count = sequence - sock->unreliableReceiveSequence;
				droppedDatagrams += count;
				Con_DPrintf("Dropped %u datagram(s)\n", count);
			}
			sock->unreliableReceiveSequence = sequence + 1;

			length -= NET_HEADERSIZE;

			SZ_Clear (&net_message);
			SZ_Write (&net_message, packetBuffer.data, length);

			ret = 2;
			break;
		}

		if (flags & NETFLAG_ACK)
		{
			if (sequence != (sock->sendSequence - 1))
			{
				Con_DPrintf("Stale ACK received\n");
				continue;
			}
			if (sequence == sock->ackSequence)
			{
				sock->ackSequence++;
				if (sock->ackSequence != sock->sendSequence)
					Con_DPrintf("ack sequencing error\n");
			}
			else
			{
				Con_DPrintf("Duplicate ACK received\n");
				continue;
			}
#ifdef SUPPORTS_SERVER_PROTOCOL_15
			sock->sendMessageLength -= host_protocol_datagram_maxsize;
#endif // SUPPORTS_SERVER_PROTOCOL_15
			if (sock->sendMessageLength > 0)
			{
#ifdef SUPPORTS_SERVER_PROTOCOL_15
				memmove (sock->sendMessage, sock->sendMessage + host_protocol_datagram_maxsize, sock->sendMessageLength);
#endif // SUPPORTS_SERVER_PROTOCOL_15
				sock->sendNext = true;
			}
			else
			{
				sock->sendMessageLength = 0;
				sock->canSend = true;
			}
			continue;
		}

		if (flags & NETFLAG_DATA)
		{
			packetBuffer.length = BigLong(NET_HEADERSIZE | NETFLAG_ACK);
			packetBuffer.sequence = BigLong(sequence);
			sfunc.Write (sock->socket, (byte *)&packetBuffer, NET_HEADERSIZE, &readaddr);

			if (sequence != sock->receiveSequence)
			{
				receivedDuplicateCount++;
				continue;
			}
			sock->receiveSequence++;

			length -= NET_HEADERSIZE;

			if (flags & NETFLAG_EOM)
			{
				SZ_Clear(&net_message);
				SZ_Write(&net_message, sock->receiveMessage, sock->receiveMessageLength);
				SZ_Write(&net_message, packetBuffer.data, length);
				sock->receiveMessageLength = 0;

				ret = 1;
				break;
			}

			memcpy (sock->receiveMessage + sock->receiveMessageLength, packetBuffer.data, length);
			sock->receiveMessageLength += length;
			continue;
		}
	}

	if (sock->sendNext)
		SendMessageNext (sock);

	return ret;
}


static void PrintStats(qsocket_t *s)
{
	Con_Printf("canSend = %4u   \n", s->canSend);
	Con_Printf("sendSeq = %4u   ", s->sendSequence);
	Con_Printf("recvSeq = %4u   \n", s->receiveSequence);
	Con_Printf("\n");
}

void NET_Stats_f (lparse_t *line)
{
	qsocket_t	*s;

	if (line->count == 1)
	{
		Con_Printf("unreliable messages sent   = %i\n", unreliableMessagesSent);
		Con_Printf("unreliable messages recv   = %i\n", unreliableMessagesReceived);
		Con_Printf("reliable messages sent     = %i\n", messagesSent);
		Con_Printf("reliable messages received = %i\n", messagesReceived);
		Con_Printf("packetsSent                = %i\n", packetsSent);
		Con_Printf("packetsReSent              = %i\n", packetsReSent);
		Con_Printf("packetsReceived            = %i\n", packetsReceived);
		Con_Printf("receivedDuplicateCount     = %i\n", receivedDuplicateCount);
		Con_Printf("shortPacketCount           = %i\n", shortPacketCount);
		Con_Printf("droppedDatagrams           = %i\n", droppedDatagrams);
	}
	else if (strcmp(line->args[1], "*") == 0)
	{
		for (s = net_activeSockets; s; s = s->next)
			PrintStats(s);
		for (s = net_freeSockets; s; s = s->next)
			PrintStats(s);
	}
	else
	{
		for (s = net_activeSockets; s; s = s->next)
		{
			if (strcasecmp (line->args[1], s->address) == 0)
				break;
		}

		if (s == NULL)
		{
			for (s = net_freeSockets; s; s = s->next)
			{
				if (strcasecmp (line->args[1], s->address) == 0)
					break;
			}
		}

		if (s == NULL)
			return;

		PrintStats(s);
	}
}


// recognize ip:port (based on ProQuake)
static const char *Strip_Port (const char *host)
{
	static char	noport[MAX_QPATH];
			/* array size as in Host_Connect_f() */
	char		*p;
	int		port;

	if (!host || !*host)
		return host;
	c_strlcpy (noport, host);
	if ((p = strrchr(noport, ':')) == NULL)
		return host;
	*p++ = '\0';
	port = atoi (p);
	if (port > 0 && port < 65536 && port != net_hostport)
	{
		net_hostport = port;
		Con_Printf("Port set to %d\n", net_hostport);
	}
	return noport;
}


static cbool testInProgress = false;
static int		testPollCount;
static int		testDriver;
static sys_socket_t		testSocket;

static void Test_Poll(void *);
static PollProcedure	testPollProcedure = {NULL, 0.0, Test_Poll};

static void Test_Poll(void *unused)
{
	struct qsockaddr clientaddr;
	int		control;
	int		len;
	char	name[32];
	char	address[64];
	int		colors;
	int		frags;
	int		connectTime;
	byte	playerNumber;

	net_landriverlevel = testDriver;

	while (1)
	{
		len = dfunc.Read (testSocket, net_message.data, net_message.maxsize, &clientaddr);
		if (len < (int)sizeof(int))
			break;

		net_message.cursize = len;

		MSG_BeginReading ();
		control = BigLong(*((int *)net_message.data));
		MSG_ReadLong();
		if (control == -1)
			break;
		if ((control & (~NETFLAG_LENGTH_MASK)) !=  (int) NETFLAG_CTL)
			break;
		if ((control & NETFLAG_LENGTH_MASK) != len)
			break;

		if (MSG_ReadByte() != CCREP_PLAYER_INFO)
		{
			Con_Printf("Unexpected repsonse to Player Info request\n");
			break;
		}

		playerNumber = MSG_ReadByte();
		c_strlcpy (name, MSG_ReadString());
		colors = MSG_ReadLong();
		frags = MSG_ReadLong();
		connectTime = MSG_ReadLong();
		c_strlcpy (address, MSG_ReadString());

		Con_Printf("%s\n  frags:%3i  colors:%d %d  time:%d\n  %s\n", name, frags, colors >> 4, colors & 0x0f, connectTime / 60, address);
	}

	testPollCount--;
	if (testPollCount)
	{
		SchedulePollProcedure(&testPollProcedure, 0.1);
	}
	else
	{
		dfunc.Close_Socket (testSocket);
		testInProgress = false;
	}
}

void Test_f (lparse_t *line)
{
	const char	*host;
	int		n;
	int		maxusers = MAX_SCOREBOARD;
	struct qsockaddr sendaddr;

	if (testInProgress)
	{
		Con_Printf ("There is already a test/rcon in progress\n");
		return;
	}

	if (line->count < 2)
	{
		Con_Printf ("Usage: test <host>\n");
		return;
	}

	host = Strip_Port (line->args[1]);

	if (host && hostCacheCount)
	{
		for (n = 0; n < hostCacheCount; n++)
		{
			if (strcasecmp (host, hostcache[n].name) == 0)
			{
				if (hostcache[n].driver != myDriverLevel)
					continue;
				net_landriverlevel = hostcache[n].ldriver;
				maxusers = hostcache[n].maxusers;
				memcpy (&sendaddr, &hostcache[n].addr, sizeof(struct qsockaddr));
				break;
			}
		}
		if (n < hostCacheCount)
			goto JustDoIt;
	}

	for (net_landriverlevel = 0; net_landriverlevel < net_numlandrivers; net_landriverlevel++)
	{
		if (!net_landrivers[net_landriverlevel].initialized)
			continue;

		// see if we can resolve the host name
		if (dfunc.GetAddrFromName(host, &sendaddr) != -1)
			break;
	}

	if (net_landriverlevel == net_numlandrivers)
	{
		Con_Printf("Could not resolve %s\n", host); 	// JPG 3.00 - added error message
		return;
	}

JustDoIt:
	testSocket = dfunc.Open_Socket(0);
	if (testSocket == INVALID_SOCKET)
	{
		Con_Printf("Could not open socket\n");  // JPG 3.00 - added error message
		return;
	}

	testInProgress = true;
	testPollCount = 20;
	testDriver = net_landriverlevel;

	for (n = 0; n < maxusers; n++)
	{
		SZ_Clear(&net_message);
		// save space for the header, filled in later
		MSG_WriteLong(&net_message, 0);
		MSG_WriteByte(&net_message, CCREQ_PLAYER_INFO);
		MSG_WriteByte(&net_message, n);
		*((int *)net_message.data) = BigLong(NETFLAG_CTL | 	(net_message.cursize & NETFLAG_LENGTH_MASK));
		dfunc.Write (testSocket, net_message.data, net_message.cursize, &sendaddr);
	}
	SZ_Clear(&net_message);
	SchedulePollProcedure(&testPollProcedure, 0.1);
}

/* JPG 3.00 - got rid of these.  Just use test vars; only ONE outstanding test of any kind.
static cbool test2InProgress = false;
static int		test2Driver;
static sys_socket_t		test2Socket;
*/

static void Test2_Poll (void *);
static PollProcedure	test2PollProcedure = {NULL, 0.0, Test2_Poll};

static void Test2_Poll (void *unused)
{
	struct qsockaddr clientaddr;
	int		control;
	int		len;
	char	name[256];
	char	value[256];

	net_landriverlevel = testDriver;
	name[0] = 0;

	len = dfunc.Read (testSocket, net_message.data, net_message.maxsize, &clientaddr);
	if (len < (int) sizeof(int))
		goto Reschedule;

	net_message.cursize = len;

	MSG_BeginReading ();
	control = BigLong(*((int *)net_message.data));
	MSG_ReadLong();
	if (control == -1)
		goto Error;
	if ((control & (~NETFLAG_LENGTH_MASK)) !=  (int) NETFLAG_CTL)
		goto Error;
	if ((control & NETFLAG_LENGTH_MASK) != len)
		goto Error;

	if (MSG_ReadByte() != CCREP_RULE_INFO)
		goto Error;

	c_strlcpy (name, MSG_ReadString());
	if (name[0] == 0)
		goto Done;
	c_strlcpy (value, MSG_ReadString());

	Con_Printf("%-16.16s  %-16.16s\n", name, value);

	SZ_Clear(&net_message);
	// save space for the header, filled in later
	MSG_WriteLong(&net_message, 0);
	MSG_WriteByte(&net_message, CCREQ_RULE_INFO);
	MSG_WriteString(&net_message, name);
	*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
	dfunc.Write (testSocket, net_message.data, net_message.cursize, &clientaddr);
	SZ_Clear(&net_message);

Reschedule:
	// JPG 3.00 - added poll counter
	testPollCount--;
	if (testPollCount)
	{
		SchedulePollProcedure(&test2PollProcedure, 0.05);
		return;
	}
	goto Done;

Error:
	Con_Printf("Unexpected response to Rule Info request\n");

Done:
	dfunc.Close_Socket (testSocket);
	testInProgress = false;
	return;
}

void Test2_f (lparse_t *line)
{
	const char	*host;
	int		n;
	struct qsockaddr sendaddr;

	if (testInProgress)
	{
		Con_Printf("There is already a test/rcon in progress\n");
		return;
	}

	if (line->count < 2)
	{
		Con_Printf ("Usage: test2 <host>\n");
		return;
	}

	host = Strip_Port (line->args[1]);

	if (host && hostCacheCount)
	{
		for (n = 0; n < hostCacheCount; n++)
		{
			if (strcasecmp (host, hostcache[n].name) == 0)
			{
				if (hostcache[n].driver != myDriverLevel)
					continue;
				net_landriverlevel = hostcache[n].ldriver;
				memcpy (&sendaddr, &hostcache[n].addr, sizeof(struct qsockaddr));
				break;
			}
		}

		if (n < hostCacheCount)
			goto JustDoIt;
	}

	for (net_landriverlevel = 0; net_landriverlevel < net_numlandrivers; net_landriverlevel++)
	{
		if (!net_landrivers[net_landriverlevel].initialized)
			continue;

		// see if we can resolve the host name
		if (dfunc.GetAddrFromName(host, &sendaddr) != -1)
			break;
	}

	if (net_landriverlevel == net_numlandrivers)
	{
		Con_Printf("Could not resolve %s\n", host);	// JPG 3.00 - added error message
		return;
	}

JustDoIt:
	testSocket = dfunc.Open_Socket (0);
	if (testSocket == INVALID_SOCKET)
	{
		Con_Printf("Could not open socket\n"); // JPG 3.00 - added error message
		return;
	}

	testInProgress = true;				// JPG 3.00 test2InProgress->testInProgress
	testPollCount = 20;					// JPG 3.00 added this
	testDriver = net_landriverlevel;	// JPG 3.00 test2Driver->testDriver

	SZ_Clear(&net_message);
	// save space for the header, filled in later
	MSG_WriteLong(&net_message, 0);
	MSG_WriteByte(&net_message, CCREQ_RULE_INFO);
	MSG_WriteString(&net_message, "");
	*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
	dfunc.Write (testSocket, net_message.data, net_message.cursize, &sendaddr);
	SZ_Clear(&net_message);
	SchedulePollProcedure(&test2PollProcedure, 0.05);
}


static void Rcon_Poll (void *);
PollProcedure	rconPollProcedure = {NULL, 0.0, Rcon_Poll};

static void Rcon_Poll (void* unused)
{
	struct qsockaddr clientaddr;
	int		control, len;

	net_landriverlevel = testDriver;

	len = dfunc.Read (testSocket, net_message.data, net_message.maxsize, &clientaddr);

	if (len < sizeof(int))
	{
		testPollCount--;
		if (testPollCount)
		{
			SchedulePollProcedure(&rconPollProcedure, 0.25);
			return;
		}
		Con_Printf("rcon: no response\n");
		goto Done;
	}

	net_message.cursize = len;

	MSG_BeginReading ();
	control = BigLong(*((int *)net_message.data));
	MSG_ReadLong();
	if (control == -1)
		goto Error;
	if ((control & (~NETFLAG_LENGTH_MASK)) !=  NETFLAG_CTL)
		goto Error;
	if ((control & NETFLAG_LENGTH_MASK) != len)
		goto Error;

	if (MSG_ReadByte() != CCREP_RCON)
		goto Error;

	Con_Printf ("%s\n", MSG_ReadString());

	goto Done;

Error:
	Con_Printf("Unexpected response to rcon command\n");

Done:
	dfunc.Close_Socket(testSocket);
	testInProgress = false;
	return;
}

// JPG 3.02 - rcon
extern cvar_t rcon_password;
extern cvar_t rcon_server;
extern char server_name[MAX_QPATH];

void Rcon_f (lparse_t *line)
{
	const char	*host;
	int		n;
	struct qsockaddr sendaddr;
	size_t offsetz = line->args[1] - line->chopped; // arg1 and beyond, skipping "rcon" command
	char *cmd_after_whitespace = &line->original[offsetz];

	// Baker: A server shouldn't be sending rcon commands
	if (cmd_from_server)
		Con_Warning ("Server has sent an rcon command\n"); 

	if (testInProgress)
	{
		Con_Printf("There is already a test/rcon in progress\n");
		return;
	}

	if (line->count < 2)
	{
		Con_Printf("usage: rcon <command>\n");
		return;
	}

	if (!*rcon_password.string)
	{
		Con_Printf("rcon_password has not been set\n");
		return;
	}

	host = rcon_server.string;

	if (!*rcon_server.string)
	{
		// JPG 3.50 - use current server
		if (cls.state == ca_connected) 
		{
			// Baker: This is dangerous and has to go.  You are giving just any server the rcon password.
			// Someone can setup an evil server and intercept this.
			host = server_name;
		}
		else
		{
			Con_Printf("rcon_server has not been set\n");
			return;
		}
	}

	Strip_Port(host);

	if (host && hostCacheCount)
	{
		for (n = 0; n < hostCacheCount; n++)
			if (strcasecmp (host, hostcache[n].name) == 0)
			{
				if (hostcache[n].driver != myDriverLevel)
					continue;
				net_landriverlevel = hostcache[n].ldriver;
				memcpy(&sendaddr, &hostcache[n].addr, sizeof(struct qsockaddr));
				break;
			}
		if (n < hostCacheCount)
			goto JustDoIt;
	}

	for (net_landriverlevel = 0; net_landriverlevel < net_numlandrivers; net_landriverlevel++)
	{
		if (!net_landrivers[net_landriverlevel].initialized)
			continue;

		// see if we can resolve the host name
		if (dfunc.GetAddrFromName(host, &sendaddr) != -1)
			break;
	}
	if (net_landriverlevel == net_numlandrivers)
	{
		Con_Printf("Could not resolve %s\n", host);
		return;
	}

JustDoIt:
	testSocket = dfunc.Open_Socket(0);
	if (testSocket == -1)
	{
		Con_Printf("Could not open socket\n");
		return;
	}

	testInProgress = true;
	testPollCount = 20;
	testDriver = net_landriverlevel;

	SZ_Clear(&net_message);
	// save space for the header, filled in later
	MSG_WriteLong(&net_message, 0);
	MSG_WriteByte(&net_message, CCREQ_RCON);
	MSG_WriteString(&net_message, rcon_password.string);
	MSG_WriteString(&net_message, cmd_after_whitespace);
	*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
	dfunc.Write (testSocket, net_message.data, net_message.cursize, &sendaddr);
	SZ_Clear(&net_message);
	SchedulePollProcedure(&rconPollProcedure, 0.05);
}


int Datagram_Init (void)
{
	int	i, num_inited;
	sys_socket_t csock;

#ifdef BAN_TEST
	banAddr.s_addr = INADDR_ANY;
	banMask.s_addr = INADDR_NONE;
#endif
	myDriverLevel = net_driverlevel;

	if (COM_CheckParm("-nolan"))
		return -1;

	num_inited = 0;
	for (i = 0; i < net_numlandrivers; i++)
		{
		csock = net_landrivers[i].Init ();
		if (csock == INVALID_SOCKET)
			continue;
		net_landrivers[i].initialized = true;
		net_landrivers[i].controlSock = csock;
		num_inited++;
		}

	if (num_inited == 0)
		return -1;

	return 0;
}


void Datagram_Shutdown (void)
{
	int i;

// shutdown the lan drivers
	for (i = 0; i < net_numlandrivers; i++)
	{
		if (net_landrivers[i].initialized)
		{
			net_landrivers[i].Shutdown ();
			net_landrivers[i].initialized = false;
		}
	}
}


void Datagram_Close (qsocket_t *sock)
{
	sfunc.Close_Socket (sock->socket);
}


void Datagram_Listen (cbool state)
{
	int i;

	for (i = 0; i < net_numlandrivers; i++)
	{
		if (net_landrivers[i].initialized)
			net_landrivers[i].Listen (state);
	}
}

// JPG 3.00 - this code appears multiple times, so factor it out
qsocket_t *Datagram_Reject (const char *message, sys_socket_t acceptsock, struct qsockaddr *addr)
{
	SZ_Clear(&net_message);
	// save space for the header, filled in later
	MSG_WriteLong(&net_message, 0);
	MSG_WriteByte(&net_message, CCREP_REJECT);
	MSG_WriteString(&net_message, message);
	*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
	dfunc.Write (acceptsock, net_message.data, net_message.cursize, addr);
	SZ_Clear(&net_message);

	return NULL;
}

extern cvar_t pq_password;			// JPG 3.00 - password protection
extern unsigned long qsmackAddr;	// JPG 3.02 - allow qsmack bots to connect to server
#ifdef SUPPORTS_PQ_RCON_FAILURE_BLACKOUT // Baker change 
typedef struct
{
	char	ip_address[22];
	float	when;
	int		count;
} rcon_fail_t;

rcon_fail_t rcon_ips_fails[100];
const int num_rcon_ips_fails = sizeof(rcon_ips_fails) / sizeof(rcon_ips_fails[0]);
int rcon_cursor;

cbool Rcon_Blackout (const char* address, float nowtime)
{
	int i;
	
	for (i = 0; i < num_rcon_ips_fails; i ++)
	{
		if (rcon_ips_fails[i].ip_address[0] == 0)
			continue; // Unused slot
		else
		{
			rcon_fail_t* slot = &rcon_ips_fails[i];
			if (strcmp(slot->ip_address, address) == 0) // found
				if (slot->count == 0 && realtime < slot->when + 300)
				{
					Con_Printf ("Slot %i has rcon black out until %f (remaining is %f)\n", i, slot->when + 300, realtime - (slot->when + 300));
					return true;
				}
				else break;
		}
	}

	return false;
}

void Rcon_Fails_Log (const char* address, float nowtime)
{
	cbool found = false;
	float oldest_time = nowtime;
	int i, empty = -1, oldest = -1; 
	
	// Find either this ip address or empty slot.
	for (i = 0; i < num_rcon_ips_fails; i ++)
	{
		if (rcon_ips_fails[i].ip_address[0] == 0)
		{
			if  (empty == -1)
			{
				empty = i;
				break;
			}
			else continue;
		}
		else if (rcon_ips_fails[i].ip_address[0] != 0 && strcmp(rcon_ips_fails[i].ip_address, address) == 0)
		{
			found = true;
			break;
		}
		else if (rcon_ips_fails[i].when <= oldest_time)
		{
			oldest_time = rcon_ips_fails[i].when;
			oldest = i;
		}
	}

	if (found == false)
	{
		// Use empty slot or oldest slot
		int myslot = empty >=0 ? empty : oldest;
		rcon_fail_t* slot = &rcon_ips_fails[myslot];
		c_strlcpy (slot->ip_address, address);
		slot->count = 1;
		slot->when = nowtime;
		Con_Printf ("Rcon failure recorded to new slot %i\n", myslot);
	}
	else
	{
		rcon_fail_t* slot = &rcon_ips_fails[i];
		c_strlcpy (slot->ip_address, address);
		slot->count ++;
		slot->when = nowtime;

		if (slot->count > 3)
			slot->count = 0; // Black out

		Con_Printf ("Rcon failure  count for existing slot of %i is count = %i\n", i, slot->count);

	}

}
#include <time.h>
#endif // Baker change + SUPPORTS_PQ_RCON_FAILURE_BLACKOUT

static qsocket_t *_Datagram_CheckNewConnections (void)
{
	struct qsockaddr clientaddr;
	struct qsockaddr newaddr;
	sys_socket_t			newsock;
	sys_socket_t			acceptsock;
	qsocket_t	*sock;
	qsocket_t	*s;
	int			len;
	int			command;
	int			control;
	int			ret;
	const char *ipstring;

	byte		cl_proquake_connection, cl_proquake_version, cl_proquake_flags;	// JPG 3.02 - bugfix!


	acceptsock = dfunc.CheckNewConnections();
	if (acceptsock == INVALID_SOCKET)
		return NULL;

	SZ_Clear(&net_message);

	len = dfunc.Read (acceptsock, net_message.data, net_message.maxsize, &clientaddr);
	if (len < (int) sizeof(int))
		return NULL;
	net_message.cursize = len;

	MSG_BeginReading ();
	control = BigLong(*((int *)net_message.data));
	MSG_ReadLong();
	if (control == -1)
		return NULL;
	if ((control & (~NETFLAG_LENGTH_MASK)) !=  (int)NETFLAG_CTL)
		return NULL;
	if ((control & NETFLAG_LENGTH_MASK) != len)
		return NULL;

	command = MSG_ReadByte();
	if (command == CCREQ_SERVER_INFO)
	{
		if (strcmp(MSG_ReadString(), "QUAKE") != 0)
			return NULL;

		SZ_Clear(&net_message);
		// save space for the header, filled in later
		MSG_WriteLong(&net_message, 0);
		MSG_WriteByte(&net_message, CCREP_SERVER_INFO);
		dfunc.GetSocketAddr(acceptsock, &newaddr);
		MSG_WriteString(&net_message, dfunc.AddrToString(&newaddr));
		MSG_WriteString(&net_message, hostname.string);
		MSG_WriteString(&net_message, sv.name);
		MSG_WriteByte(&net_message, net_activeconnections);
		MSG_WriteByte(&net_message, svs.maxclients);
		MSG_WriteByte(&net_message, NET_PROTOCOL_VERSION);
		*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
		dfunc.Write (acceptsock, net_message.data, net_message.cursize, &clientaddr);
		SZ_Clear(&net_message);

		return NULL;
	}

	if (command == CCREQ_PLAYER_INFO)
	{
		int			playerNumber, activeNumber, clientNumber;
		int         a, b, c;
		char        address[16];
		const char*	name_display;

		client_t	*client;

		playerNumber = MSG_ReadByte();
		activeNumber = -1;

		for (clientNumber = 0, client = svs.clients; clientNumber < svs.maxclients; clientNumber++, client++)
		{
			if (client->active)
			{
				activeNumber++;
				if (activeNumber == playerNumber)
					break;
			}
		}

		if (clientNumber == svs.maxclients)
			return NULL;

		SZ_Clear(&net_message);
		// save space for the header, filled in later
		MSG_WriteLong(&net_message, 0);
		MSG_WriteByte(&net_message, CCREP_PLAYER_INFO);
		MSG_WriteByte(&net_message, playerNumber);

		// If name privacy set, external requests (test command) send "private"
		if (pq_privacy_name.value)
			name_display = "private";
		else name_display = client->name;

		MSG_WriteString(&net_message, name_display);
		MSG_WriteLong(&net_message, client->colors);
		MSG_WriteLong(&net_message, (int)client->edict->v.frags);
		MSG_WriteLong(&net_message, (int)(net_time - client->netconnection->connecttime));

		switch ((int)pq_privacy_ipmasking.value)
		{
		case 0:		MSG_WriteString(&net_message, client->netconnection->address); break; // Unfiltered


		case 1: 	// Masked
					if (sscanf(client->netconnection->address, "%d.%d.%d", &a, &b, &c) == 3)
					{
						c_snprintf3 (address, "%d.%d.%d.xxx", a, b, c);
						MSG_WriteString(&net_message, address); break; // Private
						break;
					}
					// Fall through if couldn't do that somehow ...

		default:	MSG_WriteString(&net_message, "private"); break; // Private
		}

		*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
		dfunc.Write (acceptsock, net_message.data, net_message.cursize, &clientaddr);
		SZ_Clear(&net_message);

		return NULL;
	}

	if (command == CCREQ_RULE_INFO)
	{
		const char	*prevCvarName;
		cvar_t	*var;

		// find the search start location
		prevCvarName = MSG_ReadString();
		var = Cvar_FindAfter (prevCvarName, CVAR_SERVERINFO);

		// send the response
		SZ_Clear(&net_message);
		// save space for the header, filled in later
		MSG_WriteLong(&net_message, 0);
		MSG_WriteByte(&net_message, CCREP_RULE_INFO);
		if (var)
		{
			MSG_WriteString(&net_message, var->name);
			MSG_WriteString(&net_message, var->string);
		}
		*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
		dfunc.Write (acceptsock, net_message.data, net_message.cursize, &clientaddr);
		SZ_Clear(&net_message);

		return NULL;
	}

	// JPG 3.00 - rcon
	if (command == CCREQ_RCON)
	{
		char pass[2048];	// 2048 = largest possible return from MSG_ReadString
		char cmd[2048];		// 2048 = largest possible return from MSG_ReadString

#ifdef SUPPORTS_PQ_RCON_FAILURE_BLACKOUT // Baker change
		char rcon_client_ip[22], *colon;
		float attempt_time = Time_Now ();

		c_strlcpy (rcon_client_ip, dfunc.AddrToString(&clientaddr));
		if ( (colon = strchr(rcon_client_ip, ':')) ) // Null terminate at colon
			*colon = 0;

#endif // Baker change + SUPPORTS_PQ_RCON_FAILURE_BLACKOUT

		c_strlcpy (pass, MSG_ReadString());
		c_strlcpy (cmd, MSG_ReadString());

		SZ_Clear(&rcon_message);
		// save space for the header, filled in later
		MSG_WriteLong(&rcon_message, 0);
		MSG_WriteByte(&rcon_message, CCREP_RCON);

#ifdef SUPPORTS_PQ_RCON_FAILURE_BLACKOUT // Baker change
		if (Rcon_Blackout (rcon_client_ip, realtime))
			MSG_WriteString(&rcon_message, "rcon ignored: too many failures, wait several minutes and try again");
		else 
#endif // Baker change + SUPPORTS_PQ_RCON_FAILURE_BLACKOUT

		if (!*rcon_password.string)
			MSG_WriteString(&rcon_message, "rcon is disabled on this server");
		else if (strcmp(pass, rcon_password.string))
		{
			time_t  ltime;
			time (&ltime);

#ifdef SUPPORTS_PQ_RCON_ATTEMPTS_LOGGED // Baker change
			Rcon_Fails_Log (rcon_client_ip, realtime);
			MSG_WriteString(&rcon_message, "rcon incorrect password (attempt logged with ip)");
			Con_Printf("(%s) rcon invalid password on \"%s\" %s\n", rcon_client_ip, cmd, ctime( &ltime ) ); //(%s) %s", host_client->netconnection->address,  &text[1]);
#endif // Baker change + SUPPORTS_RCON_ATTEMPTS_LOGGED
		}
		else
		{
#ifdef SUPPORTS_RCON_ATTEMPTS_LOGGED // Baker change
			MSG_WriteString(&rcon_message, "");
			rcon_active = true;
			Con_Printf("(%s) Rcon command: \"%s\" %s\n ", rcon_client_ip, cmd, ctime( &ltime ) ); //(%s) %s", host_client->netconnection->address,  &text[1])
			Cmd_ExecuteString (cmd, src_command);
			rcon_active = false;
#endif // Baker change + SUPPORTS_RCON_ATTEMPTS_LOGGED

		}

		*((int *)rcon_message.data) = BigLong(NETFLAG_CTL | (rcon_message.cursize & NETFLAG_LENGTH_MASK));
		dfunc.Write (acceptsock, rcon_message.data, rcon_message.cursize, &clientaddr);
		SZ_Clear(&rcon_message);

		return NULL;
	}

	if (command != CCREQ_CONNECT)
		return NULL;

	if (strcmp (MSG_ReadString(), "QUAKE") != 0)
		return NULL;

	if (MSG_ReadByte() != NET_PROTOCOL_VERSION)
		return Datagram_Reject("Incompatible version.\n", acceptsock, &clientaddr);

// LOCKED_SERVER
	ipstring = dfunc.AddrToString (&clientaddr);

	if (Admin_Check_ServerLock(ipstring)) // ipstring isn't needed, but if someone is prevented from joining log it
		return Datagram_Reject ("Server isn't accepting new players at the moment.\n", acceptsock, &clientaddr);

#ifdef BAN_TEST
	// check for a ban
	if (Admin_Check_Ban (ipstring) )
		return Datagram_Reject ("You have been banned.\n", acceptsock, &clientaddr);

	if (Admin_Check_Whitelist (ipstring) )
		return Datagram_Reject ("You aren't whitelisted.  If you should be and are very new, try again in a minute.\n", acceptsock, &clientaddr);
#endif

	// see if this guy is already connected
	for (s = net_activeSockets; s; s = s->next)
	{
		if (s->driver != net_driverlevel)
			continue;
		ret = dfunc.AddrCompare(&clientaddr, &s->addr);
		if (ret >= 0)
		{
			// is this a duplicate connection request?
			if (ret == 0 && net_time - s->connecttime < 2.0)
			{
				// yes, so send a duplicate reply
				SZ_Clear(&net_message);
				// save space for the header, filled in later
				MSG_WriteLong(&net_message, 0);
				MSG_WriteByte(&net_message, CCREP_ACCEPT);
				dfunc.GetSocketAddr(s->socket, &newaddr);
				MSG_WriteLong(&net_message, dfunc.GetSocketPort(&newaddr));
				*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
				dfunc.Write (acceptsock, net_message.data, net_message.cursize, &clientaddr);
				SZ_Clear(&net_message);

				return NULL;
			}
			// it's somebody coming back in from a crash/disconnect
			// so close the old qsocket and let their retry get them back in
			//NET_Close(s);  // JPG - finally got rid of the worst mistake in Quake
			//return NULL;
		}
	}

	// Now find out if this is a ProQuake client

	// JPG - support for mods.  Baker: Removed Qsmack check.
	cl_proquake_connection 	= len > 12 ? MSG_ReadByte() : 0;
	cl_proquake_version 	= len > 13 ? MSG_ReadByte() : 0;
	cl_proquake_flags 		= len > 14 ? MSG_ReadByte() : 0;

	// Baker: Skipped cheat-free check
	if (pq_password.value && (len <= 18 || pq_password.value != MSG_ReadLong()))
		return Datagram_Reject("You must use ProQuake v3.1 or above\n(http://www.quakeone.com/proquake) and set pq_password to the server password\n", acceptsock, &clientaddr);

	// allocate a QSocket
	sock = NET_NewQSocket ();
	if (sock == NULL)
		return Datagram_Reject("Server is full.\n", acceptsock, &clientaddr);

	// allocate a network socket
	newsock = dfunc.Open_Socket(0);
	if (newsock == INVALID_SOCKET)
	{
		NET_FreeQSocket(sock);
		return NULL;
	}

	// connect to the client
	if (dfunc.Connect (newsock, &clientaddr) == -1)
	{
		dfunc.Close_Socket(newsock);
		NET_FreeQSocket(sock);
		return NULL;
	}

	// everything is allocated, just fill in the details
	sock->socket = newsock;
	sock->landriver = net_landriverlevel;
	sock->addr = clientaddr;
	c_strlcpy (sock->address, dfunc.AddrToString(&clientaddr));

	// send him back the info about the server connection he has been allocated
	SZ_Clear(&net_message);
	// save space for the header, filled in later
	MSG_WriteLong(&net_message, 0);
	MSG_WriteByte(&net_message, CCREP_ACCEPT);
	dfunc.GetSocketAddr(newsock, &newaddr);
	MSG_WriteLong(&net_message, dfunc.GetSocketPort(&newaddr));
//	MSG_WriteString(&net_message, dfunc.AddrToString(&newaddr));
	*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
	dfunc.Write (acceptsock, net_message.data, net_message.cursize, &clientaddr);
	SZ_Clear(&net_message);

	return sock;
}

qsocket_t *Datagram_CheckNewConnections (void)
{
	qsocket_t *ret = NULL;

	for (net_landriverlevel = 0; net_landriverlevel < net_numlandrivers; net_landriverlevel++)
	{
		if (net_landrivers[net_landriverlevel].initialized)
		{
			if ((ret = _Datagram_CheckNewConnections ()) != NULL)
				break;
		}
	}
	return ret;
}


static void _Datagram_SearchForHosts (cbool xmit)
{
	int		ret;
	int		n;
	int		i;
	struct qsockaddr readaddr;
	struct qsockaddr myaddr;
	int		control;

	dfunc.GetSocketAddr (dfunc.controlSock, &myaddr);
	if (xmit)
	{
		SZ_Clear(&net_message);
		// save space for the header, filled in later
		MSG_WriteLong(&net_message, 0);
		MSG_WriteByte(&net_message, CCREQ_SERVER_INFO);
		MSG_WriteString(&net_message, "QUAKE");
		MSG_WriteByte(&net_message, NET_PROTOCOL_VERSION);
		*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
		dfunc.Broadcast(dfunc.controlSock, net_message.data, net_message.cursize);
		SZ_Clear(&net_message);
	}

	while ((ret = dfunc.Read (dfunc.controlSock, net_message.data, net_message.maxsize, &readaddr)) > 0)
	{
		if (ret < (int) sizeof(int))
			continue;
		net_message.cursize = ret;

//		Con_Printf ("Received reply from %s\n", dfunc.AddrToString (&readaddr));

		// don't answer our own query
		if (dfunc.AddrCompare(&readaddr, &myaddr) >= 0)
			continue;

		// is the cache full?
		if (hostCacheCount == HOSTCACHESIZE)
			continue;

		MSG_BeginReading ();
		control = BigLong(*((int *)net_message.data));
		MSG_ReadLong();
		if (control == -1)
			continue;
		if ((control & (~NETFLAG_LENGTH_MASK)) !=  (int)NETFLAG_CTL)
			continue;
		if ((control & NETFLAG_LENGTH_MASK) != ret)
			continue;

		if (MSG_ReadByte() != CCREP_SERVER_INFO)
			continue;

		dfunc.GetAddrFromName(MSG_ReadString(), &readaddr);

		// search the cache for this server
		for (n = 0; n < hostCacheCount; n++)
		{
			if (dfunc.AddrCompare(&readaddr, &hostcache[n].addr) == 0)
				break;
		}

		// is it already there?
		if (n < hostCacheCount)
			continue;

		// add it
		hostCacheCount++;
		c_strlcpy (hostcache[n].name, MSG_ReadString());
		c_strlcpy (hostcache[n].map, MSG_ReadString());
		hostcache[n].users = MSG_ReadByte();
		hostcache[n].maxusers = MSG_ReadByte();
		if (MSG_ReadByte() != NET_PROTOCOL_VERSION)
		{
			c_strlcpy (hostcache[n].cname, hostcache[n].name);
			hostcache[n].cname[14] = 0;
			c_strlcpy (hostcache[n].name, "*");
			c_strlcat (hostcache[n].name, hostcache[n].cname);
		}
		// Baker: Slist This is where we write the address it.
		memcpy (&hostcache[n].addr, &readaddr, sizeof(struct qsockaddr));
		hostcache[n].driver = net_driverlevel;
		hostcache[n].ldriver = net_landriverlevel;
		c_strlcpy (hostcache[n].cname, dfunc.AddrToString(&readaddr));

		// check for a name conflict
		for (i = 0; i < hostCacheCount; i++)
		{
			if (i == n)
				continue;
			if (strcasecmp (hostcache[n].name, hostcache[i].name) == 0)
			{
				i = strlen(hostcache[n].name);
				if (i < 15 && hostcache[n].name[i-1] > '8')
				{
					hostcache[n].name[i] = '0';
					hostcache[n].name[i+1] = 0;
				}
				else
				{
					hostcache[n].name[i-1]++;
				}
				i = -1;
			}
		}
	}
}

void Datagram_SearchForHosts (cbool xmit)
{
	for (net_landriverlevel = 0; net_landriverlevel < net_numlandrivers; net_landriverlevel++)
	{
		if (hostCacheCount == HOSTCACHESIZE)
			break;
		if (net_landrivers[net_landriverlevel].initialized)
			_Datagram_SearchForHosts (xmit);
	}
}


static qsocket_t *_Datagram_Connect (const char *host)
{
	struct qsockaddr sendaddr;
	struct qsockaddr readaddr;
	qsocket_t	*sock;
	sys_socket_t newsock;
	sys_socket_t clientsock; // ProQuake

	int			len; // ProQuake full NAT connect (2/6)
	int			ret;
	int			reps;
	double		start_time;
	int			control;
	const char	*reason;

	// see if we can resolve the host name
	if (dfunc.GetAddrFromName(host, &sendaddr) == -1)
	{
		Con_Printf("Could not resolve %s\n", host);	// JPG 3.20 - added this
		return NULL;
	}

	newsock = dfunc.Open_Socket (0);
	if (newsock == INVALID_SOCKET)
		return NULL;

	sock = NET_NewQSocket ();
	if (sock == NULL)
		goto ErrorReturn2;

	sock->socket = newsock;
	sock->landriver = net_landriverlevel;

	// connect to the host
	if (dfunc.Connect (newsock, &sendaddr) == -1)
		goto ErrorReturn;

	// send the connection request
	Con_Printf("trying...\n");
	SCR_UpdateScreen ();
	start_time = net_time;

	for (reps = 0; reps < 3; reps++)
	{
		SZ_Clear(&net_message);
		// save space for the header, filled in later
		MSG_WriteLong(&net_message, 0);
		MSG_WriteByte(&net_message, CCREQ_CONNECT);
		MSG_WriteString(&net_message, "QUAKE");
		MSG_WriteByte(&net_message, NET_PROTOCOL_VERSION);

// ProQuake full NAT connect (3/6)
#if 1
#define MOD_PROQUAKE		0x01
#define PROQUAKE_SERIES_VERSION		5.50
		MSG_WriteByte(&net_message, MOD_PROQUAKE);			// JPG - added this
		MSG_WriteByte(&net_message, PROQUAKE_SERIES_VERSION * 10);	// JPG 3.00 - added this
		MSG_WriteByte(&net_message, 0);						// JPG 3.00 - added this (flags)
		MSG_WriteLong(&net_message, 0);		// JPG 3.00 - password protected servers
#endif
		*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
		dfunc.Write (newsock, net_message.data, net_message.cursize, &sendaddr);
		SZ_Clear(&net_message);
		do
		{
			ret = dfunc.Read (newsock, net_message.data, net_message.maxsize, &readaddr);
			// if we got something, validate it
			if (ret > 0)
			{
				// is it from the right place?
				if (sfunc.AddrCompare(&readaddr, &sendaddr) != 0)
				{
					Con_Printf("wrong reply address\n");
					Con_Printf("Expected: %s | %s\n", dfunc.AddrToString (&sendaddr), StrAddr(&sendaddr));
					Con_Printf("Received: %s | %s\n", dfunc.AddrToString (&readaddr), StrAddr(&readaddr));
					SCR_UpdateScreen ();
					ret = 0;
					continue;
				}

				if (ret < (int) sizeof(int))
				{
					ret = 0;
					continue;
				}

				net_message.cursize = ret;
				MSG_BeginReading ();

				control = BigLong(*((int *)net_message.data));
				MSG_ReadLong();
				if (control == -1)
				{
					ret = 0;
					continue;
				}
				if ((control & (~NETFLAG_LENGTH_MASK)) !=  (int)NETFLAG_CTL)
				{
					ret = 0;
					continue;
				}
				if ((control & NETFLAG_LENGTH_MASK) != ret)
				{
					ret = 0;
					continue;
				}
			}
		}
		while (ret == 0 && (SetNetTime() - start_time) < 2.5);

		if (ret)
			break;

		Con_Printf("still trying...\n");
		SCR_UpdateScreen ();
		start_time = SetNetTime();
	}

	if (ret == 0)
	{
		reason = "No Response";
		Con_Printf("%s\n", reason);
		c_strlcpy (m_return_reason, reason);
		goto ErrorReturn;
	}

	if (ret == -1)
	{
		reason = "Network Error";
		Con_Printf("%s\n", reason);
		c_strlcpy (m_return_reason, reason);
		goto ErrorReturn;
	}

	len = ret; // JPG - save length for ProQuake connections

	ret = MSG_ReadByte();
	if (ret == CCREP_REJECT)
	{
		reason = MSG_ReadString();
		Con_Printf("%s\n", reason);
		c_strlcpy (m_return_reason, reason);
		goto ErrorReturn;
	}

	if (ret == CCREP_ACCEPT)
	{
		// Baker: Here is where we read the port that the server assigned us
		int port = MSG_ReadLong();
		memcpy (&sock->addr, &sendaddr, sizeof(struct qsockaddr));
		dfunc.SetSocketPort (&sock->addr, port);

#if 1
		// Client has received CCREP_ACCEPT meaning client may connect
		// Now find out if this is a ProQuake server ...

		sock->proquake_connection = len > 9 ?  MSG_ReadByte() : MOD_NONE;    // JPG - support for mods
		sock->proquake_version = len > 10 ? MSG_ReadByte () : 0;  // JPG 3.00 - version and flags
		sock->proquake_flags = len > 11 ? MSG_ReadByte() : 0;
		// Cheatfree was here ... sock->mod == MOD_PROQUAKE && (sock->mod_flags & PQF_CHEATFREE)
#else
		Con_DPrintf ("Client port on the server is %s\n", dfunc.AddrToString(&sock->addr));

		sock->proquake_connection = (len > 9 && MSG_ReadByte () == 1) ? 1 : 0;
		Con_DPrintf ("Server is ProQuake ? %i\n", sock->proquake_connection);
#endif
	}
	else
	{
		reason = "Bad Response";
		Con_Printf("%s\n", reason);
		c_strlcpy (m_return_reason, reason);
		goto ErrorReturn;
	}

	dfunc.GetNameFromAddr (&sendaddr, sock->address, sizeof(sock->address) );

	Con_Printf ("Connection accepted\n");
	sock->lastMessageTime = SetNetTime();

	// JPG 3.40 - make NAT work by opening a new socket
	if (sock->proquake_connection == MOD_PROQUAKE && sock->proquake_version >= 34)
	{
		clientsock = dfunc.Open_Socket (0);
		if (clientsock == -1)
			goto ErrorReturn;
		dfunc.Close_Socket(newsock);
		newsock = clientsock;
		sock->socket = newsock;
	}

	sock->encrypt = 2;	// JPG 3.50

	// switch the connection to the specified address
	if (dfunc.Connect (newsock, &sock->addr) == -1)
	{
		reason = "Connect to Game failed";
		Con_Printf("%s\n", reason);
		c_strlcpy (m_return_reason, reason);

		goto ErrorReturn;
	}

	m_return_onerror = false;
	return sock;

ErrorReturn:
	NET_FreeQSocket(sock);

ErrorReturn2:
	dfunc.Close_Socket(newsock);
	if (m_return_onerror)
	{
		Key_SetDest (key_menu); m_state = m_return_state; // Baker: A menu keydest needs to know menu item
		m_return_onerror = false;
	}

	return NULL;
}

qsocket_t *Datagram_Connect (const char *host)
{
	qsocket_t *ret = NULL;

	host = Strip_Port (host);
	for (net_landriverlevel = 0; net_landriverlevel < net_numlandrivers; net_landriverlevel++)
	{
		if (net_landrivers[net_landriverlevel].initialized)
		{
			if ((ret = _Datagram_Connect (host)) != NULL)
				break;
		}
	}
	return ret;
}

