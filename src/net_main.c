/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
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

// net_main.c

#include <core.h>
#include "q_stdinc.h"
#include "arch_def.h"
#include "net_sys.h"
#include "quakedef.h"
#include "net_defs.h"

qsocket_t	*net_activeSockets = NULL;
qsocket_t	*net_freeSockets = NULL;
int			net_numsockets = 0;


cbool	tcpipAvailable = false;

int			net_hostport;
int			DEFAULTnet_hostport = 26000;

char		my_tcpip_address[NET_NAMELEN];

cbool	slistInProgress = false;
cbool	slistSilent = false;
cbool	slistLocal = true;
static double	slistStartTime;
static int		slistLastShown;

static void Slist_Send (void *);
static void Slist_Poll (void *);
static PollProcedure	slistSendProcedure = {NULL, 0.0, Slist_Send};
static PollProcedure	slistPollProcedure = {NULL, 0.0, Slist_Poll};

sizebuf_t		net_message;
int				net_activeconnections = 0;

int messagesSent = 0;
int messagesReceived = 0;
int unreliableMessagesSent = 0;
int unreliableMessagesReceived = 0;

// JPG 3.00 - rcon
char		server_name[MAX_QPATH];

#define RCON_BUFF_SIZE_8192	8192
char		rcon_buff[RCON_BUFF_SIZE_8192];
sizebuf_t	rcon_message = {false, false, rcon_buff, RCON_BUFF_SIZE_8192, 0};
cbool		rcon_active = false;


// these two macros are to make the code more readable
#define sfunc	net_drivers[sock->driver]
#define dfunc	net_drivers[net_driverlevel]

int	net_driverlevel;

double			net_time;


double SetNetTime(void)
{
	net_time = System_DoubleTime();
	return net_time;
}


/*
===================
NET_NewQSocket

Called by drivers when a new communications endpoint is required
The sequence and buffer fields will be filled in properly
===================
*/
qsocket_t *NET_NewQSocket (void)
{
	qsocket_t	*sock;

	if (net_freeSockets == NULL)
		return NULL;

	if (net_activeconnections >= svs.maxclients)
		return NULL;

	// get one from free list
	sock = net_freeSockets;
	net_freeSockets = sock->next;

	// add it to active list
	sock->next = net_activeSockets;
	net_activeSockets = sock;

	sock->disconnected = false;
	sock->connecttime = net_time;
	c_strlcpy (sock->address, "UNSET ADDRESS");
	sock->driver = net_driverlevel;
	sock->socket = 0;
	sock->driverdata = NULL;
	sock->canSend = true;
	sock->sendNext = false;
	sock->lastMessageTime = net_time;
	sock->ackSequence = 0;
	sock->sendSequence = 0;
	sock->unreliableSendSequence = 0;
	sock->sendMessageLength = 0;
	sock->receiveSequence = 0;
	sock->unreliableReceiveSequence = 0;
	sock->receiveMessageLength = 0;

	return sock;
}


void NET_FreeQSocket(qsocket_t *sock)
{
	qsocket_t	*s;

	// remove it from active list
	if (sock == net_activeSockets)
		net_activeSockets = net_activeSockets->next;
	else
	{
		for (s = net_activeSockets; s; s = s->next)
		{
			if (s->next == sock)
			{
				s->next = sock->next;
				break;
			}
		}

		if (!s)
			System_Error ("NET_FreeQSocket: not active");
	}

	// add it to free list
	sock->next = net_freeSockets;
	net_freeSockets = sock;
	sock->disconnected = true;
}


double NET_QSocketGetTime (qsocket_t *s)
{
	return s->connecttime;
}


const char *NET_QSocketGetAddressString (qsocket_t *s)
{
	return s->address;
}

#if 1
cbool NET_QSocketIsProQuakeServer (qsocket_t *s)
{
   return s->proquake_connection;
}
#endif

void NET_Listen_f (lparse_t *line)
{
	if (line->count != 2)
	{
		Con_Printf ("\"listen\" is \"%d\"\n", svs.listening ? 1 : 0);
		return;
	}

	svs.listening = atoi(line->args[1]) ? true : false;

	for (net_driverlevel=0 ; net_driverlevel<net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		dfunc.Listen (svs.listening);
	}
}


void MaxPlayers_f (lparse_t *line)
{
	int 	n;

	if (line->count != 2)
	{
		Con_Printf ("\"maxplayers\" is \"%d\"\n", svs.maxclients);
		return;
	}

	if (sv.active)
	{
		Con_Printf ("maxplayers can not be changed while a server is running.\n");
		return;
	}

	n = atoi(line->args[1]);
	if (n < 1)
		n = 1;
	if (n > svs.maxclientslimit)
	{
		n = svs.maxclientslimit;
		Con_Printf ("\"maxplayers\" set to \"%d\"\n", n);
	}

	if ((n == 1) && svs.listening)
		Cbuf_AddText ("listen 0\n");

	if ((n > 1) && (!svs.listening))
		Cbuf_AddText ("listen 1\n");

	svs.maxclients = n;
	if (n == 1)
		Cvar_SetValueQuick (&pr_deathmatch, 0);
	else
		Cvar_SetValueQuick (&pr_deathmatch, 1);
}


void NET_Port_f (lparse_t *line)
{
	int 	n;

	if (line->count != 2)
	{
		Con_Printf ("\"port\" is \"%d\"\n", net_hostport);
		return;
	}

	n = atoi(line->args[1]);
	if (n < 1 || n > 65534)
	{
		Con_Printf ("Bad value, must be between 1 and 65534\n");
		return;
	}

	DEFAULTnet_hostport = n;
	net_hostport = n;

	if (svs.listening)
	{
		// force a change to the new port
		Cbuf_AddText ("listen 0\n");
		Cbuf_AddText ("listen 1\n");
	}
}


static void PrintSlistHeader(void)
{
	Con_Printf("Address               Server          Map             Users\n");
	Con_Printf("--------------------- --------------- --------------- -----\n");
	slistLastShown = 0;
}


static void PrintSlist(void)
{
	int n;

	for (n = slistLastShown; n < hostCacheCount; n++)
	{
		if (hostcache[n].maxusers) // Eek ... UDP specific fix
			Con_Printf("%-21.21s %-15.15s %-15.15s %2u/%2u\n", UDP_AddrToString (&hostcache[n].addr), hostcache[n].name, hostcache[n].map, hostcache[n].users, hostcache[n].maxusers);
		else
			Con_Printf("%-15.15s %-15.15s\n", hostcache[n].name, hostcache[n].map);
	}
	slistLastShown = n;
}


static void PrintSlistTrailer(void)
{
	if (hostCacheCount)
		Con_Printf("== end list ==\n\n");
	else
		Con_Printf("No Quake servers found.\n\n");
}


void NET_Slist_f (lparse_t *unused)
{
	if (slistInProgress)
		return;

	if (! slistSilent)
	{
		Con_Printf("Looking for Quake servers...\n");
		PrintSlistHeader();
	}

	slistInProgress = true;
	slistStartTime = System_DoubleTime();

	SchedulePollProcedure(&slistSendProcedure, 0.0); // net_main.c  Slist_Send Slist_Poll
	SchedulePollProcedure(&slistPollProcedure, 0.1);

	hostCacheCount = 0;
}


void NET_SlistSort (void)
{
	if (hostCacheCount > 1)
	{
		int	i, j;
		hostcache_t temp;
		for (i = 0; i < hostCacheCount; i++)
		{
			for (j = i + 1; j < hostCacheCount; j++)
			{
				if (strcmp(hostcache[j].name, hostcache[i].name) < 0)
				{
					memcpy(&temp, &hostcache[j], sizeof(hostcache_t));
					memcpy(&hostcache[j], &hostcache[i], sizeof(hostcache_t));
					memcpy(&hostcache[i], &temp, sizeof(hostcache_t));
				}
			}
		}
	}
}


const char *NET_SlistPrintServer (int idx)
{
	static char	string[64];

	if (idx < 0 || idx >= hostCacheCount)
		return "";

	if (hostcache[idx].maxusers)
	{
		c_snprintf4 (string, "%-15.15s %-15.15s %2u/%2u\n",
					hostcache[idx].name, hostcache[idx].map,
					hostcache[idx].users, hostcache[idx].maxusers);
	}
	else
	{
		c_snprintf2 (string, "%-15.15s %-15.15s\n",
					hostcache[idx].name, hostcache[idx].map);
	}

	return string;
}


const char *NET_SlistPrintServerName (int idx)
{
	if (idx < 0 || idx >= hostCacheCount)
		return "";
	return hostcache[idx].cname;
}


static void Slist_Send (void *unused)
{
	for (net_driverlevel=0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (!slistLocal && IS_LOOP_DRIVER(net_driverlevel))
			continue;
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		dfunc.SearchForHosts (true);
	}

	if ((System_DoubleTime() - slistStartTime) < 0.5)
		SchedulePollProcedure(&slistSendProcedure, 0.75);
}


static void Slist_Poll (void *unused)
{
	for (net_driverlevel=0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (!slistLocal && IS_LOOP_DRIVER(net_driverlevel))
			continue;
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		dfunc.SearchForHosts (false);
	}

	if (! slistSilent)
		PrintSlist();

	if ((System_DoubleTime() - slistStartTime) < 1.5)
	{
		SchedulePollProcedure(&slistPollProcedure, 0.1);
		return;
	}

	if (! slistSilent)
		PrintSlistTrailer();
	slistInProgress = false;
	slistSilent = false;
	slistLocal = true;
}


/*
===================
NET_Connect
===================
*/

int hostCacheCount = 0;
hostcache_t hostcache[HOSTCACHESIZE];

qsocket_t *NET_Connect (const char *host)
{
	qsocket_t		*ret;
	int				n;
	int				numdrivers = net_numdrivers;

	SetNetTime();

	if (host && *host == 0)
		host = NULL;

	if (host)
	{
		if (strcasecmp (host, "local") == 0)
		{
			numdrivers = 1;
			goto JustDoIt;
		}

		if (hostCacheCount)
		{
			for (n = 0; n < hostCacheCount; n++)
				if (strcasecmp (host, hostcache[n].name) == 0)
				{
					host = hostcache[n].cname;
					break;
				}
			if (n < hostCacheCount)
				goto JustDoIt;
		}
	}

	slistSilent = host ? true : false;
	NET_Slist_f (NULL);

	while(slistInProgress)
		NET_Poll();

	if (host == NULL)
	{
		if (hostCacheCount != 1)
			return NULL;
		host = hostcache[0].cname;
		Con_Printf("Connecting to...\n%s @ %s\n\n", hostcache[0].name, host);
	}

	if (hostCacheCount)
	{
		for (n = 0; n < hostCacheCount; n++)
		{
			if (strcasecmp (host, hostcache[n].name) == 0)
			{
				host = hostcache[n].cname;
				break;
			}
		}
	}

JustDoIt:
	for (net_driverlevel=0 ; net_driverlevel<numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		ret = dfunc.Connect (host);
		if (ret)
			return ret;
	}

	if (host)
	{
		Con_Printf("\n");
		PrintSlistHeader();
		PrintSlist();
		PrintSlistTrailer();
	}

	return NULL;
}


/*
===================
NET_CheckNewConnections
===================
*/
qsocket_t *NET_CheckNewConnections (void)
{
	qsocket_t	*ret;

	Admin_Remote_Update (); // See if we have a finished remote update

	SetNetTime();

	for (net_driverlevel=0 ; net_driverlevel<net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		if (!IS_LOOP_DRIVER(net_driverlevel) && svs.listening == false)
			continue;
		ret = dfunc.CheckNewConnections ();
		if (ret)
		{
			return ret;
		}
	}

	return NULL;
}

/*
===================
NET_Close
===================
*/
void NET_Close (qsocket_t *sock)
{
	if (!sock)
		return;

	if (sock->disconnected)
		return;

	SetNetTime();

	// call the driver_Close function
	sfunc.Close (sock);

	NET_FreeQSocket(sock);
}


/*
=================
NET_GetMessage

If there is a complete message, return it in net_message

returns 0 if no data is waiting
returns 1 if a message was received
returns -1 if connection is invalid
=================
*/
int	NET_GetMessage (qsocket_t *sock)
{
	int ret;

	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		Con_Printf("NET_GetMessage: disconnected socket\n");
		return -1;
	}

	SetNetTime();

	ret = sfunc.QGetMessage(sock);

	// see if this connection has timed out
	if (ret == 0 && !IS_LOOP_DRIVER(sock->driver))
	{
		if (net_time - sock->lastMessageTime > net_messagetimeout.value)
		{
			Con_Printf ("Disconnecting client due to net_messagetimeout %g\n", net_messagetimeout.value);
			NET_Close(sock);
			return -1;
		}
		// From ProQuake: qflood/qkick protection
		if (net_time - sock->lastMessageTime > net_connecttimeout.value && sv.active &&
			host_client && sock == host_client->netconnection && !strcmp(host_client->name, "unconnected"))
		{
			Con_Printf ("Disconnecting client due to net_connecttimeout %g\n", net_connecttimeout.value);
			NET_Close(sock);
			return -1;
		}
	}

	if (ret > 0)
	{
		if (!IS_LOOP_DRIVER(sock->driver))
		{
			sock->lastMessageTime = net_time;
			if (ret == 1)
				messagesReceived++;
			else if (ret == 2)
				unreliableMessagesReceived++;
		}
	}

	return ret;
}


/*
==================
NET_SendMessage

Try to send a complete length+message unit over the reliable stream.
returns 0 if the message cannot be delivered reliably, but the connection
		is still considered valid
returns 1 if the message was sent properly
returns -1 if the connection died
==================
*/
int NET_SendMessage (qsocket_t *sock, sizebuf_t *data)
{
	int		r;

	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		Con_Printf("NET_SendMessage: disconnected socket\n");
		return -1;
	}

	SetNetTime();
	r = sfunc.QSendMessage(sock, data);
	if (r == 1 && !IS_LOOP_DRIVER(sock->driver))
		messagesSent++;

	return r;
}


int NET_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data)
{
	int		r;

	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		Con_Printf("NET_SendMessage: disconnected socket\n");
		return -1;
	}

	SetNetTime();
	r = sfunc.SendUnreliableMessage(sock, data);
	if (r == 1 && !IS_LOOP_DRIVER(sock->driver))
		unreliableMessagesSent++;

	return r;
}


/*
==================
NET_CanSendMessage

Returns true or false if the given qsocket can currently accept a
message to be transmitted.
==================
*/
cbool NET_CanSendMessage (qsocket_t *sock)
{
	if (!sock)
		return false;

	if (sock->disconnected)
		return false;

	SetNetTime();

	return sfunc.CanSendMessage(sock);
}


int NET_SendToAll (sizebuf_t *data, double blocktime)
{
	double		start;
	int			i;
	int			count = 0;
	cbool	msg_init [MAX_SCOREBOARD];  /* did we write the message to the client's connection	*/
	cbool	msg_sent [MAX_SCOREBOARD];  /* did the msg arrive its destination (canSend state).	*/

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->netconnection)
			continue;
		if (host_client->active)
		{
			if (IS_LOOP_DRIVER(host_client->netconnection->driver))
			{
				NET_SendMessage(host_client->netconnection, data);
				msg_init[i] = true;
				msg_sent[i] = true;
				continue;
			}
			count++;
			msg_init[i] = false;
			msg_sent[i] = false;
		}
		else
		{
			msg_init[i] = true;
			msg_sent[i] = true;
		}
	}

	start = System_DoubleTime();
	while (count)
	{
		count = 0;
		for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		{
			if (! msg_init[i])
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					msg_init[i] = true;
					NET_SendMessage(host_client->netconnection, data);
				}
				else
				{
					NET_GetMessage (host_client->netconnection);
				}
				count++;
				continue;
			}

			if (! msg_sent[i])
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					msg_sent[i] = true;
				}
				else
				{
					NET_GetMessage (host_client->netconnection);
				}
				count++;
				continue;
			}
		}
		if ((System_DoubleTime() - start) > blocktime)
			break;
	}
	return count;
}


//=============================================================================

/*
====================
NET_Init
====================
*/

void NET_Init (void)
{
	int			i;
	qsocket_t	*s;

	i = COM_CheckParm ("-port");

	if (i)
	{
		if (i < com_argc-1)
			DEFAULTnet_hostport = atoi (com_argv[i+1]);
		else
			System_Error ("NET_Init: you must specify a number after -port");
	}
	net_hostport = DEFAULTnet_hostport;

	net_numsockets = svs.maxclientslimit;
	if (!isDedicated)
		net_numsockets++;
	if (COM_CheckParm("-listen") || isDedicated)
		svs.listening = true;

	SetNetTime();

	for (i = 0; i < net_numsockets; i++)
	{
		s = (qsocket_t *)Hunk_AllocName(sizeof(qsocket_t), "qsocket");
		s->next = net_freeSockets;
		net_freeSockets = s;
		s->disconnected = true;
	}

	// allocate space for network message buffer
	SZ_Alloc (&net_message, NET_MARK_V_MAXMESSAGE);

	Cmd_AddCommands (NET_Init);

	// initialize all the drivers
	for (i = net_driverlevel=0 ; net_driverlevel<net_numdrivers ; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].Init() == -1)
			continue;
		i++;
		net_drivers[net_driverlevel].initialized = true;
		if (svs.listening)
			net_drivers[net_driverlevel].Listen (true);
	}

	/* Loop_Init() returns -1 for dedicated server case,
	 * therefore the i == 0 check is correct */
	if (i == 0
			&& isDedicated
	   )
	{
		System_Error("Network not available!");
	}

//	if (*my_ipx_address)
//	{
//		Con_DPrintf("IPX address %s\n", "(n/a)");
//	}
	if (*my_tcpip_address)
	{
		Con_DPrintf("TCP/IP address %s\n", my_tcpip_address);
	}
}

/*
====================
NET_Shutdown
====================
*/

void		NET_Shutdown (void)
{
	qsocket_t	*sock;

	SetNetTime();

	for (sock = net_activeSockets; sock; sock = sock->next)
		NET_Close(sock);

//
// shutdown the drivers
//
	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == true)
		{
			net_drivers[net_driverlevel].Shutdown ();
			net_drivers[net_driverlevel].initialized = false;
		}
	}
}


static PollProcedure *pollProcedureList = NULL;

void NET_Poll(void)
{
	PollProcedure *pp;

	SetNetTime();

	for (pp = pollProcedureList; pp; pp = pp->next)
	{
		if (pp->nextTime > net_time)
			break;
		pollProcedureList = pp->next;
		pp->procedure(pp->arg);
	}
}


void SchedulePollProcedure(PollProcedure *proc, double timeOffset)
{
	PollProcedure *pp, *prev;

	proc->nextTime = System_DoubleTime() + timeOffset;
	for (pp = pollProcedureList, prev = NULL; pp; pp = pp->next)
	{
		if (pp->nextTime >= proc->nextTime)
			break;
		prev = pp;
	}

	if (prev == NULL)
	{
		proc->next = pollProcedureList;
		pollProcedureList = proc;
		return;
	}

	proc->next = pp;
	prev->next = proc;
}


