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
// host.c -- coordinates spawning and killing of local servers

#include "quakedef.h"
#include <setjmp.h>

/*

A server can always be started, even if the system started out as a client
to a remote system.

A client can NOT be started if the system started as a dedicated server.

Memory is cleared / released when a server or client begins, not when they end.

*/

build_t build;
host_parms_t host_parms;

fn_set_t qfunction_set =
{
	// Printing
	System_Error, Con_Warning, Con_Printf, Con_DPrintf,

	// Memory
	malloc, calloc, realloc, strdup, free,

	// File IO
	FS_fopen_read, FS_fopen_write_create_path, FS_fclose
};


cbool	host_initialized;		// true if into command execution
cbool	host_post_initialized;	// true if completed initial config execution

double		host_frametime_;
double		realtime;				// without any filtering or bounding
double		oldrealtime;			// last frame run

int			host_framecount;

int			host_hunklevel;

int			minimum_memory;

client_t	*host_client;			// current client

jmp_buf 	host_abortserver;



devstats_t dev_stats, dev_peakstats;
overflowtimes_t dev_overflows; //this stores the last time overflow messages were displayed, not the last time overflows occured

#if 0
/*
================
Max_Edicts_f -- johnfitz
================
*/
void Max_Edicts_f (cvar_t *var)
{
	static float oldval = 1024; //must match the default value for max_edicts

	//TODO: clamp it here?

	if (host_max_edicts.value == oldval)
		return;

	if (cls.state == ca_connected || sv.active)
		Con_Printf ("Changes will not take effect until the next level load.\n");

	oldval = host_max_edicts.value;
}
#endif

/*
================
Host_EndGame
================
*/
void Host_EndGame (const char *message, ...)
{
	VA_EXPAND (string, 1024, message)

	Con_DPrintf ("Host_EndGame: %s\n",string);

	if (sv.active)
		Host_ShutdownServer (false);

	if (isDedicated)
		System_Error ("Host_EndGame: %s",string);	// dedicated servers exit

	if (cls.demonum != -1)
	{
		CL_StopPlayback ();	// JPG 1.05 - patch by CSR to fix crash
		CL_NextDemo ();
	}
	else
	{
#if 1
		SCR_EndLoadingPlaque (); // Baker: any disconnect state should end the loading plague, right?
		if (key_dest == key_game)
			Key_SetDest (key_console);
#endif


		CL_Disconnect ();
	}

	longjmp (host_abortserver, 1);
}

/*
================
Host_Error

This shuts down both the client and server
================
*/
void Host_Error (const char *error, ...)
{
	static	cbool inerror = false;
	VA_EXPAND (string, 1024, error)

	if (inerror)
		System_Error ("Host_Error: recursively entered");
	inerror = true;

	SCR_EndLoadingPlaque ();		// reenable screen updates

	Con_Printf ("Host_Error: %s\n",string);

	// Baker: If host isn't initialized, fall back to System_Error
	if (!host_initialized)
		System_Error ("%s", string);

	if (sv.active)
		Host_ShutdownServer (false);

	if (isDedicated)
		System_Error ("Host_Error: %s\n",string);	// dedicated servers exit

	CL_Disconnect ();
	cls.demonum = -1;
	cl.intermission = 0; //johnfitz -- for errors during intermissions (changelevel with no map found, etc.)

	inerror = false;

	longjmp (host_abortserver, 1);
}

/*
================
Host_FindMaxClients
================
*/
void Host_FindMaxClients (void)
{
	int	cmdline_dedicated;
	int cmdline_listen;

	svs.maxclients = 1;

	cmdline_dedicated = COM_CheckParm ("-dedicated");
	if (cmdline_dedicated)
	{
		cls.state = ca_dedicated;
		if (cmdline_dedicated != (com_argc - 1))
			svs.maxclients = atoi (com_argv[cmdline_dedicated+1]);
		else
			svs.maxclients = 8; // Default for -dedicated with no command line
	}
	else
		cls.state = ca_disconnected;

	cmdline_listen = COM_CheckParm ("-listen");
	if (cmdline_listen)
	{
		if (isDedicated)
			System_Error ("Only one of -dedicated or -listen can be specified");
		if (cmdline_listen != (com_argc - 1))
			svs.maxclients = atoi (com_argv[cmdline_listen+1]);
		else
			svs.maxclients = 8;
	}

	if (svs.maxclients < 1)
		svs.maxclients = 8;
	else if (svs.maxclients > MAX_SCOREBOARD)
		svs.maxclients = MAX_SCOREBOARD;

	if (cmdline_dedicated || cmdline_listen)
	{
		// Baker: only do this if -dedicated or listen
		// So, why does this need done at all since all we are doing is an allocation below.
		// Well, here is why ....
		// we really want the -dedicated and -listen parameters to operate as expected and
		// not be ignored.  This limit shouldn't be able to be changed in game if specified.
		// But we don't want to hurt our ability to play against the bots either.
	svs.maxclientslimit = svs.maxclients;
	} else svs.maxclientslimit = 16; // Baker: the new default for using neither -listen or -dedicated

	if (svs.maxclientslimit < 4)
		svs.maxclientslimit = 4;
	svs.clients = (struct client_s *)Hunk_AllocName (svs.maxclientslimit*sizeof(client_t), "clients");

	if (svs.maxclients > 1)
		Cvar_SetQuick (&pr_deathmatch, "1");
	else
		Cvar_SetQuick (&pr_deathmatch, "0");
}

void Host_Version_f (void)
{
	Con_Printf ("Quake Version %1.2f\n", QUAKE_VERSION);
	Con_Printf ("%s Version %1.2f\n", ENGINE_NAME, ENGINE_VERSION);
	Con_Printf ("Exe: %s (%i kb)\n", File_URL_SkipPath(Folder_Binary_URL()), (int)File_Length(Folder_Binary_URL())/1024  );
	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
#ifndef SERVER_ONLY
	Con_Printf ("Caches: %s\n", Folder_Caches_URL());
#endif
}
#pragma message ("S_BlockSound on startup does it ever get unblocked?????")

/* cvar callback functions : */
void Host_Callback_Notify (cvar_t *var)
{
	if (sv.active)
		SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name, var->string);
}

// Baker:  Hint to tell dedicated server we are after initial execution of configs.
void Host_Post_Initialization_f (void)
{
	if (!host_post_initialized)
	{
		host_post_initialized = true;
	}
}


/*
=======================
Host_InitLocal
======================
*/
void Host_InitLocal (void)
{
	Cmd_AddCommands (Host_Init);


	if (COM_CheckParm("+capturedemo"))
		cls.capturedemo_and_exit = true;

	if (COM_CheckParm ("-developer"))
		Cvar_SetValueQuick (&developer, 1);

	Host_FindMaxClients ();
}


/*
===============
Host_WriteConfiguration

Writes key bindings and archived cvars to config.cfg
===============
*/
void Host_WriteConfiguration (void)
{
// dedicated servers initialize the host but don't parse and set the
// config.cfg cvars
	if (host_initialized && !isDedicated)
	{
		const char *writelist[3] = {game_startup_dir, va("%s/%s", Folder_Caches_URL(), File_URL_SkipPath (game_startup_dir)), NULL};
		int i;

		for (i = 0; writelist[i]; i ++)
		{
			const char *cursor = writelist[i];
			char outconfig_name[MAX_OSPATH];
			FILE	*f = NULL;

			c_snprintf2 (outconfig_name, "%s/%s", cursor, CONFIG_CFG);

			f = FS_fopen_write_create_path (outconfig_name, "w");

			if (f)
			{
				VID_Cvars_Sync_To_Mode (&vid.modelist[vid.modenum_user_selected]); //johnfitz -- write actual current mode to config file, in case cvars were messed with

				fprintf (f, "// %s\n", ENGINE_FAMILY_NAME);
				Key_WriteBindings (f);
				Cvar_WriteVariables (f);
				FS_fclose (f);

			} else Con_Printf ("Couldn't write %s.\n", CONFIG_CFG);
		}
	}
}


/*
=================
SV_ClientPrintf

Sends text across to be displayed
FIXME: make this just a stuffed echo?
=================
*/
int SV_ClientPrintf (const char *fmt, ...)
{
	VA_EXPAND (string, 1024, fmt)

	MSG_WriteByte (&host_client->message, svc_print);
	MSG_WriteString (&host_client->message, string);
	return 0;
}

/*
=================
SV_BroadcastPrintf

Sends text to all active clients
=================
*/
int SV_BroadcastPrintf (const char *fmt, ...)
{
	int			i;

	VA_EXPAND (string, 1024, fmt)

	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active && svs.clients[i].spawned)
		{
			MSG_WriteByte (&svs.clients[i].message, svc_print);
			MSG_WriteString (&svs.clients[i].message, string);
		}
	}
	return 0;
}

/*
=================
Host_ClientCommands

Send text over to the client to be executed
=================
*/
void Host_ClientCommands (const char *fmt, ...)
{
	VA_EXPAND (string, 1024, fmt)

	MSG_WriteByte (&host_client->message, svc_stufftext);
	MSG_WriteString (&host_client->message, string);
}

/*
=====================
SV_DropClient

Called when the player is getting totally kicked off the host
if (crash = true), don't bother sending signofs
=====================
*/
void SV_DropClient (cbool crash)
{
	int		saveSelf;
	int		i;
	client_t *client;

	if (!crash)
	{
		// send any final messages (don't check for errors)
		if (NET_CanSendMessage (host_client->netconnection))
		{
			MSG_WriteByte (&host_client->message, svc_disconnect);
			NET_SendMessage (host_client->netconnection, &host_client->message);
		}

		if (host_client->edict && host_client->spawned)
		{
		// call the prog function for removing a client
		// this will set the body to a dead frame, among other things
			saveSelf = pr_global_struct->self;
			pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
			PR_ExecuteProgram (pr_global_struct->ClientDisconnect);
			pr_global_struct->self = saveSelf;
		}

		Dedicated_Printf ("Client %s removed\n",host_client->name);
	}

// break the net connection
	NET_Close (host_client->netconnection);
	host_client->netconnection = NULL;

// free the client (the body stays around)
	host_client->active = false;
	Player_IPv4_List_Update ();

	host_client->name[0] = 0;
	host_client->old_frags = -999999;
	net_activeconnections--;

// send notification to all clients
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		MSG_WriteByte (&client->message, svc_updatename);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteString (&client->message, "");
		MSG_WriteByte (&client->message, svc_updatefrags);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteShort (&client->message, 0);
		MSG_WriteByte (&client->message, svc_updatecolors);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteByte (&client->message, 0);
	}
}

/*
==================
Host_ShutdownServer

This only happens at the end of a game, not between levels
==================
*/
void Host_ShutdownServer(cbool crash)
{
	int		i;
	int		count;
	sizebuf_t	buf;
	byte		message[4];
	double	start;

	if (!sv.active)
		return;

	sv.active = false;

// stop all client sounds immediately
	if (cls.state == ca_connected)
		CL_Disconnect ();

// flush any pending messages - like the score!!!
	start = System_DoubleTime();
	do
	{
		count = 0;
		for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		{
			if (host_client->active && host_client->message.cursize)
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					NET_SendMessage(host_client->netconnection, &host_client->message);
					SZ_Clear (&host_client->message);
				}
				else
				{
					NET_GetMessage(host_client->netconnection);
					count++;
				}
			}
		}
		if ((System_DoubleTime() - start) > 3.0)
			break;
	}
	while (count);

// make sure all the clients know we're disconnecting
	buf.data = message;
	buf.maxsize = 4;
	buf.cursize = 0;
	MSG_WriteByte(&buf, svc_disconnect);
	count = NET_SendToAll(&buf, 5.0);
	if (count)
		Con_Printf("Host_ShutdownServer: NET_SendToAll failed for %u clients\n", count);

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active)
			SV_DropClient(crash);

//
// clear structures
//
// Baker --- this is redundant, but I want this function to do as expected
	memset (&sv, 0, sizeof(sv)); // ServerSpawn already do this by Host_ClearMemory
	memset (svs.clients, 0, svs.maxclientslimit*sizeof(client_t));
}


/*
================
Host_ClearMemory

This clears all the memory used by both the client and server, but does
not reinitialize anything.
================
*/
void Host_ClearMemory (void)
{
	Con_DPrintf ("Clearing memory\n");

#ifdef WINQUAKE_RENDERER_SUPPORT
	D_FlushCaches ();
#endif // WINQUAKE_RENDERER_SUPPORT

	Mod_ClearAll ();
/* host_hunklevel MUST be set at this point */
	if (host_hunklevel)
	Hunk_FreeToLowMark (host_hunklevel);
	else System_Error ("Tried to free hunk without hunk");
	cls.signon = 0;
	memset (&sv, 0, sizeof(sv));
	memset (&cl, 0, sizeof(cl));
}


//==============================================================================
//
// Host Frame
//
//==============================================================================

/*
===================
Host_FilterTime

Returns false if the time is too short to run a frame
===================
*/
//float frame_timescale = 1.0f;
cbool Host_FilterTime (double time)
{
	double	maxfps;

	realtime += time;

	//johnfitz -- max fps cvar
	maxfps = CLAMP (10.0, host_maxfps.value, 1000.0);

	// Baker: Don't sleep during a capturedemo (CPU intensive!) or during a timedemo (performance testing)
	if (!cls.capturedemo && !cls.timedemo && realtime - oldrealtime < 1.0/maxfps)
	{
		if (host_sleep.value || (!vid.ActiveApp && (!sv.active || svs.maxclients == 1)))
		{
//			Con_Printf ("Extra sleep\n");
			System_Sleep (QUAKE_SLEEP_TIME); // Lower cpu (sleep = 1 usually -- or 50 for Mac + WinQuake)
		}
		return false; // framerate is too high
	}

	//johnfitz
#ifdef SUPPORTS_AVI_CAPTURE // Baker change
	if (Movie_IsActive())
		host_frametime_ = Movie_FrameTime ();
	else
#endif // Baker change +

	host_timeslice = host_frametime_ = realtime - oldrealtime;
	oldrealtime = realtime;

	//johnfitz -- host_timescale is more intuitive than host_framerate
	if (cls.demoplayback && cls.demospeed && !cls.timedemo && !cls.capturedemo && cls.demonum == -1)
	{
		host_frametime_ *= cls.demospeed;
	}
	else
	if (host_timescale.value > 0 && !(cls.demoplayback && cls.demospeed && !cls.timedemo && !cls.capturedemo && cls.demonum == -1) )
	{
		host_frametime_ *= host_timescale.value;
	}
	//johnfitz
	else if (host_framerate.value > 0)
	{
		host_frametime_ = host_framerate.value;
	}
	else
	{
		 // don't allow really long or short frames
		host_frametime_ = CLAMP (0.001, host_frametime_, 0.1); //johnfitz -- use CLAMP
	}

	return true;
}


/*
===================
Host_GetConsoleCommands

Add commands typed at dedicated server exactly as if they had been typed at the console
===================
*/
void Host_GetConsoleCommands (void)
{
	const char	*cmd;

	while (1)
	{
		cmd = Dedicated_ConsoleInput ();

		if (!cmd)
			break;

		Cbuf_AddText (cmd);
	}
}



/*
==================
Host_Frame

Runs all active servers
==================
*/
double sv_frametime;
double cl_frametime;
double s_frametime;
double host_timeslice; // Time slice not affected by timescale or demospeed, for 2D
void _Host_Frame (double time)
{
	static double		time1 = 0;
	static double		time2 = 0;
	static double		time3 = 0;
	int			pass1, pass2, pass3;
	// something bad happened, or the server disconnected
	if (setjmp (host_abortserver) ) return;

// keep the random time dependent
	rand ();

// decide the simulation time
	if (!Host_FilterTime (time))
		return;			// don't run too fast, or packets will flood out

	sv_frametime = cl_frametime = s_frametime = /*host_timeslice =*/ host_frametime_;

// get new key events
	Input_Local_SendKeyEvents ();

// allow mice or other external controllers to add commands
	Input_Commands (); // Baker: Joystick (on Windows at least) and Mac uses for keyrepeats

// process console commands
	Cbuf_Execute ();
	NET_Poll();

// if running the server locally, make intentions now
	if (sv.active)
		CL_SendCmd (); // This is where mouse input is read
#ifdef _WIN32 // Baker: We probably need this for OS X as well
	else if (console1.forcedup && key_dest == key_game)
		Input_Think (); // Baker: If we have console forced up, Input still needs to "think" (i.e. determine if we should keep the mouse)
#endif

//-------------------
//
// server operations
//
//-------------------

// check for commands typed to the host
	Host_GetConsoleCommands (); // Dedicated

	if (sv.active)
	{
		SV_UpdateServer(sv_frametime);
	}

//-------------------
//
// client operations
//
//-------------------

// if running the server remotely, send intentions now after
// the incoming messages have been read
	if (!sv.active)
		CL_SendCmd ();

// fetch results from server
	if (cls.state == ca_connected)
		CL_UpdateClient (cl_frametime, true);

// update video
	if (host_speeds.value)
		time1 = System_DoubleTime ();

	{
// Not sure if the best place for this
#ifdef CORE_PTHREADS
		Admin_Remote_Update ();
		Con_Queue_PrintRun ();
#endif // CORE_PTHREADS
		SCR_UpdateScreen ();
		if (cls.signon == SIGNONS && !sv.frozen && !cl.paused)
		{
			CL_RunParticles (); //johnfitz -- separated from rendering
			CL_DecayLights ();
		}
	}

	if (host_speeds.value)
		time2 = System_DoubleTime ();

// update audio
	if (cls.signon == SIGNONS)
	{
		S_Update (r_origin, vpn, vright, vup);
	} else S_Update (vec3_origin, vec3_origin, vec3_origin, vec3_origin);

	CDAudio_Update();
#ifdef SUPPORTS_NEHAHRA
	FMOD_Volume_Think ();
#endif // SUPPORTS_NEHAHRA

	if (host_speeds.value)
	{
		pass1 = (time1 - time3)*1000;
		time3 = System_DoubleTime ();
		pass2 = (time2 - time1)*1000;
		pass3 = (time3 - time2)*1000;
		Con_Printf ("%3i tot %3i server %3i gfx %3i snd\n",
					pass1+pass2+pass3, pass1, pass2, pass3);
	}

	host_framecount++;
}

void Host_Frame (double time)
{
	double	time1, time2;
	static double	timetotal;
	static int		timecount;
	int		i, c, m;

	if (!serverprofile.value)
	{
		_Host_Frame (time);
		return;
	}

	time1 = System_DoubleTime ();
	_Host_Frame (time);
	time2 = System_DoubleTime ();

	timetotal += time2 - time1;
	timecount++;

	if (timecount < 1000)
		return;

	m = timetotal * 1000/timecount;
	timecount = 0;
	timetotal = 0;
	c = 0;

	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active)
			c++;
	}

	Con_Printf ("serverprofile: %2i clients %2i msec\n",  c,  m);
}

/*
====================
Host_Init
====================
*/


void Host_Init (void)
{
	voidfunc_t startup_function[] =
	{
		Cbuf_Init,
		Cmd_Init,
		Cvar_Init,
		COM_Init,
		COM_InitFilesystem,
		Con_Init,  // Con_Printfs begin here
		Host_InitLocal,
		W_LoadWadFile,
		PR_Init,
		Mod_Init,
		NET_Init,
		Admin_Init,
		SV_Init,
	// Baker: Dedicated server stops here
		Key_Init,
		View_Init,
		Chase_Init,
		M_Init,
		Lists_Init,
		VID_Init,
		Input_Init,
		Draw_Init,
		SCR_Init,
		R_Init,
		S_Init,
		CDAudio_Init,
		Sbar_Init,
		CL_Init,
		NQ_Init, // StephenKoren: Neural AI initialization.
		Recent_File_Init,
		Utilities_Init,
		Courtesy_Cvars, // Baker: Register our courtesy cvars so WinQuake/GLQuake versions keep each others values
		NULL, // Baker: Terminator
	};

	voidfunc_t* runfunc;

	Dedicated_Printf ("Host_Init\n");

	// Run startup functions
	for (runfunc = &startup_function[0]; *runfunc; runfunc++)
	{
		(*runfunc) ();

		// Dedicated startup stops after SV_Init
		if ( (*runfunc) == SV_Init && isDedicated)
			break;
	}

// Leave this for the moment.  It's obvious enough to remove.
	if (0) {
		double secs = Time_Now ();

		Con_SafePrintf ("Time Now is %s\n", Time_To_String (secs) );
		Con_SafePrintf ("Time Now GMT is %s\n", Time_To_String_GMT (secs) );

		{
			const char *s = Time_To_String (Time_Now ());
			double secs2 = Time_String_To_Time (s);
			Con_SafePrintf ("Time Now is %s\n", Time_To_String (secs2) );
		}
	}
#pragma message ("Have dedicated server show ip address and port on startup?")

	Cbuf_InsertText ("exec quake.rc\n");
	Cbuf_AddText ("\n_host_post_initialized\n");  // Baker -- hint to dedicated server.

	Hunk_AllocName (0, "-HOST_HUNKLEVEL-");
	host_hunklevel = Hunk_LowMark ();

	// Nehahra
	// This must be run *AFTER* Hunk Level is set.  Although that may change real soon.

#ifdef SUPPORTS_NEHAHRA
	if (com_gametype == gametype_nehahra)
		Nehahra_Init ();
#endif // SUPPORTS_NEHAHRA

	host_initialized = true;
	Con_SafePrintf ("\n========= Quake Initialized =========\n\n");
}


/*
===============
Host_Shutdown

FIXME: this is a callback from System_Quit and System_Error.  It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void Host_Shutdown(void)
{
	static cbool isdown = false;

	if (isdown)
	{
		printf ("recursive shutdown\n");
		return;
	}

	isdown = true;

// keep Con_Printf from trying to update the screen
	scr_disabled_for_loading = true; // shutdown

	Host_WriteConfiguration ();

	NET_Shutdown ();

	if (!isDedicated)
	{
		if (console1.initialized)
			History_Shutdown ();

		CDAudio_Shutdown ();
		S_Shutdown();
		Input_Shutdown ();
		VID_Shutdown();
#ifdef CORE_PTHREADS
		ReadList_Ensure_Shutdown ();
#endif // CORE_PTHREADS
	}
}

cbool Read_Early_Cvars_For_File (const char *config_file_name, const cvar_t* list[])
{
	cbool found_any_cvars = false;
	const cvar_t* var;
	char	config_buffer[8192];
	FILE	*f;
	int		bytes_size = COM_FOpenFile (config_file_name, &f);
	int i;

	// Read the file into the buffer.  Read the first 8000 bytes (if longer, tough cookies)
	// Because it is pretty likely that size of file will get a "SZ_GetSpace: overflow without allowoverflow set"
	// During command execution

	if (bytes_size !=-1)
	{
		int	bytes_in = c_min (bytes_size, 8000); // Cap at 8000
		int bytes_read = fread (config_buffer, 1, bytes_in, f);
		config_buffer [bytes_read + 1] = 0; // Null terminate just in case

		FS_fclose (f);
	} else return false;

	for (i = 0, var = list[i]; var; i++, var = list[i])
	{
		float value;
		cbool found = COM_Parse_Float_From_String (&value, config_buffer, var->name);

#if 0
		System_Alert (va("Cvar %s was %s and is %g", video_cvars[i]->name, found ? "Found" : "Not found", found ? value : 0));
#endif

		if (found == false)
			continue;

		Cvar_SetValueByName (var->name, value);
		found_any_cvars = true;
	}

	return found_any_cvars;
}

