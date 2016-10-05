/*
Copyright (C) 2009-2013 Baker

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
// lists.c -- Lists (maps, demos, games, keys, saves)

#include "quakedef.h"


//	int	unused;	const char	*description, *extension, *associated_commands;
list_info_t list_info[] =
{
	{ list_type_none,		"commands/aliases/vars",NULL,		NULL,		},
	{ list_type_config,		"config",	".cfg",		"exec"					},
	{ list_type_demo,		"demo",		".dem",		"playdemo,capturedemo"	},
	{ list_type_demos_menu,	"demos",	".dem",		"none"					},
	{ list_type_game,		"game",		NULL,		"game,uninstall",		},
	{ list_type_game_hud,	"-game",	NULL,		"game"					},
	{ list_type_give,		"give",		NULL,		"give"					},
	{ list_type_key,		"key",		NULL,		"bind,unbind"			},
	{ list_type_map,		"map",		".bsp",		"map,changelevel"		},
	{ list_type_map2,		"map",		".bsp",		"map,changelevel,sv_map_rotation"		},
	{ list_type_levels_menu,"levels",	".bsp",		"none"					},
	{ list_type_savegame,	"save game",".sav",		"save,load"				},
	{ list_type_sky,		"sky",		"rt.tga",	"sky"					},
	{ list_type_mp3,		"setmusic",	".mp3",		"sky"					},
	{ list_type_sound,		"sound",	".wav",		"play,playvol"			},
#ifdef GLQUAKE_TEXTUREMODES
	{ list_type_texmode,	"texmode",	NULL,		"gl_texturemode"		},
#endif // GLQUAKE_TEXTUREMODES
};


static void List_QPrint (list_info_t* list_info_item)
{
	int count;
	clist_t	*cursor;

	for (cursor = list_info_item->plist, count = 0; cursor; cursor = cursor->next, count ++)
		Con_SafePrintf ("   %s\n", cursor->name);

	if (count)
		Con_SafePrintf ("%i %s(s)\n", count, list_info_item->description);
	else Con_SafePrintf ("no %ss found\n", list_info_item->description);
}


//==============================================================================
//johnfitz -- modlist management
//==============================================================================

void Lists_Update_ModList (void)
{
	DIR				*dir_p;
	struct dirent	*dir_t;
	char			dir_string[MAX_OSPATH];

	FS_FullPath_From_Basedir (dir_string, "");
//	c_snprintf (dir_string, "%s/", com_basedir); // Fill in the com_basedir into dir_string

	dir_p = opendir(dir_string);
	if (dir_p == NULL)
		return;

	if (&list_info[list_type_game].plist)
	{
		List_Free (&list_info[list_type_game].plist);
		if (list_info[list_type_game].plist) System_Error ("Not freed?");
	}

	while ((dir_t = readdir(dir_p)) != NULL)
	{
		cbool		progs_found = false, pak_found = false, subfolder_found = false;
		char			mod_dir_string[MAX_OSPATH];
		DIR				*mod_dir_p;
		struct dirent	*mod_dir_t;

		// Baker: Ignore "." and ".."
		if (!strcmp(dir_t->d_name, ".") || !strcmp(dir_t->d_name, "..") )
			continue;

		// Baker: Try to open the directory, if not it is a file.
		c_snprintf2 (mod_dir_string, "%s%s/", dir_string, dir_t->d_name);
		mod_dir_p = opendir (mod_dir_string);
		if (mod_dir_p == NULL)
			continue;

		// Baker: We want to find evidence this is playable and not just a folder.
		// Baker: Find either a progs.dat, a pak file or .bsp or textures
		{
			const char *sub_folders[] = {"maps","sound","textures","progs", "music", NULL}; // Baker: should cover it
			int i;
			for (i = 0; sub_folders[i]; i++ )
			{
				const char *cur = sub_folders[i];
				DIR *mod_subdir_p;

				c_snprintf3 (mod_dir_string, "%s%s/%s/", dir_string, dir_t->d_name, cur);
//				System_Alert (mod_dir_string);
#if 1
				// A super long directory name isn't supported by Quake.
				if (strlen(dir_t->d_name) > MAX_QPATH)
					continue;
#endif

				mod_subdir_p = opendir (mod_dir_string);
				if (mod_subdir_p)
				{
					// Found content
					subfolder_found = true;
					closedir (mod_subdir_p);
					break;
				}

			}
		}

		if (!subfolder_found)
		{
			// find progs.dat and pak file(s)
			while ((mod_dir_t = readdir(mod_dir_p)) != NULL)
			{
				if (!strcmp(mod_dir_t->d_name, ".") || !strcmp(mod_dir_t->d_name, ".."))
					continue;
				if (strcasecmp(mod_dir_t->d_name, "progs.dat") == 0)
					progs_found = true;
				if (strstr(mod_dir_t->d_name, ".pak") || strstr(mod_dir_t->d_name, ".PAK"))
					pak_found = true;
				if (progs_found || pak_found)
					break;
			}
		}

		closedir(mod_dir_p);

		// Baker: If we didn't find a progs, pak or subfolder like "maps", skip this.
		if (!subfolder_found && !progs_found && !pak_found)
			continue;
		List_Add(&list_info[list_type_game].plist, dir_t->d_name);
	}

	closedir(dir_p);
}




typedef const char *(*constcharfunc_t) (void);
static void List_Func_Rebuild (clist_t** list, constcharfunc_t runFunction)
{
	const char *namestring;
	if (*list)
	{
		List_Free (list);
		if (*list) System_Error ("Not freed?");
	}

	while ( (namestring = runFunction()    ) )
	{
		List_Add (list, namestring);
//		Con_Printf ("Adding %s\n", namestring);
	}

}

void Shareware_Notify (void)
{
	if (!static_registered)
		Con_Printf ("\nNote: You are using shareware.\nCustom maps and games are not available.\n\n");
}

void List_Configs_f (void)		{ List_QPrint (&list_info[list_type_config]);  }
void List_Demos_f (void)		{ List_QPrint (&list_info[list_type_demo]); }
void List_Games_f (void) 		{ Lists_Update_ModList ();  List_QPrint (&list_info[list_type_game]); Shareware_Notify ();  }
void List_Game_Huds_f (void)	{ List_QPrint (&list_info[list_type_game_hud]); }
void List_Keys_f (void)			{ List_QPrint (&list_info[list_type_key]); }
void List_Maps_f (void)	 		{ List_QPrint (&list_info[list_type_map]); Shareware_Notify (); }
void List_Savegames_f (void)	{ List_QPrint (&list_info[list_type_savegame]); }
void List_Skys_f (void)			{ List_QPrint (&list_info[list_type_sky]); Con_Printf ("Located in gfx/env folder\n"); }
void List_MP3s_f (void)			{ List_QPrint (&list_info[list_type_mp3]); Con_Printf ("Located in music folder\n"); }

void List_Sounds_f (void)		{ List_QPrint (&list_info[list_type_sound]); Con_Printf ("These are cached sounds only -- loaded in memory\n"); }




// These rely on the cache
void Lists_Refresh_NewMap (void)
{
	List_Func_Rebuild (&list_info[list_type_sound].plist, S_Sound_ListExport);
}

// For when demo record is finished
void Lists_Update_Demolist (void)
{
	List_Filelist_Rebuild (&list_info[list_type_demo].plist, "", list_info[list_type_demo].extension, 0, SPECIAL_GAMEDIR_ONLY_IF_COUNT);
}

void Lists_Update_Savelist (void)
{
	List_Filelist_Rebuild (&list_info[list_type_savegame].plist, "", list_info[list_type_savegame].extension, 0, SPECIAL_GAMEDIR_ONLY);
}

char game_startmap[MAX_QPATH];
cbool game_map_tiers;
enum startmap_e
{
	startmap_named_start = 0,
	startmap_undergate_intro_rockgate,
	startmap_start_in_name,
	startmap_same_as_gamedir,
	startmap_alphabest,
	startmap_nothing,
};

void Lists_Update_Maps (void)
{
	game_map_tiers = List_Filelist_Rebuild (&list_info[list_type_map].plist, "/maps", list_info[list_type_map].extension, 32*1024, SPECIAL_GAMEDIR_ONLY_IF_COUNT);
	if (game_map_tiers)
		List_Filelist_Rebuild (&list_info[list_type_map2].plist, "/maps", list_info[list_type_map2].extension, 32*1024, SPECIAL_GAMEDIR_TIER_2);
#pragma message ("We need some sort of general completion hint for single player mods that should play anything")
	// else clear
	// Determine the start map

	// Find a map named start, then if only 1 that one, then undergate, then one with "start" in the name, last resort: top alphabetical

	{
		enum startmap_e best_type = startmap_nothing;
		clist_t	*best = NULL;
		clist_t	*cursor;

		for (cursor = (&list_info[list_type_map])->plist; cursor; cursor = cursor->next)
		{
			if (best_type > startmap_named_start && !strcmp (cursor->name, "start"))
			{
				best = cursor;
				best_type = startmap_named_start;
				break; // This cannot be beat
			}
			else if (best_type > startmap_undergate_intro_rockgate && (!strcmp (cursor->name, "undergate") || !strcmp (cursor->name, "intro") || !strcmp (cursor->name, "rockgate")) )
			{
				best = cursor;
				best_type = startmap_undergate_intro_rockgate;
			}
			else if (best_type > startmap_start_in_name && strstr (cursor->name, "start") )
			{
				best = cursor;
				best_type = startmap_start_in_name;
			}
			else if (best_type > startmap_same_as_gamedir && strstr (cursor->name, gamedir_shortname() ) )
			{
				best = cursor;
				best_type = startmap_same_as_gamedir;
			}
			else if (best_type > startmap_alphabest) // Baker: This should work because map list is alpha sorted
			{
				best = cursor;
				best_type = startmap_alphabest;
			}
		}

		c_strlcpy (game_startmap, best->name);
		Con_DPrintf ("Start map determined to be %s\n", game_startmap);
	}

}

void Lists_Update_Levels_Menu (void)
{
	List_Filelist_Rebuild (&list_info[list_type_levels_menu].plist, "/maps", list_info[list_type_levels_menu].extension, 32*1024, SPECIAL_GAMEDIR_PREFERENCE);
}

void Lists_Update_Demos_Menu (void)
{
	List_Filelist_Rebuild (&list_info[list_type_demos_menu].plist, "", list_info[list_type_demos_menu].extension, 0, SPECIAL_GAMEDIR_ONLY);
}


void Lists_NewGame (void)
{
	// Only demos and maps should be here (for menu).  That won't be regenerating in real time.
	Lists_Update_ModList (); // Works
	Lists_Refresh_NewMap ();
	Lists_Update_Demolist();
	Lists_Update_Savelist ();

	Lists_Update_Maps ();
	List_Filelist_Rebuild (&list_info[list_type_sky].plist, "/gfx/env", list_info[list_type_sky].extension, 0, 4);
	List_Filelist_Rebuild (&list_info[list_type_mp3].plist, "/music", list_info[list_type_mp3].extension, 0, 0);
	List_Filelist_Rebuild (&list_info[list_type_config].plist, "", list_info[list_type_config].extension, 0, 2);

	M_Menu_Levels_NewGame ();
	M_Menu_Demos_NewGame ();
#ifdef CORE_PTHREADS
	ReadList_NewGame ();	
#endif // CORE_PTHREADS
}

static const char *gamehuds[] =
{
	"-nehahra",
	"-hipnotic",
	"-rogue",
	"-quoth",
};
#define NUM_GAMEHUDS (int)(sizeof(gamehuds)/sizeof(gamehuds[0]))


const char *GameHud_ListExport (void)
{
	static char returnbuf[32];

	static int last = -1; // Top of list.
	// We want first entry >= this
	int		wanted = CLAMP(0, last + 1, NUM_GAMEHUDS );  // Baker: bounds check

	int i;

	for (i = wanted; i < NUM_GAMEHUDS ; i++)
	{

		if (i >= wanted) // Baker: i must be >=want due to way I setup this function
		{
			c_strlcpy (returnbuf, gamehuds[i]);
			String_Edit_To_Lower_Case (returnbuf); // Baker: avoid hassles with uppercase keynames

			last = i;
//			Con_Printf ("Added %s\n", returnbuf);
			return returnbuf;
		}
	}

	// Not found, reset to top of list and return NULL
	last = -1;
	return NULL;
}

static const char *give_strings[] = {
	"armor",
	"cells",
	"goldkey",		// trans
	"health",
	"nails",
	"rockets",
	"rune1",		// trans
	"rune2",		// trans
	"rune3",		// trans
	"rune4",		// trans
	"shells",
	"silverkey",	// trans
}; ARRAY_SIZE(give_strings);

const char *Give_ListExport (void)
{
	static char returnbuf[32];
	
	static int last = -1; // Top of list.
	// We want first entry >= this
	int	wanted = CLAMP(0, last + 1, _numgive_strings);  // Baker: bounds check

	int i;

	for (i = wanted; i < _numgive_strings ; i++)
	{

		if (i >= wanted) // Baker: i must be >=want due to way I setup this function
		{
			c_strlcpy (returnbuf, give_strings[i]);
			String_Edit_To_Lower_Case (returnbuf); // Baker: avoid hassles with uppercase keynames

			last = i;
//			Con_Printf ("Added %s\n", returnbuf);
			return returnbuf;
		}
	}

	// Not found, reset to top of list and return NULL
	last = -1;
	return NULL;
}

void Lists_Init (void)
{
	Cmd_AddCommands (Lists_Init);

	Lists_NewGame ();

	Lists_Update_ModList ();
	List_Func_Rebuild (&list_info[list_type_key].plist, Key_ListExport);
	List_Func_Rebuild (&list_info[list_type_game_hud].plist, GameHud_ListExport);
	List_Func_Rebuild (&list_info[list_type_give].plist, Give_ListExport);
#ifdef GLQUAKE_TEXTUREMODES
	List_Func_Rebuild (&list_info[list_type_texmode].plist, TexMgr_TextureModes_ListExport);
#endif // GLQUAKE_TEXTUREMODES

}

