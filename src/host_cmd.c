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

#include "quakedef.h"

/*
==================
Host_Quit_f
==================
*/

void Host_Quit (void)
{
	CL_Disconnect ();
	Host_ShutdownServer(false);

	System_Quit ();
}

// This is the command
void Host_Quit_f (lparse_t *unused)
{
	// Force shareware sell screen?
	Host_Quit ();
}


const char *gamedir_type_names[MAXGAMETYPES] =
{
	"", // id1 doesn't require one
	"-rogue" ,
	"-hipnotic",
	"-quoth",
	"-nehahra",
};

const char *Gamedir_TypeName (void)
{
	return gamedir_type_names[com_gametype];
}

typedef enum
{
	game_fail_none = 0,
	game_fail_rogue = 1,
	game_fail_hipnotic = 2,
	game_fail_quoth = 3,
	game_fail_nehahra = 4,
	MAX_GAMEDIR_TYPES =5,
	game_fail_shareware = 6,
	game_fail_relative_path_not_allowed = 7,
	game_fail_not_installed = 8,
} gamedir_fail_t;

const char *fail_reason_strings[] =
{
	NULL, // id1 doesn't require one
	"Rogue is not installed.", // 2
	"Hipnotic is not installed.", // 2
	"Quoth is not installed.", // 3
	"Nehahra is not installed.", // 3
	NULL, // 4
	"You must have the registered version to use modified games", // 5
	"Relative pathnames are not allowed.", // 6
	"Game not installed.", // 7
};



gametype_t gametype_eval (const char *hudstring)
{
	if	 	 (!hudstring)							return gametype_standard; // To avoid potential NULL comparison
	else if  (!strcasecmp (hudstring, "-rogue"))	return gametype_rogue;
	else if  (!strcasecmp (hudstring, "-hipnotic"))	return gametype_hipnotic;
	else if  (!strcasecmp (hudstring, "-quoth"))	return gametype_quoth;
	else if  (!strcasecmp (hudstring, "-nehahra"))	return gametype_nehahra;

	return gametype_standard;
}

typedef enum
{
	game_change_fail = -1,
	game_no_change = 0,
	game_change_success = 1
} game_result_t;

// returns 0 if everything is ok
gamedir_fail_t game_available (const char *dir, gametype_t gm)
{
	cbool custom_game = !!strcasecmp (dir, GAMENAME /* "id1*/);

	if (static_registered == false && (custom_game || gm != gametype_standard))
		return game_fail_shareware; // shareware and modified gamedir

	if (strstr (dir, ".."))
		return game_fail_relative_path_not_allowed;

	if (gm == gametype_rogue && File_Exists (basedir_to_url("rogue"))  == false)
		return game_fail_rogue;

	if (gm == gametype_hipnotic && File_Exists (basedir_to_url ("hipnotic")) == false)
		return game_fail_hipnotic;

	if (gm == gametype_quoth && File_Exists (basedir_to_url ("quoth")) == false)
		return game_fail_quoth;

	if (gm == gametype_nehahra && File_Exists (basedir_to_url ("nehahra")) == false)
		return game_fail_nehahra;

	if (custom_game && File_Exists (basedir_to_url (dir)) == false)
		return game_fail_not_installed;

	return game_fail_none /* 0 */;
}


#pragma message ("Do not allow a user command in console to change gamedir while demo is playing")
#pragma message ("Technically a demo could change the gamedir while we are in menu!!")

int Host_Gamedir_Change (const char *gamedir_new, const char *new_hud_typestr, cbool liveswitch, const char** info_string)
{
	gametype_t	new_gametype	= gametype_eval (new_hud_typestr);
	cbool		is_custom		= !!strcasecmp(gamedir_new, GAMENAME); // GAMENAME is "id1"

	cbool		gamedir_change	= !!strcasecmp (gamedir_shortname(), gamedir_new );
	cbool		gametype_change	= (new_gametype != com_gametype);
	cbool		any_change		= (gamedir_change || gametype_change);

	int			change_fail		= game_available (gamedir_new, new_gametype);

	if (any_change == false)
	{
		Con_DPrintf ("Gamedir change is no change\n");
		return game_no_change;
	}

	if (change_fail)
	{
		*info_string = fail_reason_strings[change_fail];
		Con_DPrintf ("%s\n", *info_string);
		return game_change_fail;
	}

	// Everything ok now ....
	Con_DPrintf ("New game and/or hud style requested\n");

	com_gametype = new_gametype;

	// If we aren't receiving this via connected signon switch
	// then kill the server.
	if (liveswitch == false)
	{
		//Kill the server
		CL_Disconnect ();
		Host_ShutdownServer(true);
	}

	//Write config file
	Host_WriteConfiguration ();

	//Kill the extra game if it is loaded. Note: RemoveAllPaths and COM_AddGameDirectory set com_gamedir
	COM_RemoveAllPaths ();

	com_modified = true;

	// add these in the same order as ID do (mission packs always get second-lowest priority)
	switch (com_gametype)
	{
	case gametype_rogue:			COM_AddGameDirectory ("rogue");		break;
	case gametype_hipnotic:			COM_AddGameDirectory ("hipnotic");	break;
	case gametype_quoth:			COM_AddGameDirectory ("quoth");		break;
//	case gametype_nehahra:			COM_AddGameDirectory ("nehahra");	break; // Nehahra must manually be added
	case gametype_standard:			com_modified = false;				break;
	default:						break; // Nehahra hits here
	}

	if (is_custom)
	{
		com_modified = true;
		COM_AddGameDirectory (gamedir_new);
	}

	//clear out and reload appropriate data
#ifdef SUPPORTS_NEHAHRA
	Nehahra_Shutdown ();
#endif // SUPPORTS_NEHAHRA
	Cache_Flush (NULL);
	Mod_ClearAll_Compact (); // Baker: Prevent use of wrong cache data

	if (!isDedicated)
	{
#ifdef GLQUAKE_TEXTURE_MANAGER // Baker: No texture mgr in WinQuake
		TexMgr_NewGame ();
#endif // GLQUAKE_TEXTURE_MANAGER
#ifdef WINQUAKE_PALETTE // FitzQuake does this in TexMgr_NewGame
		VID_Palette_NewGame ();
#endif // WINQUAKE_PALETTE

		Draw_NewGame ();

		R_NewGame ();

		cls.music_run = false;
	}

	Lists_NewGame ();
	Recent_File_NewGame ();

#ifdef SUPPORTS_NEHAHRA
	Nehahra_Init ();
#endif // SUPPORTS_NEHAHRA

	// Return description of current setting
	*info_string = gamedir_new;
	return game_change_success;
}


/*
==================
Host_Game_f
==================
*/

void Host_Game_f (lparse_t *line)
{
	const char *gametypename;
	const char *gamedir_directory;
	const char *hudstyle = NULL;
	const char *feedback_string;
	int result;

	switch (line->count)
	{
	case 3:
		hudstyle = line->args[2];
		// Falls through ...
	case 2:
		gamedir_directory = line->args[1];
		result = Host_Gamedir_Change (line->args[1], line->args[2], false, &feedback_string);

		switch (result)
		{
		case game_change_fail:
			Con_Printf ("%s\n", feedback_string);
			break;
		case game_no_change:
			Con_Printf ("Game already set!\n");
			break;
		case game_change_success:
			Con_Printf("\"game\" changed to \"%s\"\n", feedback_string);
			break;
		}
		break;

	default:
		//Diplay the current gamedir
		gametypename = Gamedir_TypeName ();
		Con_Printf("\"game\" is \"%s%s\"\n", gamedir_shortname(), gametypename[0] ?
					va(" %s", gametypename) : "" );
		Con_Printf("Start map is %s\n", game_startmap);
		break;
	}

}

/*
=============
Host_Mapname_f -- johnfitz
=============
*/
void Host_Mapname_f (void)
{
	if (sv.active)
	{
		Con_Printf ("\"mapname\" is \"%s\"\n", sv.name);
		return;
	}

	if (cls.state == ca_connected)
	{
		Con_Printf ("\"mapname\" is \"%s\"\n", cl.worldname);
		return;
	}

	Con_Printf ("no map loaded\n");
}

/*
==================
Host_Status_f
==================
*/
void Host_Status_f (lparse_t *line)
{
	client_t	*client;
	int			seconds;
	int			minutes;
	int			hours = 0;
	int			j;
	int		    (*print_fn) (const char *fmt, ...) __fp_attribute__((__format__(__printf__,1,2)));
	int			a, b, c;

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
		print_fn = SV_ClientPrintf;

	print_fn ("host:    %s\n", Cvar_VariableString ("hostname"));
	print_fn ("version: %4.2f\n", ENGINE_VERSION);
	if (tcpipAvailable)
		print_fn ("tcp/ip:  %s:%d\n", my_tcpip_address, net_hostport);
	print_fn ("map:     %s\n", sv.name);
	print_fn ("players: %i active (%i max)\n\n", net_activeconnections, svs.maxclients);
	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		seconds = (int)(net_time - NET_QSocketGetTime(client->netconnection) );
		minutes = seconds / 60;
		if (minutes)
		{
			seconds -= (minutes * 60);
			hours = minutes / 60;
			if (hours)
				minutes -= (hours * 60);
		}
		else
			hours = 0;

		print_fn ("#%-2u %-16.16s  %3i  %2i:%02i:%02i\n", j+1, client->name, (int)client->edict->v.frags, hours, minutes, seconds);
		//print_fn ("   %s\n", NET_QSocketGetAddressString(client->netconnection));

		if (cmd_source == src_command || !pq_privacy_ipmasking.value)
			print_fn ("   %s\n", NET_QSocketGetAddressString(client->netconnection));
		else if (1 <= (int)pq_privacy_ipmasking.value && (int)pq_privacy_ipmasking.value <= 2 && sscanf(NET_QSocketGetAddressString(client->netconnection), "%d.%d.%d", &a, &b, &c) == 3)
			print_fn ("   %d.%d.%d.xxx\n", a, b, c);
		else print_fn ("   private\n");
	}
}



/*
==================
Host_God_f

Sets client to godmode
==================
*/
void Host_God_f (lparse_t *line)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer (line);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	if (sv.disallow_major_cheats && !host_client->privileged)
	{
		SV_ClientPrintf ("No cheats allowed, use sv_cheats 1 and restart level to enable.\n");
		return;
	}

	//johnfitz -- allow user to explicitly set god mode to on or off
	switch (line->count)
	{
	case 1:
		sv_player->v.flags = (int)sv_player->v.flags ^ FL_GODMODE;
		if (!((int)sv_player->v.flags & FL_GODMODE) )
			SV_ClientPrintf ("godmode OFF\n");
		else
			SV_ClientPrintf ("godmode ON\n");
		break;
	case 2:
		if (atof(line->args[1]))
		{
			sv_player->v.flags = (int)sv_player->v.flags | FL_GODMODE;
			SV_ClientPrintf ("godmode ON\n");
		}
		else
		{
			sv_player->v.flags = (int)sv_player->v.flags & ~FL_GODMODE;
			SV_ClientPrintf ("godmode OFF\n");
		}
		break;
	default:
		Con_Printf("god [value] : toggle god mode. values: 0 = off, 1 = on\n");
		break;
	}
	//johnfitz
}

/*
==================
Host_Notarget_f
==================
*/
void Host_Notarget_f (lparse_t *line)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer (line);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	if (sv.disallow_major_cheats && !host_client->privileged)
	{
		SV_ClientPrintf ("No cheats allowed, use sv_cheats 1 and restart level to enable.\n");
		return;
	}

	//johnfitz -- allow user to explicitly set notarget to on or off
	switch (line->count)
	{
	case 1:
		sv_player->v.flags = (int)sv_player->v.flags ^ FL_NOTARGET;
		if (!((int)sv_player->v.flags & FL_NOTARGET) )
			SV_ClientPrintf ("notarget OFF\n");
		else
			SV_ClientPrintf ("notarget ON\n");
		break;
	case 2:
		if (atof(line->args[1]))
		{
			sv_player->v.flags = (int)sv_player->v.flags | FL_NOTARGET;
			SV_ClientPrintf ("notarget ON\n");
		}
		else
		{
			sv_player->v.flags = (int)sv_player->v.flags & ~FL_NOTARGET;
			SV_ClientPrintf ("notarget OFF\n");
		}
		break;
	default:
		Con_Printf("notarget [value] : toggle notarget mode. values: 0 = off, 1 = on\n");
		break;
	}
	//johnfitz
}


/*
==================
Host_Noclip_f
==================
*/
void Host_Noclip_f (lparse_t *line)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer (line);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	if (sv.disallow_major_cheats && !host_client->privileged)
	{
		SV_ClientPrintf ("No cheats allowed, use sv_cheats 1 and restart level to enable.\n");
		return;
	}

	//johnfitz -- allow user to explicitly set noclip to on or off
	switch (line->count)
	{
	case 1:
		if (sv_player->v.movetype != MOVETYPE_NOCLIP)
		{
			cl.noclip_anglehack = true;
			sv_player->v.movetype = MOVETYPE_NOCLIP;
			SV_ClientPrintf ("noclip ON\n");
		}
		else
		{
			cl.noclip_anglehack = false;
			sv_player->v.movetype = MOVETYPE_WALK;
			SV_ClientPrintf ("noclip OFF\n");
		}
		break;
	case 2:
		if (atof(line->args[1]))
		{
			cl.noclip_anglehack = true;
			sv_player->v.movetype = MOVETYPE_NOCLIP;
			SV_ClientPrintf ("noclip ON\n");
		}
		else
		{
			cl.noclip_anglehack = false;
			sv_player->v.movetype = MOVETYPE_WALK;
			SV_ClientPrintf ("noclip OFF\n");
		}
		break;
	default:
		Con_Printf("noclip [value] : toggle noclip mode. values: 0 = off, 1 = on\n");
		break;
	}
	//johnfitz
}

/*
==================
Host_Fly_f

Sets client to flymode
==================
*/
void Host_Fly_f (lparse_t *line)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer (line);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	if (sv.disallow_major_cheats && !host_client->privileged)
	{
		SV_ClientPrintf ("No cheats allowed, use sv_cheats 1 and restart level to enable.\n");
		return;
	}

	//johnfitz -- allow user to explicitly set noclip to on or off
	switch (line->count)
	{
	case 1:
		if (sv_player->v.movetype != MOVETYPE_FLY)
		{
			sv_player->v.movetype = MOVETYPE_FLY;
			SV_ClientPrintf ("flymode ON\n");
		}
		else
		{
			sv_player->v.movetype = MOVETYPE_WALK;
			SV_ClientPrintf ("flymode OFF\n");
		}
		break;
	case 2:
		if (atof(line->args[1]))
		{
			sv_player->v.movetype = MOVETYPE_FLY;
			SV_ClientPrintf ("flymode ON\n");
		}
		else
		{
			sv_player->v.movetype = MOVETYPE_WALK;
			SV_ClientPrintf ("flymode OFF\n");
		}
		break;
	default:
		Con_Printf("fly [value] : toggle fly mode. values: 0 = off, 1 = on\n");
		break;
	}
	//johnfitz
}

void Host_Legacy_FreezeAll_f (lparse_t *unused)
{
	Con_Printf ("Use 'freezeall' instead of sv_freezenonclients.  It is shorter.\n");
}

void Host_Freezeall_f (lparse_t *line)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer (line);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	if (sv.disallow_major_cheats && !host_client->privileged)
	{
		SV_ClientPrintf ("No cheats allowed, use sv_cheats 1 and restart level to enable.\n");
		return;
	}

	switch (line->count)
	{
	case 1:
		sv.frozen = !sv.frozen;

		if (sv.frozen)
			SV_ClientPrintf ("freeze mode ON\n");
		else
			SV_ClientPrintf ("freeze mode OFF\n");

		break;
	case 2:
		if (atof(line->args[1]))
		{
			sv.frozen = true;
			SV_ClientPrintf ("freeze mode ON\n");
		}
		else
		{
			sv.frozen = false;
			SV_ClientPrintf ("freeze mode OFF\n");
		}
		break;
	default:
		Con_Printf("freezeall [value] : toggle freeze mode. values: 0 = off, 1 = on\n");
		break;
	}

}

/*
==================
Host_Ping_f

==================
*/
void Host_Ping_f (lparse_t *line)
{
	int		i, j, ping_display;
	float	total;
	client_t	*client;

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer (line);
		return;
	}

	SV_ClientPrintf ("Client ping times:\n");
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		total = 0;
		for (j=0 ; j<NUM_PING_TIMES ; j++)
			total+=client->ping_times[j];
		total /= NUM_PING_TIMES;

		ping_display = (int)(total*1000);
		if (pq_ping_rounding.value)
			ping_display = CLAMP(40, c_rint(ping_display / 40) * 40, 999);
		SV_ClientPrintf ("%4i %s\n", ping_display, client->name);
	}
}


void Host_Changelevel_Required_Msg (cvar_t* var)
{
	if (host_post_initialized)
		Con_Printf ("%s change takes effect on map restart/change.\n", var->name);
}


/*
===============================================================================

SERVER TRANSITIONS

===============================================================================
*/


/*
======================
Host_Map_f

handle a
map <servername>
command from the console.  Active clients are kicked off.
======================
*/
void Host_Map_f (lparse_t *line)
{
	int		i;
	char	name[MAX_QPATH];

	// Quakespasm: map with no parameter says name
	if (line->count < 2)	//no map name given
	{
		if (isDedicated)
		{
			if (sv.active)
				Con_Printf ("Current map: %s\n", sv.name);
			else
				Con_Printf ("Server not active\n");
		}
		else if (cls.state == ca_connected)
		{
			Con_Printf ("\nCurrent map: %s\n", cl.worldname);
			Con_Printf ("Title:       %s\n", cl.levelname);
			Con_Printf ("Sky key:     %s\n", level.sky_key);
			Con_Printf ("Fog key:     %s\n", level.fog_key);
			Con_Printf ("Water-vised: %s\n", level.water_vis_known ? (level.water_vis ? "Yes" : "No") : "Not determined yet" );
			Con_Printf ("\nType \"copy ents\" to copy entities to clipboard\n\n");
		}
		else
		{
			Con_Printf ("map <levelname>: start a new server\n");
		}
		return;
	}


	if (cmd_source != src_command)
		return;

	CL_Clear_Demos_Queue (); // timedemo is a very intentional action

#ifdef BUGFIX_DEMO_RECORD_BEFORE_CONNECTED_FIX
	// Baker: Don't cause demo shutdown if started recording before
	// playing map
	// Since this is map startup, it doesn't affect anything else
	// Like typing "map <my name>" while already connected.
	if (cls.state == ca_connected)
#endif // BUGFIX_DEMO_RECORD_BEFORE_CONNECTED_FIX
		CL_Disconnect ();

	Host_ShutdownServer(false);

	SCR_BeginLoadingPlaque_Force_NoTransition ();
	Key_SetDest (key_game);
	console1.visible_pct = 0;

	cls.mapstring[0] = 0;
	for (i = 0 ; i < line->count ; i++)
	{
		c_strlcat (cls.mapstring, line->args[i]);
		c_strlcat (cls.mapstring, " ");
	}
	c_strlcat (cls.mapstring, "\n");

	svs.serverflags = 0;			// haven't completed an episode yet
	c_strlcpy (name, line->args[1]);
	SV_SpawnServer (name);
	if (!sv.active)
		return;

	if (!isDedicated)
	{
		memset (cls.spawnparms, 0, sizeof(cls.spawnparms));
		for (i = 2 ; i < line->count ; i++)
		{
			c_strlcat (cls.spawnparms, line->args[i]);
			c_strlcat (cls.spawnparms, " ");
		}

		Cmd_ExecuteString ("connect local", src_command);
	}
}

/*
==================
Host_Changelevel_f

Goes to a new map, taking all clients along
==================
*/
void Host_Changelevel_f (lparse_t *line)
{
	char	newlevel[MAX_QPATH];
	int		i; //johnfitz

	if (line->count != 2)
	{
		Con_Printf ("changelevel <levelname> : continue game on a new level\n");
		return;
	}
	if (!sv.active || cls.demoplayback)
	{
		Con_Printf ("Only the server may changelevel\n");
		return;
	}

	//johnfitz -- check for client having map before anything else
	c_snprintf (newlevel, "maps/%s.bsp", line->args[1]);
	if (COM_OpenFile (newlevel, &i) == -1)
		Host_Error ("cannot find map %s", newlevel);

// Baker: Shouldn't this close it?
#if 1
	COM_CloseFile (i);
#endif

	//johnfitz

	if (!isDedicated)
	{
		#if 1 // Baker:  Clear the noise
			S_BlockSound ();
			S_ClearBuffer ();
			S_UnblockSound ();
		#endif
	}

	SV_SaveSpawnparms ();
	c_strlcpy (newlevel, line->args[1]);
	SV_SpawnServer (newlevel);
}

/*
==================
Host_Restart_f

Restarts the current server for a dead player
==================
*/
void Host_Restart_f (lparse_t *unnused)
{
	char	mapname[MAX_QPATH];

	if (cls.demoplayback || !sv.active)
		return;

	if (cmd_source != src_command)
		return;
	c_strlcpy (mapname, sv.name);	// mapname gets cleared in spawnserver
	SV_SpawnServer (mapname);
}

/*
==================
Host_Reconnect_f

This command causes the client to wait for the signon messages again.
This is sent just before a server changes levels
==================
*/
// This can have 2 different scenarios.  Entry might be at 0, in that case 4 should clear plaque
//If signon is 4, that is death or changelevel.  What do we do?  Clear immediately?  But in 0 case, don't
void Host_Reconnect_f (lparse_t *unused)
{
#if 1 // Baker:  Clear the noise
	S_BlockSound ();
	S_ClearBuffer ();
	S_UnblockSound ();
#endif

	// Consider stopping sound here?

	if (cls.demoplayback)
	{
		Con_DPrintf("Demo playing; ignoring reconnect\n");
		SCR_EndLoadingPlaque (); // reconnect happens before signon reply #4
		return;
	} else
	{
		SCR_BeginLoadingPlaque_Force_Transition ();
		cls.signon = 0;		// need new connection messages
	}
}

/*
=====================
Host_Connect_f

User command to connect to server
=====================
*/
void Host_Connect_f (lparse_t *line)
{
	char	name[MAX_QPATH];

	cls.demonum = -1;		// stop demo loop in case this fails
	if (cls.demoplayback)
	{
		CL_StopPlayback ();

		CL_Clear_Demos_Queue (); // timedemo is a very intentional action
		CL_Disconnect ();
	}
	SCR_BeginLoadingPlaque_Force_NoTransition ();
	Key_SetDest (key_game);
	console1.visible_pct = 0;

	c_strlcpy (name, line->args[1]);
	CL_EstablishConnection (name);
	Host_Reconnect_f (NULL);
	c_strlcpy (server_name, name); // JPG - 3.50
}

keyvalue_t hintnames[MAX_NUM_HINTS + 1] =
{
	{ "game", hint_game },
	{ "skill", hint_skill },
	{ "fileserver_port", hint_fileserver_port },
	{ "", 0 },
};


/*
===============================================================================

LOAD / SAVE GAME

===============================================================================
*/

#define	SAVEGAME_VERSION	5

/*
===============
Host_SavegameComment

Writes a SAVEGAME_COMMENT_LENGTH character comment describing the current
===============
*/
void Host_SavegameComment (char *text)
{
	int		i;
	char	kills[20];

	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		text[i] = ' ';
	memcpy (text, cl.levelname, c_min(strlen(cl.levelname),22)); //johnfitz -- only copy 22 chars.
	c_snprintf2 (kills, "kills:%3i/%3i", cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS]);
	memcpy (text+22, kills, strlen(kills));
// convert space to _ to make stdio happy
	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
	{
		if (text[i] == ' ' || text[i] == '\n' || text[i] == 0x1A) // Aguirre save game comment fix part 1
			text[i] = '_';
	}
	text[SAVEGAME_COMMENT_LENGTH] = '\0';
}

const char *Host_Savegame (const char *in_savename, cbool user_initiated)
{
	static char	name[MAX_OSPATH];
	FILE	*f;
	int		i;
	char	comment[SAVEGAME_COMMENT_LENGTH+1];

	FS_FullPath_From_QPath (name, in_savename);
	File_URL_Edit_Force_Extension (name, ".sav", sizeof(name));

	if (user_initiated)
		Con_Printf ("Saving game to %s...\n", name);

	f = FS_fopen_write (name, "w"); // Would need to add 'b' for binary here.
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open save file for writing.\n");
		return NULL;
	}

	fprintf (f, "%i\n", SAVEGAME_VERSION);
	Host_SavegameComment (comment);
	fprintf (f, "%s\n", comment);
	for (i = 0 ; i < NUM_SPAWN_PARMS ; i++)
		fprintf (f, "%f\n", svs.clients->spawn_parms[i]);
	fprintf (f, "%d\n", sv.current_skill);
	fprintf (f, "%s\n", sv.name);
	fprintf (f, "%f\n",sv.time);

// write the light styles

	for (i = 0 ; i < MAX_LIGHTSTYLES ; i++)
	{
		if (sv.lightstyles[i])
			fprintf (f, "%s\n", sv.lightstyles[i]);
		else
			fprintf (f,"m\n");
	}


	ED_WriteGlobals (f);
	for (i = 0 ; i < sv.num_edicts ; i++)
	{
		ED_Write (f, EDICT_NUM(i));
		// fflush (f); // Baker: This makes save games slow as hell.  Fix from MH
	}
	FS_fclose (f);
	return name;
}


/*
===============
Host_Savegame_f
===============
*/
void Host_Savegame_f (lparse_t *line)
{
	const char *saved_name = NULL;
	int i;

	if (cmd_source != src_command)
		return;

	if (!sv.active)
	{
		Con_Printf ("Not playing a local game.\n");
		return;
	}

	if (cl.intermission)
	{
		Con_Printf ("Can't save in intermission.\n");
		return;
	}

	if (svs.maxclients != 1)
	{
		Con_Printf ("Can't save multiplayer games.\n");
		return;
	}

	if (line->count != 2)
	{
		Con_Printf ("save <savename> : save a game\n");
		return;
	}

	if (strstr(line->args[1], ".."))
	{
		Con_Printf ("Relative pathnames are not allowed.\n");
		return;
	}

	for (i = 0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active && (svs.clients[i].edict->v.health <= 0) )
		{
			Con_Printf ("Can't savegame with a dead player\n");
			return;
		}
	}

	saved_name = Host_Savegame (line->args[1], true);

	if (saved_name)
	{
		Lists_Update_Savelist ();
		Recent_File_Set_FullPath (saved_name);
		Con_Printf ("done.\n");
	}
}


/*
===============
Host_Loadgame_f
===============
*/
void Host_Loadgame_f (lparse_t *line)
{
	char	name[MAX_OSPATH];
	FILE	*f;
	char	mapname[MAX_QPATH];
	float	time, tfloat;
	char	str[32768];
	const char *start;
	int		i, r;
	edict_t	*ent;
	int		entnum;
	int		version;
	cbool anglehack;
	float			spawn_parms[NUM_SPAWN_PARMS];

	if (cmd_source != src_command)
		return;

	if (line->count != 2)
	{
		Con_Printf ("load <savename> : load a game\n");
		return;
	}

	cls.demonum = -1;		// stop demo loop in case this fails

	FS_FullPath_From_QPath (name, line->args[1]);
	File_URL_Edit_Default_Extension (name, ".sav", sizeof(name));

// we can't call SCR_BeginLoadingPlaque, because too much stack space has
// been used.  The menu calls it before stuffing loadgame command
//	SCR_BeginLoadingPlaque ();

	Con_Printf ("Loading game from %s...\n", name);
#pragma message ("Aguirre has a read-binary fix for save games with special characters")
	f = FS_fopen_read (name, "r"); // aguirRe: Use binary mode to avoid EOF issues in savegame files
	// Baker changed back to "r" from "rb" because it adds extra new lines..
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open load file for reading.\n");
		return;
	}

	fscanf (f, "%i\n", &version);
	if (version != SAVEGAME_VERSION)
	{
		FS_fclose (f);
		Con_Printf ("Savegame is version %i, not %i\n", version, SAVEGAME_VERSION);
		return;
	}

	SCR_BeginLoadingPlaque_Force_NoTransition ();
	Key_SetDest (key_game);
	console1.visible_pct = 0;

#if 0
	// aguirre: Kludge to read saved games with newlines in title
	do
		fscanf (f, "%s\n", str);
	while (!feof(f) && !strstr(str, "kills:"));
#else
	fscanf (f, "%s\n", str);
#endif

	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		fscanf (f, "%f\n", &spawn_parms[i]);
// this silliness is so we can load 1.06 save files, which have float skill values
	fscanf (f, "%f\n", &tfloat);
	sv.current_skill = (int)(tfloat + 0.1);
	Cvar_SetValueQuick (&pr_skill, (float)sv.current_skill);

	fscanf (f, "%s\n",mapname);
	fscanf (f, "%f\n",&time);

	CL_Clear_Demos_Queue (); // timedemo is a very intentional action
	CL_Disconnect ();

	SV_SpawnServer (mapname);

	if (!sv.active)
	{
		FS_fclose (f);
		Con_Printf ("Couldn't load map\n");
		SCR_EndLoadingPlaque ();
		return;
	}
	sv.paused = true;		// pause until all clients connect
	sv.loadgame = true;

// load the light styles

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		fscanf (f, "%s\n", str);
		sv.lightstyles[i] = (const char *)Hunk_Strdup (str, "lightstyles");
	}

// load the edicts out of the savegame file
	entnum = -1;		// -1 is the globals
	while (!feof(f))
	{
		for (i = 0 ; i < (int) sizeof(str) - 1 ; i ++)
		{
			r = fgetc (f);
			if (r == EOF || !r)
				break;
			str[i] = r;
			if (r == '}')
			{
				i++;
				break;
			}
		}
		if (i ==  (int) sizeof(str)-1)
		{
			FS_fclose (f);
			System_Error ("Loadgame buffer overflow");
		}
		str[i] = 0;
		start = str;
		start = COM_Parse(str);
		if (!com_token[0])
			break;		// end of file
		if (strcmp(com_token,"{"))
		{
			FS_fclose (f);
			System_Error ("First token isn't a brace");
		}

		if (entnum == -1)
		{	// parse the global vars
			ED_ParseGlobals (start);
		}
		else
		{	// parse an edict

			ent = EDICT_NUM(entnum);
			memset (&ent->v, 0, progs->entityfields * 4);
			ent->free = false;
			ED_ParseEdict (start, ent, &anglehack);

		// link it into the bsp tree
			if (!ent->free)
				SV_LinkEdict (ent, false);
		}

		entnum++;
	}

	sv.num_edicts = entnum;
	sv.time = time;
	sv.auto_save_time = sv.time + AUTO_SAVE_MINIMUM_TIME;

	FS_fclose (f);

	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		svs.clients->spawn_parms[i] = spawn_parms[i];

	if (!isDedicated)
	{
		CL_EstablishConnection ("local");
		Host_Reconnect_f (NULL);
	}
}

//============================================================================

/*
======================
Host_Name_f
======================
*/
void Host_Name_f (lparse_t *line)
{
	char	newName[32];

	if (line->count == 1)
	{
		Con_Printf ("\"name\" is \"%s\"\n", cl_name.string);
		return;
	}
	if (line->count == 2)
	{
		c_strlcpy(newName, line->args[1]);
	}
	else
	{
		size_t offsetz = line->args[1] - line->chopped; // Offset into line after whitespace
		char *args_after_command = &line->original[offsetz];

		c_strlcpy(newName, args_after_command);
	}

	newName[15] = 0;	// client_t structure actually says name[32].

	if (cmd_source == src_command)
	{
		if (strcmp(cl_name.string, newName) == 0)
			return;
		Cvar_SetQuick (&cl_name, newName);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer (line);
		return;
	}

	if (host_client->name[0] && strcmp(host_client->name, "unconnected") )
	{
		if (strcmp(host_client->name, newName) != 0)
			Con_Printf ("%s renamed to %s\n", host_client->name, newName);
	}
	c_strlcpy (host_client->name, newName);
	host_client->edict->v.netname = PR_SetEngineString(host_client->name);

// send notification to all clients

	MSG_WriteByte (&sv.reliable_datagram, svc_updatename);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteString (&sv.reliable_datagram, host_client->name);
}

#ifdef SUPPORTS_PQ_WORD_FILTER // Baker change
// TODO: Despace and remove non-alphanumeric and check
const char* bad_words[] = 
{
// Lite bad words are replaced with asterisks
	"fuck", "f**k",
	"shit", "s__t", 
	"butt", "****",
	"cock", "****",
	"douche", "******",

// Worse bad words are replaced with something stupid to make speaker feel awkward
	"wetback", "houston",
	"fukking", "beijing",
	"fucking", "cheddar",
	"anus", "snus", 
	"fag", "fan", 
	"fgt", "yam",
	"faggot", "maddog",
	"faget", "italy",
	"fggt", "hawk",
	"fagot", "cairo",
	"ngr", "ron",
	"nigger", "friend",
	"niger", "frank",
	"n1g", "man",
	"queer", "zebop",
	"cunt", "cute",
	"homo", "mang",
	"penis", "sonar",
	"penus", "sugar",
	"vagina", "robert",
	
	NULL, NULL
};

// 2 elements

char *String_Edit_Normalize_Text (const char *text)
{
	static char normalized_buffer[SYSTEM_STRING_SIZE_1024];
	char *cur = normalized_buffer;
	
	c_strlcpy (normalized_buffer, text);

	for ( ; *cur; cur ++)
	{
		if (*cur > 128) *cur -= 128;  // debronze
		*cur = tolower(*cur); // lower
		     if (*cur == '4') *cur = 'a';
		else if (*cur == '3') *cur = 'e';
		else if (*cur == '1') *cur = 'i';
		else if (*cur == '0') *cur = 'o';
	}

	return normalized_buffer;
}


char* WordFilter_Check (const char* text)
{
	static char new_text[SYSTEM_STRING_SIZE_1024];
	int i;
	
	const char *norm_text = String_Edit_Normalize_Text (text);
	char *curword;
	cbool replacement = false;

	for (i = 0; bad_words[i]; i += 2)
	{
		if ( (curword = strstr(norm_text, bad_words[i]) ) )
		{
			int replace_len = strlen(bad_words[i + 1]);
			int replace_offset = curword - norm_text;
#ifdef _DEBUG
			int src_len = strlen(bad_words[i + 0]);
			cbool ok = src_len = replace_len;
			if (!ok) System_Error ("No match length word filter!");
#endif
			if (replacement == false && (replacement = true) /* evile assignment */)
				c_strlcpy (new_text, text);
			
			memcpy (&new_text[replace_offset], bad_words[i + 1], replace_len);
		}
	}

	if (replacement)
		return new_text;
	else return NULL;
}

#endif // Baker change + SUPPORTS_PQ_WORD_FILTER



void Host_Say (lparse_t *line, cbool teamonly)
{
	int		j;
	client_t *client;
	client_t *save;
	char	*p;
	char	text[64];
	cbool	fromServer = false;

	if (cmd_source == src_command)
	{
		if (isDedicated)
		{
			fromServer = true;
			teamonly = false;
		}
		else
		{
			Cmd_ForwardToServer (line);
			return;
		}
	}

	if (line->count < 2)
		return;

	save = host_client;

	p = (char*)&line->original[line->args[1]-line->chopped-1]; // evil

// remove quotes if present
	if (*p == '\"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

// turn on color set 1
	if (!fromServer)
	{
		char *text_filtered = NULL;

		double connected_time = (net_time - NET_QSocketGetTime(host_client->netconnection));
		// R00k - dont allow new connecting players to spam obscenities...
		if (pq_chat_connect_mute_seconds.value && connected_time < pq_chat_connect_mute_seconds.value)
			return;

		// JPG 3.00 - don't allow messages right after a colour/name change
		if (pq_chat_color_change_delay.value && sv.time - host_client->color_change_time < 1)
			return;

		if (pq_chat_frags_to_talk.value && connected_time < 90 && host_client->old_frags < pq_chat_frags_to_talk.value)
		{
			SV_ClientPrintf ("Server: Play some and then you can talk\n");
			return;
		}

		// JPG 3.20 - optionally remove '\r'
		if (pq_remove_cr.value)
		{
			char *ch;
			for (ch = p ; *ch ; ch++)
			{
				if (*ch == '\r')
					*ch += 128;
			}
		}

#ifdef SUPPORTS_PQ_WORD_FILTER // Baker change
		if (pq_chat_word_filter.string[0] != '0' && pq_chat_word_filter.string[0] && (text_filtered = WordFilter_Check (p)) )
			p = text_filtered;
#endif // Baker change + SUPPORTS_PQ_WORD_FILTER


		// JPG 3.11 - feature request from Slot Zero
		if (pq_chat_log_player_number.value)
			Dedicated_Printf ("(%s %s) #%d  ", NET_QSocketGetAddressString(host_client->netconnection), text_filtered ? "word filtered": "", NUM_FOR_EDICT(host_client->edict)  );

		if (pr_teamplay.value && teamonly) // JPG - added () for mm2
			c_snprintf (text, "\001(%s): ", save->name);
		else c_snprintf (text, "\001%s: ", save->name);

	}
	else
		c_snprintf (text, "\001<%s> ", hostname.string);

	j = sizeof(text) - 2 - strlen(text);  // -2 for /n and null terminator
	if ((int)strlen(p) > j)
		p[j] = 0;

	c_strlcat (text, p);
	c_strlcat (text, "\n");

	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client || !client->active || !client->spawned)
			continue;
		if (pr_teamplay.value && teamonly && client->edict->v.team != save->edict->v.team)
			continue;
		host_client = client;
		SV_ClientPrintf("%s", text);
	}
	host_client = save;

	// JPG 3.20 - optionally write player binds to server log
	if (pq_chat_to_log.value)
		Con_Printf ("(%s) %s", NET_QSocketGetAddressString(host_client->netconnection),  &text[1]);
	else Dedicated_Printf ("%s", &text[1]);
}


void Host_Say_f (lparse_t *line)
{
	Host_Say (line, false);
}


void Host_Say_Team_f (lparse_t *line)
{
	Host_Say (line, true);
}


void Host_Tell_f (lparse_t *line)
{
	int			j;
	client_t	*client;
	client_t	*save;
	char		*p;
	char		text[64];
	size_t		offsetz;

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer (line);
		return;
	}

	if (line->count < 3)
		return;

	c_strlcpy (text, host_client->name);
	c_strlcat (text, ": ");

	offsetz = line->args[2] - line->chopped; // Offset into line after whitespace
	p = &line->original[offsetz]; // evil

// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

// check length & truncate if necessary
	j = sizeof(text) - 2 - strlen(text);  // -2 for /n and null terminator
	if ((int)strlen(p) > j)
		p[j] = 0;

	c_strlcat (text, p);
	c_strlcat (text, "\n");

	save = host_client;
	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client->active || !client->spawned)
			continue;
		if (strcasecmp(client->name, line->args[1]))
			continue;
		host_client = client;
		SV_ClientPrintf("%s", text);
		break;
	}
	host_client = save;
}


/*
==================
Host_Color_f
==================
*/
void Host_Color_f (lparse_t *line)
{
	int		top, bottom;
	int		playercolor;

	if (line->count == 1)
	{
		Con_Printf ("\"color\" is \"%i %i\"\n", ((int)cl_color.value) >> 4, ((int)cl_color.value) & 0x0f);
		Con_Printf ("color <0-13> [0-13]\n");
		return;
	}

	if (line->count == 2)
		top = bottom = atoi(line->args[1]);
	else
	{
		top = atoi(line->args[1]);
		bottom = atoi(line->args[2]);
	}

	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;

#ifdef SUPPORTS_COOP_ENHANCEMENTS
	if (vm_coop_enhancements.value &&  cmd_source != src_command && sv.active && pr_global_struct->coop && pr_teamplay.value)
	{
		if (isDedicated)
			bottom = 12; // Dedicated, you get yellow I guess.
		else bottom = (int)cl_color.value & 15;
	}
#endif // SUPPORTS_COOP_ENHANCEMENTS

	playercolor = top*16 + bottom;

	if (cmd_source == src_command)
	{
		Cvar_SetValueQuick (&cl_color, playercolor);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer (line);
		return;
	}

	host_client->colors = playercolor;
	host_client->edict->v.team = bottom + 1;

// send notification to all clients
	MSG_WriteByte (&sv.reliable_datagram, svc_updatecolors);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteByte (&sv.reliable_datagram, host_client->colors);
}

/*
==================
Host_Kill_f
==================
*/
void Host_Kill_f (lparse_t *line)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer (line);
		return;
	}

	if (sv_player->v.health <= 0)
	{
		SV_ClientPrintf ("Can't suicide -- already dead!\n");
		return;
	}

	pr_global_struct->time = sv.time;
	pr_global_struct->self = EDICT_TO_PROG(sv_player);
	PR_ExecuteProgram (pr_global_struct->ClientKill);
}


/*
==================
Host_Pause_f
==================
*/

void Host_Pause_f (lparse_t *line)
{
	// If playing back a demo, we pause now
	if (cls.demoplayback && cls.demonum == -1) // Don't allow startdemos to be paused
		cl.paused ^= 2;		// to handle demo-pause
	if (cmd_source == src_command)
	{
		if (!cls.demoplayback)
			Cmd_ForwardToServer (line);
		return;
	}

	if (!sv_pausable.value)
		SV_ClientPrintf ("Pause not allowed.\n");
	else
	{
		// If not playing back a demo, we pause here
		if (!cls.demoplayback) // Don't allow startdemos to be paused
			cl.paused ^= 2;		// to handle demo-pause
		sv.paused ^= 1;

		if (sv.paused)
		{
			SV_BroadcastPrintf ("%s paused the game\n", PR_GetString( sv_player->v.netname));
		}
		else
		{
			SV_BroadcastPrintf ("%s unpaused the game\n",PR_GetString( sv_player->v.netname));
		}

	// send notification to all clients
		MSG_WriteByte (&sv.reliable_datagram, svc_setpause);
		MSG_WriteByte (&sv.reliable_datagram, sv.paused);
	}
}

//===========================================================================


/*
==================
Host_PreSpawn_f
==================
*/
void Host_PreSpawn_f (lparse_t *unused)
{
	if (cmd_source == src_command)
	{
		Con_Printf ("prespawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Printf ("prespawn not valid -- already spawned\n");	// JPG 3.02 already->already
		return;
	}

	SZ_Write (&host_client->message, sv.signon.data, sv.signon.cursize);
	MSG_WriteByte (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 2);
	host_client->sendsignon = true;
}

/*
==================
Host_Spawn_f
==================
*/
void Host_Spawn_f (void)
{
	int		i;
	client_t	*client;
	edict_t	*ent;
#ifdef SUPPORTS_NEHAHRA
	func_t		RestoreGame;
    dfunction_t	*f;
#endif // SUPPORTS_NEHAHRA

	if (cmd_source == src_command)
	{
		Con_Printf ("spawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Printf ("Spawn not valid -- already spawned\n");	// JPG 3.02 already->already
		return;
	}

// run the entrance script
	if (sv.loadgame)
	{	// loaded games are fully inited already
		// if this is the last client to be connected, unpause
		sv.paused = false;

#ifdef SUPPORTS_NEHAHRA
		// nehahra stuff
	    if ((f = ED_FindFunction("RestoreGame"))) {
			if ((RestoreGame = (func_t)(f - pr_functions))) {
				Con_DPrintf ("Calling RestoreGame\n");
				pr_global_struct->time = sv.time;
				pr_global_struct->self = EDICT_TO_PROG(sv_player);
				PR_ExecuteProgram (RestoreGame);
			}
		}
#endif // SUPPORTS_NEHAHRA
	}
	else
	{
		// set up the edict
		ent = host_client->edict;

		memset (&ent->v, 0, progs->entityfields * 4);
		ent->v.colormap = NUM_FOR_EDICT(ent);
		ent->v.team = (host_client->colors & 15) + 1;
		ent->v.netname = PR_SetEngineString(host_client->name);

		// copy spawn parms out of the client_t
		for (i=0 ; i< NUM_SPAWN_PARMS ; i++)
			(&pr_global_struct->parm1)[i] = host_client->spawn_parms[i];

		// call the spawn function
		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->ClientConnect);

		if ((System_DoubleTime() - NET_QSocketGetTime(host_client->netconnection) ) <= sv.time)
			Dedicated_Printf ("%s entered the game\n", host_client->name);

		PR_ExecuteProgram (pr_global_struct->PutClientInServer);
	}


// send all current names, colors, and frag counts
	SZ_Clear (&host_client->message);

// send time of update
	MSG_WriteByte (&host_client->message, svc_time);
	MSG_WriteFloat (&host_client->message, sv.time);

	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		MSG_WriteByte (&host_client->message, svc_updatename);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteString (&host_client->message, client->name);
		MSG_WriteByte (&host_client->message, svc_updatefrags);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteShort (&host_client->message, client->old_frags);
		MSG_WriteByte (&host_client->message, svc_updatecolors);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteByte (&host_client->message, client->colors);
	}

// send all current light styles
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		MSG_WriteByte (&host_client->message, svc_lightstyle);
		MSG_WriteByte (&host_client->message, (char)i);
		MSG_WriteString (&host_client->message, sv.lightstyles[i]);
	}

//
// send some stats
//
	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALSECRETS);
	MSG_WriteLong (&host_client->message, pr_global_struct->total_secrets);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALMONSTERS);
	MSG_WriteLong (&host_client->message, pr_global_struct->total_monsters);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_SECRETS);
	MSG_WriteLong (&host_client->message, pr_global_struct->found_secrets);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_MONSTERS);
	MSG_WriteLong (&host_client->message, pr_global_struct->killed_monsters);

//
// send a fixangle
// Never send a roll angle, because savegames can catch the server
// in a state where it is expecting the client to correct the angle
// and it won't happen if the game was just loaded, so you wind up
// with a permanent head tilt
	ent = EDICT_NUM( 1 + (host_client - svs.clients) );
	MSG_WriteByte (&host_client->message, svc_setangle);

	if (sv.loadgame) // MH load game angles fix ...
	{
		MSG_WriteAngle (&host_client->message, ent->v.v_angle[0]);
		MSG_WriteAngle (&host_client->message, ent->v.v_angle[1]);
		MSG_WriteAngle (&host_client->message, 0 );
	}
	else
	{
		MSG_WriteAngle (&host_client->message, ent->v.angles[0] );
		MSG_WriteAngle (&host_client->message, ent->v.angles[1] );
		MSG_WriteAngle (&host_client->message, 0 );
	}


	SV_WriteClientdataToMessage (sv_player, &host_client->message);

	MSG_WriteByte (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 3);
	host_client->sendsignon = true;
}

/*
==================
Host_Begin_f
==================
*/
void Host_Begin_f (void)
{
	if (cmd_source == src_command)
	{
		Con_Printf ("begin is not valid from the console\n");
		return;
	}

	host_client->spawned = true;
}

//===========================================================================


/*
==================
Host_Kick_f

Kicks a user off of the server
==================
*/
void Host_Kick_f (lparse_t *line)
{
	const char		*who;
	const char		*message = NULL;
	client_t	*save;
	int			i;
	cbool	byNumber = false;

	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer (line);
			return;
		}
	}
	else if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	save = host_client;

	if (line->count > 2 && strcmp(line->args[1], "#") == 0)
	{
		i = atof(line->args[2]) - 1;
		if (i < 0 || i >= svs.maxclients)
			return;
		if (!svs.clients[i].active)
			return;
		host_client = &svs.clients[i];
		byNumber = true;
	}
	else
	{
		for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
		{
			if (!host_client->active)
				continue;
			if (strcasecmp(host_client->name, line->args[1]) == 0)
				break;
		}
	}

	if (i < svs.maxclients)
	{
		if (cmd_source == src_command)
			if (isDedicated)
				who = "Console";
			else
				who = cl_name.string;
		else
			who = save->name;

		// can't kick yourself!
		if (host_client == save)
			return;

		if (line->count > 2)
		{
			size_t offsetz = line->args[1] - line->chopped; // Offset into line after whitespace
			char *args_after_command = &line->original[offsetz];

			message = COM_Parse(args_after_command);
			if (byNumber)
			{
				message++;							// skip the #
				while (*message == ' ')				// skip white space
					message++;
				message += strlen(line->args[2]);	// skip the number
			}
			while (*message && *message == ' ')
				message++;
		}
		if (message)
			SV_ClientPrintf ("Kicked by %s: %s\n", who, message);
		else
			SV_ClientPrintf ("Kicked by %s\n", who);
		SV_DropClient (false);
	}

	host_client = save;
}

/*
===============================================================================

DEBUGGING TOOLS

===============================================================================
*/

/*
==================
Host_Give_f
==================
*/


void Host_Give_f (lparse_t *line)
{
	char		tbuf[256];
	const char	*t = tbuf;
	int			v;
	eval_t		*val;

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer (line);
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	// This allows sv_cheats -1 to disallow major cheats but still allow give
	// for cooperative play where a coop map might have too little ammo or health
	if (sv.disallow_minor_cheats && !host_client->privileged)
	{
		SV_ClientPrintf ("No cheats allowed, use sv_cheats 1 and restart level to enable.\n");
		return;
	}

	if (line->count == 1)
	{
		// Help
		Con_Printf ("usage: give <item> <quantity>\n");
		Con_Printf (" 1-8 = weapon, a = armor\n");
		Con_Printf (" h = health, silverkey, goldkey\n");
		Con_Printf (" s,n,r,c = ammo, rune1-rune4\n");
		Con_Printf (" rune/key toggles if held\n");
		return;
	}

	c_strlcpy (tbuf, line->args[1]);
	v = atoi (line->args[2]);

	     if (!strcmp(tbuf, "goldkey")) c_strlcpy (tbuf, "kg");
	else if (!strcmp(tbuf, "silverkey")) c_strlcpy (tbuf, "ks");
	else if (!strcmp(tbuf, "rune1")) c_strlcpy (tbuf, "q1");
	else if (!strcmp(tbuf, "rune2")) c_strlcpy (tbuf, "q2");
	else if (!strcmp(tbuf, "rune3")) c_strlcpy (tbuf, "q3");
	else if (!strcmp(tbuf, "rune4")) c_strlcpy (tbuf, "q4");



	switch (t[0])
	{
#if 1
	case 'k':
		// Baker: Give key will remove key if you have it, add key if you don't
		// Helps debug maps where you need to test trigger.
		if (t[1] && t[1] == 'g')
			sv_player->v.items = (int)sv_player->v.items ^ IT_KEY2;
		else sv_player->v.items = (int)sv_player->v.items ^ IT_KEY1;
		break;

	case 'q':
		{
			int sigil;
			switch (t[1])
			{
			case '1': sigil = 1;	break;
			case '2': sigil = 2;	break;
			case '3': sigil = 4;	break;
			case '4': sigil = 8;	break;
			default:  sigil = 0;	break;
			}
			if (sigil)
			{
				SV_ClientPrintf ("may require 'changelevel start' or equivalent for intended effect.\n");
				pr_global_struct->serverflags = (int)pr_global_struct->serverflags ^ sigil;
			}
			break;
		}
#endif
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
      // MED 01/04/97 added hipnotic give stuff
      if (com_gametype == gametype_hipnotic || com_gametype == gametype_quoth)
      {
         if (t[0] == '6')
         {
            if (t[1] == 'a')
               sv_player->v.items = (int)sv_player->v.items | HIT_PROXIMITY_GUN;
            else
               sv_player->v.items = (int)sv_player->v.items | IT_GRENADE_LAUNCHER;
         }
         else if (t[0] == '9')
            sv_player->v.items = (int)sv_player->v.items | HIT_LASER_CANNON;
         else if (t[0] == '0')
            sv_player->v.items = (int)sv_player->v.items | HIT_MJOLNIR;
         else if (t[0] >= '2')
            sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
      }
      else
      {
         if (t[0] >= '2')
            sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
      }
		break;

    case 's':
		if (com_gametype == gametype_rogue)
		{
			val = GETEDICTFIELDVALUE(sv_player, eval_ammo_shells1);
		    if (val)
			    val->_float = v;
		}
        sv_player->v.ammo_shells = v;
        break;

    case 'n':
		if (com_gametype == gametype_rogue)
		{
			val = GETEDICTFIELDVALUE(sv_player, eval_ammo_nails1);
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_nails = v;
			}
		}
		else
		{
			sv_player->v.ammo_nails = v;
		}
        break;

    case 'l':
		if (com_gametype == gametype_rogue)
		{
			val = GETEDICTFIELDVALUE(sv_player, eval_ammo_lava_nails);
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_nails = v;
			}
		}
        break;

    case 'r':
		if (com_gametype == gametype_rogue)
		{
			val = GETEDICTFIELDVALUE(sv_player, eval_ammo_rockets1);
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_rockets = v;
			}
		}
		else
		{
			sv_player->v.ammo_rockets = v;
		}
        break;

    case 'm':
		if (com_gametype == gametype_rogue)
		{
			val = GETEDICTFIELDVALUE(sv_player, eval_ammo_multi_rockets);
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_rockets = v;
			}
		}
        break;

    case 'h':
        sv_player->v.health = v;
        break;

    case 'c':
		if (com_gametype == gametype_rogue)
		{
			val = GETEDICTFIELDVALUE (sv_player, eval_ammo_cells1);
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_cells = v;
			}
		}
		else
		{
			sv_player->v.ammo_cells = v;
		}
        break;

    case 'p':
		if (com_gametype == gametype_rogue)
		{
			val = GETEDICTFIELDVALUE (sv_player, eval_ammo_plasma);
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_cells = v;
			}
		}
        break;

	//johnfitz -- give armour
    case 'a':
		if (v > 150)
		{
		    sv_player->v.armortype = 0.8;
	        sv_player->v.armorvalue = v;
			sv_player->v.items = sv_player->v.items -
				((int)(sv_player->v.items) & (int)(IT_ARMOR1 | IT_ARMOR2 | IT_ARMOR3)) +
				IT_ARMOR3;
		}
		else if (v > 100)
		{
		    sv_player->v.armortype = 0.6;
	        sv_player->v.armorvalue = v;
			sv_player->v.items = sv_player->v.items -
				((int)(sv_player->v.items) & (int)(IT_ARMOR1 | IT_ARMOR2 | IT_ARMOR3)) +
					IT_ARMOR2;
		}
		else if (v >= 0)
		{
		    sv_player->v.armortype = 0.3;
	        sv_player->v.armorvalue = v;
			sv_player->v.items = sv_player->v.items -
				 ((int)(sv_player->v.items) & (int)(IT_ARMOR1 | IT_ARMOR2 | IT_ARMOR3)) +
				IT_ARMOR1;
		}
		break;
	//johnfitz
    }

	//johnfitz -- update currentammo to match new ammo (so statusbar updates correctly)
	switch ((int)(sv_player->v.weapon))
	{
	case IT_SHOTGUN:
	case IT_SUPER_SHOTGUN:
		sv_player->v.currentammo = sv_player->v.ammo_shells;
		break;
	case IT_NAILGUN:
	case IT_SUPER_NAILGUN:
	case RIT_LAVA_SUPER_NAILGUN:
		sv_player->v.currentammo = sv_player->v.ammo_nails;
		break;
	case IT_GRENADE_LAUNCHER:
	case IT_ROCKET_LAUNCHER:
	case RIT_MULTI_GRENADE:
	case RIT_MULTI_ROCKET:
		sv_player->v.currentammo = sv_player->v.ammo_rockets;
		break;
	case IT_LIGHTNING:
	case HIT_LASER_CANNON:
	case HIT_MJOLNIR:
		sv_player->v.currentammo = sv_player->v.ammo_cells;
		break;
	case RIT_LAVA_NAILGUN: //same as IT_AXE
		if (com_gametype == gametype_rogue)
			sv_player->v.currentammo = sv_player->v.ammo_nails;
		break;
	case RIT_PLASMA_GUN: //same as HIT_PROXIMITY_GUN
		if (com_gametype == gametype_rogue)
			sv_player->v.currentammo = sv_player->v.ammo_cells;
		if (com_gametype == gametype_hipnotic || com_gametype == gametype_quoth)
			sv_player->v.currentammo = sv_player->v.ammo_rockets;
		break;
	}
	//johnfitz
}



/*
===============================================================================

DEMO LOOP CONTROL

===============================================================================
*/


/*
==================
Host_Startdemos_f
==================
*/
void Host_Startdemos_f (lparse_t *line)
{
	int		i, c;

	if (isDedicated)
	{
#pragma message ("Baker: Eliminate this somehow or automatically add 'map start' to dedicated server server BEFORE stuffcmds?")
#pragma message ("If I start a dedicated server with +map dm6, what stops this from happening?  The sv.active?")
		if (!sv.active)
			Cbuf_AddText ("map start\n");
		return;
	}

	if (!host_startdemos.value)
		return;

	// Baker: Old behavior restored.
	if (cls.demonum == -1)
		return;

	c = line->count - 1;
	if (c > MAX_DEMOS)
	{
		Con_Printf ("Max %i demos in demoloop\n", MAX_DEMOS);
		c = MAX_DEMOS;
	}

	if (line->count != 1)
	{
		cls.demonum = 0;

	}
	Con_Printf ("%i demo(s) in loop\n", c);

	for (i = 1 ; i < c + 1 ; i++)
		strlcpy (cls.demos[i - 1], line->args[i], sizeof(cls.demos[0]));

	// LordHavoc: clear the remaining slots
	for (;i <= MAX_DEMOS;i++)
		cls.demos[i-1][0] = 0;
/*
	if (line->count == 0)
	{
		for (i = 1;i <= MAX_DEMOS;i++)
		cls.demos[i-1][0] = 0;
		CL_Clear_Demos_Queue ();
		cls.demonum = -1;
		CL_Disconnect ();
		CL_NextDemo (); // Baker attempt
		CL_Disconnect ();


	}
*/
	if (!sv.active && cls.demonum != -1 && !cls.demoplayback && cls.state != ca_connected)
	{
		cls.demonum = 0;
		CL_NextDemo ();
	}
	else
	{
		cls.demonum = -1;
	}
}



/*
==================
Host_Stopdemo_f

Return to looping demos
==================
*/
void Host_Stopdemo_f (lparse_t *unused)
{
	if (isDedicated)
		return;

	if (!cls.demoplayback)
		return;
	CL_StopPlayback ();

// Baker :Since this is an intentional user action,
// clear the demos queue.
	CL_Clear_Demos_Queue ();

	CL_Disconnect ();
}

//=============================================================================




