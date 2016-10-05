/*
Copyright (C) 2000	LordHavoc, Ender
Copyright (C) 2014	Baker

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
// nehahra.c

#include "quakedef.h"

#ifdef SUPPORTS_NEHAHRA

#ifdef DIRECT_SOUND_QUAKE
#include "winquake.h"
#endif
#include <fmod/fmod.h>
#include <fmod/fmod_errors.h>
#ifndef _WIN32
#include <dlfcn.h>
#endif

static cbool modplaying = false;

// cutscene demo usage
cvar_t  nehx00	= {"nehx00", "0"};
cvar_t	nehx01	= {"nehx01", "0"};
cvar_t  nehx02	= {"nehx02", "0"};
cvar_t	nehx03	= {"nehx03", "0"};
cvar_t  nehx04	= {"nehx04", "0"};
cvar_t	nehx05	= {"nehx05", "0"};
cvar_t  nehx06	= {"nehx06", "0"};
cvar_t	nehx07	= {"nehx07", "0"};
cvar_t  nehx08	= {"nehx08", "0"};
cvar_t	nehx09	= {"nehx09", "0"};
cvar_t  nehx10	= {"nehx10", "0"};
cvar_t	nehx11	= {"nehx11", "0"};
cvar_t  nehx12	= {"nehx12", "0"};
cvar_t	nehx13	= {"nehx13", "0"};
cvar_t  nehx14	= {"nehx14", "0"};
cvar_t	nehx15	= {"nehx15", "0"};
cvar_t  nehx16	= {"nehx16", "0"};
cvar_t	nehx17	= {"nehx17", "0"};
cvar_t  nehx18	= {"nehx18", "0"};
cvar_t	nehx19	= {"nehx19", "0"};
cvar_t  cutscene = {"cutscene", "1"};

int	num_sfxorig;
void Neh_CheckMode (void);

FMUSIC_MODULE	*musicHandle = NULL;

static signed char (F_API *qFSOUND_Init)(int, int, unsigned int);
static signed char (F_API *qFSOUND_SetBufferSize)(int);
static int (F_API *qFSOUND_GetMixer)(void);
static signed char (F_API *qFSOUND_SetMixer)(int);
static int (F_API *qFSOUND_GetError)(void);
static void (F_API *qFSOUND_Close)(void);
static FMUSIC_MODULE * (F_API *qFMUSIC_LoadSongEx)(const char *, int, int, unsigned int, const int *, int);
static signed char (F_API *qFMUSIC_FreeSong)(FMUSIC_MODULE *);
static signed char (F_API *qFMUSIC_PlaySong)(FMUSIC_MODULE *);
static signed char (F_API *qFMUSIC_SetPaused)(FMUSIC_MODULE *, signed char); // Baker
static signed char (F_API *qFMUSIC_SetMasterVolume)(FMUSIC_MODULE *, int); // Baker


#ifdef _WIN32
static	HINSTANCE fmod_handle = NULL;
#else
static	void	*fmod_handle = NULL;
#endif
static cbool fmod_loaded;

#ifdef _WIN32
#define FSOUND_GETFUNC(f, g) (qFSOUND_##f = (void *)GetProcAddress(fmod_handle, "_FSOUND_" #f #g))
#define FMUSIC_GETFUNC(f, g) (qFMUSIC_##f = (void *)GetProcAddress(fmod_handle, "_FMUSIC_" #f #g))
#else
#define FSOUND_GETFUNC(f, g) (qFSOUND_##f = (void *)dlsym(fmod_handle, "FSOUND_" #f))
#define FMUSIC_GETFUNC(f, g) (qFMUSIC_##f = (void *)dlsym(fmod_handle, "FMUSIC_" #f))
#endif

void FMOD_LoadLibrary (void)
{
	fmod_loaded = false;

#ifdef _WIN32
	if (!(fmod_handle = LoadLibrary("nehahra/fmod.dll")))
#else
	if (!(fmod_handle = dlopen("libfmod-3.73.so", RTLD_NOW)))
#endif
	{
		Con_Warning ("FMOD module not found ( nehahra/fmod.dll )\n");
		goto fail;
	}

	FSOUND_GETFUNC(Init, @12);
	FSOUND_GETFUNC(SetBufferSize, @4);
	FSOUND_GETFUNC(GetMixer, @0);
	FSOUND_GETFUNC(SetMixer, @4);
	FSOUND_GETFUNC(GetError, @0);
	FSOUND_GETFUNC(Close, @0);
	FMUSIC_GETFUNC(LoadSongEx, @24);
	FMUSIC_GETFUNC(FreeSong, @4);
	FMUSIC_GETFUNC(PlaySong, @4);
	FMUSIC_GETFUNC(SetPaused, @8); // Baker
	FMUSIC_GETFUNC(SetMasterVolume, @8); // Baker

	fmod_loaded = qFSOUND_Init && qFSOUND_SetBufferSize && qFSOUND_GetMixer &&
			qFSOUND_SetMixer && qFSOUND_GetError && qFSOUND_Close &&
			qFMUSIC_LoadSongEx && qFMUSIC_FreeSong && qFMUSIC_PlaySong && qFMUSIC_SetPaused && qFMUSIC_SetMasterVolume;

	if (!fmod_loaded)
	{
		Con_Warning ("FMOD module not initialized\n");
		goto fail;
	}

	Con_Printf ("FMOD module initialized\n");
	return;

fail:
	if (fmod_handle)
	{
#ifdef _WIN32
		FreeLibrary (fmod_handle);
#else
		dlclose (fmod_handle);
#endif
		fmod_handle = NULL;
	}
}

void FMOD_Volume_Think (void)
{
	static float old_sfxvolume = -1;
	static float old_bgmvolume = -1;
	int newvolume;

	if (!modplaying) // Can't act on it anyway.
		return;

	if (sfxvolume.value == old_sfxvolume && bgmvolume.value == old_bgmvolume)
		return;

	newvolume = (bgmvolume.value * sfxvolume.value) * 255; // bgm can go 0-3, sfx - 0-1
	newvolume = CLAMP (0, newvolume, 256);
	Con_DPrintf ("FMOD volume changed to %i\n", (int)newvolume);

	qFMUSIC_SetPaused (musicHandle, true);
	qFMUSIC_SetMasterVolume (musicHandle, newvolume);
	qFMUSIC_SetPaused (musicHandle, false);

	old_sfxvolume = sfxvolume.value;
	old_bgmvolume = bgmvolume.value;
}

void FMOD_Pause (void)
{
	if (modplaying)
		qFMUSIC_SetPaused (musicHandle, true);
}


void FMOD_Resume (void)
{
	if (modplaying)
		qFMUSIC_SetPaused (musicHandle, false);

}


void FMOD_Stop (void)
{
	if (modplaying && qFMUSIC_FreeSong)
		qFMUSIC_FreeSong (musicHandle);

	modplaying = false;
}

void FMOD_Stop_f (lparse_t *unused)
{
	FMOD_Stop ();
}


void FMOD_Play_f (lparse_t *line)
{
	char	modname[256], *buffer;
	int	mark;

	c_strlcpy (modname, line->args[1]);

	if (modplaying)
		FMOD_Stop ();

	if (strlen(modname) < 3)
	{
		Con_Printf ("Usage: %s <filename.ext>", line->args[1]);
		return;
	}

	mark = Hunk_LowMark ();

	if (!(buffer = (char *)COM_LoadHunkFile(modname)))
	{
		Con_Printf ("ERROR: Couldn't open %s\n", modname);
		return;
	}

	musicHandle = qFMUSIC_LoadSongEx (buffer, 0, com_filesize, FSOUND_LOADMEMORY, NULL, 0);

	Hunk_FreeToLowMark (mark);

	if (!musicHandle)
	{
		Con_Printf ("%s\n", FMOD_ErrorString(qFSOUND_GetError()));
		return;
	}

	modplaying = true;
	FMOD_Volume_Think ();
	qFMUSIC_PlaySong (musicHandle);
}

void FMOD_Init (void)
{
//	qFSOUND_SetBufferSize (300);
	if (!qFSOUND_Init(11025, 32, 0))
	{
		Con_Printf ("%s\n", FMOD_ErrorString(qFSOUND_GetError()));
		return;
	}
#pragma message ("What do about these guys.  Command orphans")

	Cmd_AddCommand ("stopmod", FMOD_Stop_f);
	Cmd_AddCommand ("playmod", FMOD_Play_f);
}

void FMOD_Close (void)
{
	if (fmod_loaded && qFSOUND_Close)
		qFSOUND_Close ();

	fmod_loaded = false;

#if 1
	if (fmod_handle)
	{
#ifdef _WIN32
		FreeLibrary (fmod_handle);
#else
		dlclose (fmod_handle);
#endif
		fmod_handle = NULL;
	}
#endif
#pragma message ("What do about these guys")
	Cmd_RemoveCommand ("stopmod");
	Cmd_RemoveCommand ("playmod");

	Con_Printf ("FMOD Shutdown.\n");
}



extern int host_hunklevel;
int nehahra_saved_hunk_level;
void Nehahra_Shutdown (void)
{
	if (!nehahra_active)
		return;

	Cmd_RemoveCommand ("gl_fogenable");
	Cmd_RemoveCommand ("gl_fogdensity");
	Cmd_RemoveCommand ("gl_fogred");
	Cmd_RemoveCommand ("gl_foggreen");
	Cmd_RemoveCommand ("gl_fogblue");
	Cmd_RemoveCommand ("loadsky");
	Cmd_RemoveCommand ("nextchase");
	Cmd_RemoveCommand ("play2");

	Cmd_RemoveCommand ("gl_notrans");
	Cmd_RemoveCommand ("r_oldsky");
	Cmd_RemoveCommand ("r_nospr32");

	// Nehahra uses these to pass data around cutscene demos
	Cvar_UnregisterVariable (&nehx00);
	Cvar_UnregisterVariable (&nehx01);
	Cvar_UnregisterVariable (&nehx02);
	Cvar_UnregisterVariable (&nehx03);
	Cvar_UnregisterVariable (&nehx04);
	Cvar_UnregisterVariable (&nehx05);
	Cvar_UnregisterVariable (&nehx06);
	Cvar_UnregisterVariable (&nehx07);
	Cvar_UnregisterVariable (&nehx08);
	Cvar_UnregisterVariable (&nehx09);
	Cvar_UnregisterVariable (&nehx10);
	Cvar_UnregisterVariable (&nehx11);
	Cvar_UnregisterVariable (&nehx12);
	Cvar_UnregisterVariable (&nehx13);
	Cvar_UnregisterVariable (&nehx14);
	Cvar_UnregisterVariable (&nehx15);
	Cvar_UnregisterVariable (&nehx16);
	Cvar_UnregisterVariable (&nehx17);
	Cvar_UnregisterVariable (&nehx18);
	Cvar_UnregisterVariable (&nehx19);
	Cvar_UnregisterVariable (&cutscene);

	FMOD_Close ();

	host_hunklevel = nehahra_saved_hunk_level;
	nehahra_saved_hunk_level = 0;

	nehahra_active = false;
}



void Nehahra_Init (void)
{
	void S_Play2_f (void);

	if (!(com_gametype == gametype_nehahra))
		return;

	if (nehahra_active)
	{
		Con_Warning ("Recursive Nehahra Init!");
		return;
	}

	nehahra_active = true;
	nehahra_saved_hunk_level = Hunk_LowMark ();

//	Cmd_AddCommands (Nehahra_Init);
	/*
	Cmd_AddCommand ("gl_fogenable", Neh_No_Command);
	Cmd_AddCommand ("gl_fogdensity", Neh_No_Command);
	Cmd_AddCommand ("gl_fogred", Neh_No_Command);
	Cmd_AddCommand ("gl_foggreen", Neh_No_Command);
	Cmd_AddCommand ("gl_fogblue", Neh_No_Command);
	Cmd_AddCommand ("nextchase", Neh_No_Command);

	Cmd_AddCommand ("play2", S_Play2_f);
	Cmd_AddCommand ("loadsky", Sky_SkyCommand_f);

	Cmd_AddCommand ("gl_notrans", Neh_No_Command);
	Cmd_AddCommand ("r_oldsky", Neh_No_Command);
	Cmd_AddCommand ("r_nospr32", Neh_No_Command);
	*/

	// Nehahra uses these to pass data around cutscene demos
	Cmd_AddCommands (Nehahra_Init);

	Neh_CheckMode ();

	FMOD_LoadLibrary ();
	if (fmod_loaded)
		FMOD_Init ();

	// Baker: We've added some commands so need to change the hunklow mark
	// We will restore it after we are done.
	Hunk_AllocName (0, "-NEH_HUNKLEVEL-");
	host_hunklevel = Hunk_LowMark ();

}

#if 0
void Neh_SetupFrame (void)
{
	if (gl_fogenable.value)
	{
		float	colors[4] = {gl_fogred.value, gl_foggreen.value, gl_fogblue.value, 1};
// nehahras glfog need multiplied by 64/100 because our fogstart won't divide it enough
		glFogi (GL_FOG_MODE, GL_EXP2);
		glFogf (GL_FOG_DENSITY, gl_fogdensity.value / 100);
		glFogfv (GL_FOG_COLOR, colors);
		glEnable (GL_FOG);
	}
	else
	{
		eglDisable (GL_FOG);
	}
}
#endif


#define SHOWLMP_MAXLABELS	256
typedef struct showlmp_s
{
	cbool	isactive;
	float		x;
	float		y;
	char		label[32];
	char		pic[128];
} showlmp_t;

showlmp_t	showlmp[SHOWLMP_MAXLABELS];

void SHOWLMP_decodehide (void)
{
	int	i;
	char	*lmplabel;

	lmplabel = MSG_ReadString ();
	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
	{
		if (showlmp[i].isactive && !strcmp(showlmp[i].label, lmplabel))
		{
			showlmp[i].isactive = false;
			return;
		}
	}
}

void SHOWLMP_decodeshow (void)
{
	int	i, k;
	char	lmplabel[256], picname[256];
	float	x, y;

	c_strlcpy (lmplabel, MSG_ReadString());
	c_strlcpy (picname, MSG_ReadString());
	x = MSG_ReadByte ();
	y = MSG_ReadByte ();
	k = -1;
	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
	{
		if (showlmp[i].isactive)
		{
			if (!strcmp(showlmp[i].label, lmplabel))
			{
				k = i;
				break;	// drop out to replace it
			}
		}
		else if (k < 0)	// find first empty one to replace
		{
			k = i;
		}
	}

	if (k < 0)
		return;	// none found to replace
	// change existing one
	showlmp[k].isactive = true;
	c_strlcpy (showlmp[k].label, lmplabel);
	c_strlcpy (showlmp[k].pic, picname);
	showlmp[k].x = x;
	showlmp[k].y = y;
}

void SHOWLMP_drawall (void)
{
	int	i;
// Baker what canvas?

	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
		if (showlmp[i].isactive)
			Draw_Pic (showlmp[i].x, showlmp[i].y, Draw_CachePic(showlmp[i].pic));
}

void SHOWLMP_clear (void)
{
	int	i;

	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
		showlmp[i].isactive = false;
}

void Neh_CheckMode (void)
{
// Check for game
	if (!COM_FindFile_Obsolete("maps/neh1m4.bsp"))
		System_Error ("You must specify the Nehahra game directory in -game!");
}

#endif // SUPPORTS_NEHAHRA



