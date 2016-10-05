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
// quakedef.h



#ifndef __QUAKEDEF_H__
#define __QUAKEDEF_H__

// quakedef.h -- primary header for client
#include <core.h>

//#define	QUAKE_GAME			// as opposed to utilities .. Baker: Moved to project level define

#define	QUAKE_VERSION			1.09
#define ENGINE_FAMILY_NAME		"Mark V"				// Config.cfg stamp
#define	ENGINE_VERSION			0.99
#define	GAMENAME				"id1"					// directory to look in by default
#define	CONFIG_CFG				"config.cfg"			// config name
#define	DOWNLOADS_FOLDER		"id1/_library"			// config name
#define	AUTOEXEC_CFG			"autoexec.cfg"			// autoexec.cfg name
#define	SETMUSIC_CFG			"setmusic.cfg"			// setmusic.cfg name, executed on any gamedir change and startup.
#define	SETMUSIC_CFG_FULL		"music/" SETMUSIC_CFG	// setmusic.cfg name, executed on any gamedir change and startup.
#define ENGINE_URL				"http://quakeone.com/markv/"

#define MEM_DEFAULT_MEMORY		(256 * 1024 * 1024)		//  256 MB
#define MEM_DEFAULT_DEDICATED	( 32 * 1024 * 1024)		//   32 MB
#define	MEM_DYNAMIC_SIZE		(  4 * 1024 * 1024)		//    4 MB

extern fn_set_t qfunction_set;

// Engine Family Name = "Mark V"
// Engine Window Name = "WinQuake Mark V"

// Baker:  Let's keep debugging consistent across platforms please!
#if defined(DEBUG) && !defined (_DEBUG)
#define _DEBUG
#endif // Baker: I'm ignoring NDEBUG for now.

// Baker: Determine sleep time
#define QUAKE_SLEEP_TIME 1 // Milliseconds
#if !defined (GLQUAKE) && !defined(_WIN32)
	#undef QUAKE_SLEEP_TIME
	#define QUAKE_SLEEP_TIME 50 // Baker: Software renderer without asm can eat up cpu
#endif

#ifdef DIRECT3D_WRAPPER
	#define TEMP_BASE_NAME "DirectX 8 " ENGINE_FAMILY_NAME
	#define ENGINE_SHOT_PREFIX "mark_v_"
#elif defined(GLQUAKE)
	#define TEMP_BASE_NAME ENGINE_FAMILY_NAME
	#define ENGINE_SHOT_PREFIX "mark_v_"
#else
	#define TEMP_BASE_NAME "WinQuake " ENGINE_FAMILY_NAME  // Prefix it WinQuake
	#define ENGINE_SHOT_PREFIX "winquake_mark_v_"
#endif

// ENGINE_NAME:
#ifdef _DEBUG
	#define ENGINE_NAME	  TEMP_BASE_NAME " Debug"			// Suffix it "Debug"
#else
	#define ENGINE_NAME	  TEMP_BASE_NAME
#endif

#ifdef DIRECT3D_WRAPPER
	#define RENDERER_NAME "DX8"
#elif defined(GLQUAKE)
	#define RENDERER_NAME "GL"
#else // Software
	#define RENDERER_NAME "SW"
#endif


#if defined(_WIN32) && !defined(SDLQUAKE)
    #define DIRECT_SOUND_QUAKE
#endif

// Baker: The following are 2 standard features that CodeBlocks + MinGW (gcc compiler) can't compile.
//		  I need to be able to disable them.  Visual Studio does them fine, obviously.
#define SUPPORTS_MP3_MUSIC				// Plays id1/music/track00.mp3 or rather <gamedir + search paths>/music/track<xx>.mp3 (Does not play in-pak mp3 files)
#define SUPPORTS_AVI_CAPTURE			// capturedemo command and friends
#define SUPPORTS_CD						// can use a cd

// Baker: The following more mark code than anything ...
#define SUPPORTS_NEHAHRA				// Nehahra (-game nehahra -nehahra)
#define SUPPORTS_XMEN_TUNEUP			// X-Men spams "CL".  Engine blocks that.
#define	SUPPORTS_CUTSCENE_PROTECTION	// Defends against cut-scenes mucking up cvars.  2 sources: demos and progs.  Doesn't address remote client.
#define SUPPORTS_BSP2_IMPROVEMENTS		// mnode_t, edict_t
//#define CUTSCENE_DEBUG
#define SUPPORTS_COOP_ENHANCEMENTS
#define SUPPORTS_LEVELS_MENU_HACK		// Baker: Sorry.
#define SUPPORTS_SERVER_PROTOCOL_15		// Ability to serve protocol 15 games

/*

	Baker: These mark differences in the code between GLQUAKE and WINQUAKE in my incarnation here, at least.

*/

// Baker: identifying recent critical bug-fixes

#define BUGFIX_DEMO_RECORD_BEFORE_CONNECTED_FIX		// Baker: Quite a bit of demo recording flexibility now.
#define BUGFIX_DEMO_RECORD_NOTHING_FIX				// Baker: A demo that didn't play a map would hang the rendering.

#ifdef GLQUAKE
	#define GLQUAKE_ALPHA_DRAWING
	#define GLQUAKE_CAPTIONS_AND_HIGHLIGHTS
	#define GLQUAKE_COLORED_LIGHTS
	#define GLQUAKE_COLORMAP_TEXTURES
	#define GLQUAKE_DRAWING_METHODS
	#define GLQUAKE_DRAW_PARTICLES
	#define GLQUAKE_SCALED_DRAWING
	#define GLQUAKE_ENTITY_INSPECTOR
	#define GLQUAKE_ENVMAP_COMMAND
	#define GLQUAKE_FLASH_BLENDS
	#define GLQUAKE_FOG
	#define GLQUAKE_RENDERER_SUPPORT
	#define GLQUAKE_RESIZABLE_WINDOW
	#define GLQUAKE_TEXTUREMODES
	#define GLQUAKE_TEXTURE_MANAGER
	#define GLQUAKE_TEXTURE_POINTER
	#define GLQUAKE_VIDBUFFER_ACCESS
	#define GLQUAKE_VIEW_BLENDS

	#define GLQUAKE_WAD_TEXTURE_UPLOAD	// Baker: I could make wad.c use new WinQuake Zero method, maybe in future.

	#define GLQUAKE_MIRRORS			// Baker: Not finished.  Someday I will finish? Sky is main issue.  Vis a little bit (cl + sv).

	#define GLQUAKE_HARDWARE_GAMMA
	#define GLQUAKE_SUPPORTS_VSYNC				// Baker: This is not restricted to GL, per se.
#endif

#ifndef GLQUAKE
	#define WINQUAKE_COLORMAP_TRANSLATION
	#define WINQUAKE_DRAW_PARTICLES
	#define WINQUAKE_PALETTE
	#define WINQUAKE_RENDERER_SUPPORT
	#define WINQUAKE_SKYBOX_SUPPORT
	#define WINQUAKE_VIDBUFFER_ACCESS
	#define WINQUAKE_VIEW_BLENDS

	#define WINQUAKE_SOFTWARE_SKYBOX
	#define WINQUAKE_STIPPLE_WATERALPHA
	#define WINQUAKE_QUICK_PALETTE

#ifdef PLATFORM_OSX
	#define WINQUAKE_VSYNC_SUPPORT
#endif // PLATFORM_OSX

#endif

// ProQuake server features
#define SUPPORTS_PQ_RQUAKE					// Baker: Keeps entities not meant for deathmatch like the doors in E4M3.  For RQuake coop which uses deathmatch scoreboard.
#define SUPPORTS_PQ_RCON_FAILURE_BLACKOUT
#define SUPPORTS_PQ_RCON_ATTEMPTS_LOGGED
#define SUPPORTS_PQ_WORD_FILTER
#define SUPPORTS_HTTP_DOWNLOAD


// These are light.  No define.
// pq_chat_color_change_mute				// Baker: ProQuake's feature to prevent a color change and then messagemode2 (team chat).
// pq_chat_time_to_talk						// Baker: R00ks invention that prevents newly connected people from spamming before ban file check.
// pq_chat_frags_to_talk					// Baker: My implementation.


//#define SUPPORTS_SHIFT_SLOW_ALWAYS_RUN	// Baker: (WORKS) Quakespasm modification.  Tried it.  I hate it.
//#define SUPPORTS_GHOSTING_AUTOMAP			// Baker: Older Mark V had this.  Neat, decentish.  Touched code everywhere though :(
//#define SUPPORTS_GHOSTING_VIA_DEMOS		// Baker: Older Mark V had this.  Ditto.  Race a ghost from a demo.
//#define SUPPORTS_64_BIT					// Baker: On Windows someday ... perhaps.  The Mac build is 64 bit, haven't had problems thus far.
//#define SUPPORTS_WINDOWS_MENUS			// Baker: Older Mark V had this.  Was interesting idea.


#ifdef __GNUC__
// Baker:  I have not been able to make the following work with CodeBlocks/MinGW32:
//  Note: MinGW's equivalents are incomplete especially with CoInitialize stuff missing in the libs.  Reckless has fixed one.
// 1) MP3 music
// 2) AVI capture
// 3) Direct3D 8
// It is important to note that the CodeBlocks/MinGW32 package comes with its own
// purified Windows headers that aren't quite the same as the real ones.  As a result
// there is no reason to expect these features to ever be able to work with
// CodeBlocks/MinGW32.
	#ifdef SUPPORTS_MP3_MUSIC
		#undef SUPPORTS_MP3_MUSIC
		#define WANTED_MP3_MUSIC // But ... can't have it.
	#endif
	#ifdef SUPPORTS_AVI_CAPTURE
		#undef SUPPORTS_AVI_CAPTURE // Must disable AVI capture :(
		#define WANTED_AVI_CAPTURE // But ... can't have it.
	#endif
#endif


#ifdef PLATFORM_OSX
	#define SUPPORTS_MP3_MUSIC

	#ifdef GLQUAKE
		#pragma message ("Hardware gamma not implemented")
		#undef SUPPORTS_CD // Lazy ... not supported yet.
		#undef GLQUAKE_HARDWARE_GAMMA
		#undef GLQUAKE_RESIZABLE_WINDOW
		#pragma message ("Baker: GLQUAKE_HARDWARE_GAMMA will need restored")
	#endif
#endif // PLATFORM_OSX

#ifdef SDLQUAKE
    #undef SUPPORTS_CD // Lazy ... not supported yet.
    #undef GLQUAKE_RESIZABLE_WINDOW // SDL 1.2 doesn't seem to have a method to make this very possible.
	#undef SUPPORTS_MP3_MUSIC
	#undef SUPPORTS_AVI_CAPTURE // Must disable AVI capture :(
#endif


#include "q_stdinc.h"

#define MAX_NUM_Q_ARGVS_50	50

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2


#define	MAX_QPATH		64			// max length of a quake game pathname

#define	ON_EPSILON		0.1			// point on plane side epsilon

#define	DIST_EPSILON		(0.03125)		// 1/32 epsilon to keep floating point happy (moved from world.c)

#ifdef SUPPORTS_SERVER_PROTOCOL_15
// Baker: To share with net_dgrm.c
extern int host_protocol_datagram_maxsize;
#endif // SUPPORTS_SERVER_PROTOCOL_15

typedef enum
{
//	Mark V limits ... Baker: I prefer not to do these ...

	MAX_MARK_V_MSGLEN				= 65536,
	MAX_MARK_V_SIGNON				= MAX_MARK_V_MSGLEN - 2,
	NET_MARK_V_MAXMESSAGE			= 65536,
	MAX_MARK_V_DATAGRAM	 			= 65527,
//	MAX_MARK_V_CLIENT_MAXMESSAGE	= MAX_MARK_V_MAXMESSAGE, // 65536
	// (NETFLAG_DATA - 1 - NET_HEADERSIZE)
	//  0x10000 - 1 - (2 * sizeof(unsigned int))
	// 65536 - 1 - 8 = 65527

	// Quakespasm increased for an upcoming map (ijed?)

	MAX_MARK_V_ENT_LEAFS			= 32,	  MAX_FITZQUAKE_WINQUAKE_ENT_LEAFS = 16,
	MAX_MARK_V_EFRAGS				= 4096,
	MAX_MARK_V_VISEDICTS			= 4096, // Rubicon Rumble

// End Mark V limits

//	FitzQuake limit					 // Standard limit (WinQuake)
	MAX_EDICTS_PROTOCOL_666			= 32000,

	MIN_SANE_EDICTS_512				=  512,
	MAX_SANE_EDICTS_8192			=  8192,

//	QUAKESPASM_MAX_MSGLEN			= 64000, // Sheesh, breaks protocol 666 compatibility.  Not using.
//	QUAKESPASM_MAXMESSAGE			= 64000, // http://www.celephais.net/board/view_thread.php?id=60452&start=721

	MAX_FITZQUAKE_MSGLEN			= 32000,  MAX_WINQUAKE_MSGLEN			=  8000,
	NET_FITZQUAKE_MAXMESSAGE		= 32000,  MAX_WINQUAKE_MAXMESSAGE		=  8192,
	MAX_FITZQUAKE_DATAGRAM_SIZE		= 32000,  MAX_WINQUAKE_DATAGRAM			=  1024,

	MAX_FITZQUAKE_SIGNON			= /* 31998 */
				MAX_FITZQUAKE_MSGLEN - 2,	  MAX_WINQUAKE_SIGNON			=  MAX_WINQUAKE_MSGLEN - 2, /* 7998 */

	MAX_FITZQUAKE_DATAGRAM_MTU		= 1400,
	// ^^ FIX ME!!!!!  It is intended to be 1400

// per-level limits
	MAX_FITZQUAKE_BEAMS				=    32,  MAX_WINQUAKE_BEAMS			=    24,
	MAX_FITZQUAKE_EFRAGS			=  2048,  MAX_WINQUAKE_EFRAGS			=   600,
	MAX_FITZQUAKE_DLIGHTS			=   128,  MAX_WINQUAKE_DLIGHTS			=    32,
	MAX_FITZQUAKE_STATIC_ENTITIES	=   512,  MAX_WINQUAKE_STATIC_ENTITIES	=   128,
	MAX_FITZQUAKE_TEMP_ENTITIES		=   256,  MAX_WINQUAKE_TEMP_ENTITIES	=    64,
	MAX_FITZQUAKE_VISEDICTS			=  1024,  MAX_WINQUAKE_VISEDICTS		=   256,

	MAX_FITZQUAKE_LIGHTMAPS			=   256,  MAX_WINQUAKE_LIGHTMAPS		=    64,

	MAX_FITZQUAKE_MAX_EDICTS		= 32000,  MAX_WINQUAKE_EDICTS			=   600,
	MAX_FITZQUAKE_MODELS			=  2048,  MAX_WINQUAKE_MODELS			=   256,
	MAX_FITZQUAKE_SOUNDS			=  2048,  MAX_WINQUAKE_SOUNDS			=   256,

	MAX_FITZQUAKE_SURFACE_EXTENTS	=  2000,  MAX_WINQUAKE_SURFACE_EXTENTS  =   256,

} engine_limits;
//johnfitz -- ents past 8192 can't play sounds in the standard protocol

#define	MAX_LIGHTSTYLES	64
#define	MAX_STYLESTRING	64
#define	SAVEGAME_COMMENT_LENGTH	39

//
// stats are integers communicated to the client by the server
//
#define	MAX_CL_STATS		32
#define	STAT_HEALTH			0
#define	STAT_FRAGS			1
#define	STAT_WEAPON			2
#define	STAT_AMMO			3
#define	STAT_ARMOR			4
#define	STAT_WEAPONFRAME	5
#define	STAT_SHELLS			6
#define	STAT_NAILS			7
#define	STAT_ROCKETS		8
#define	STAT_CELLS			9
#define	STAT_ACTIVEWEAPON	10
#define	STAT_TOTALSECRETS	11
#define	STAT_TOTALMONSTERS	12
#define	STAT_SECRETS		13		// bumped on client side by svc_foundsecret
#define	STAT_MONSTERS		14		// bumped by svc_killedmonster

// stock defines

#define	IT_SHOTGUN				1
#define	IT_SUPER_SHOTGUN		2
#define	IT_NAILGUN				4
#define	IT_SUPER_NAILGUN		8
#define	IT_GRENADE_LAUNCHER		16
#define	IT_ROCKET_LAUNCHER		32
#define	IT_LIGHTNING			64
#define IT_SUPER_LIGHTNING      128
#define IT_SHELLS               256
#define IT_NAILS                512
#define IT_ROCKETS              1024
#define IT_CELLS                2048
#define IT_AXE                  4096
#define IT_ARMOR1               8192
#define IT_ARMOR2               16384
#define IT_ARMOR3               32768
#define IT_SUPERHEALTH          65536
#define IT_KEY1                 131072
#define IT_KEY2                 262144
#define	IT_INVISIBILITY			524288
#define	IT_INVULNERABILITY		1048576
#define	IT_SUIT					2097152
#define	IT_QUAD					4194304
#define IT_SIGIL1               (1<<28)
#define IT_SIGIL2               (1<<29)
#define IT_SIGIL3               (1<<30)
#define IT_SIGIL4               (1<<31)

//===========================================
//rogue changed and added defines

#define RIT_SHELLS              128
#define RIT_NAILS               256
#define RIT_ROCKETS             512
#define RIT_CELLS               1024
#define RIT_AXE                 2048
#define RIT_LAVA_NAILGUN        4096
#define RIT_LAVA_SUPER_NAILGUN  8192
#define RIT_MULTI_GRENADE       16384
#define RIT_MULTI_ROCKET        32768
#define RIT_PLASMA_GUN          65536
#define RIT_ARMOR1              8388608
#define RIT_ARMOR2              16777216
#define RIT_ARMOR3              33554432
#define RIT_LAVA_NAILS          67108864
#define RIT_PLASMA_AMMO         134217728
#define RIT_MULTI_ROCKETS       268435456
#define RIT_SHIELD              536870912
#define RIT_ANTIGRAV            1073741824
#define RIT_SUPERHEALTH         2147483648

//MED 01/04/97 added hipnotic defines
//===========================================
//hipnotic added defines
#define HIT_PROXIMITY_GUN_BIT 16
#define HIT_MJOLNIR_BIT       7
#define HIT_LASER_CANNON_BIT  23
#define HIT_PROXIMITY_GUN   (1<<HIT_PROXIMITY_GUN_BIT)
#define HIT_MJOLNIR         (1<<HIT_MJOLNIR_BIT)
#define HIT_LASER_CANNON    (1<<HIT_LASER_CANNON_BIT)
#define HIT_WETSUIT         (1<<(23+2))
#define HIT_EMPATHY_SHIELDS (1<<(23+3))

//===========================================

#define	MAX_SCOREBOARD		16
#define	MAX_SCOREBOARDNAME	32

#define	SOUND_CHANNELS		8

typedef struct
{
	char		_basedir[MAX_OSPATH];
	const char	*basedir; // This can be overriden with -basedir, this is the current working directory (cwd command fills this)
	int			argc;
	char		**argv;
	void		*membase;
	int			memsize;
} host_parms_t;

int Main_Central (char *cmdline, void *main_window_holder_addr, cbool do_loop);

#include "arch_def.h"
#include "buffers.h"
#include "common.h"
#include "bspfile.h"
#include "sys.h"
#include "zone.h"
#include "q_mathlib.h"
#include "cvar.h"

#include "protocol.h"
#include "net_admin.h"
#include "net.h"

#include "cmd.h"
#include "crc.h"

#include "progs.h"
#include "server.h"

#include "text_undo.h"
#include "text_edit.h"
#include "console.h"
#include "wad.h"
#include "vid.h"
#include "screen.h"
#include "draw.h"
#include "render.h"
#include "view.h"
#include "sbar.h"
#include "q_sound.h"
#include "client.h"
#include "model.h"
#ifndef GLQUAKE
	#include "d_iface.h"
#endif

#include "world.h"

#include "neural.h" //stephenkoren

#ifdef GLQUAKE
#include "gl_texmgr.h" //johnfitz
#endif

#include "input.h"
#include "text_history.h"
#include "keys.h"
#include "menu.h"
#include "q_music.h"

#ifdef GLQUAKE
	#include "glquake.h"
	#include "gl_renderer.h"
#else
	#include "r_local.h"
	#include "d_local.h"
#endif
#ifdef SUPPORTS_NEHAHRA
#include "nehahra.h"
#endif // SUPPORTS_NEHAHRA

#include "q_image.h"
#include "movie.h" // Baker
#include "talk_macro.h" // Baker
#include "q_lists.h"
#include "location.h" // Baker
#include "tool_inspector.h" // Baker
#include "tool_texturepointer.h" // Baker
#include "recent_file.h" // Baker
#include "download.h" // Baker
#include "utilities.h" // Baker
//#include "sys_win_menu.h"



//=============================================================================

// the host system specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use

//
// host
//
extern host_parms_t host_parms;

extern	cbool	host_initialized;		// true if into command execution
extern	cbool	host_post_initialized;	// true if beyond initial command execution (config.cfg already run, etc.)
extern	double		host_frametime_;
extern	int			host_framecount;	// incremented every frame, never reset
extern	double		realtime;			// not bounded in any way, changed at
										// start of every frame, never reset

extern double sv_frametime;
extern double cl_frametime; // This is sort of used.  For input frames (mouse, etc.).  Sort of.
extern double s_frametime;
extern double host_timeslice;


void Host_ClearMemory (void);
void Host_Init (void);
void Host_Shutdown(void);
void Host_Error (const char *error, ...) __attribute__((__format__(__printf__,1,2), __noreturn__));
void Host_EndGame (const char *message, ...) __attribute__((__format__(__printf__,1,2), __noreturn__));
void Host_Frame (double time);
//cmd void Host_Quit_f (void); // Quit command, does a confirm
void Host_Quit (void); // Quit.  No confirm (like clicking "X" on window)
void Host_ClientCommands (const char *fmt, ...) __attribute__((__format__(__printf__,1,2)));
void Host_ShutdownServer (cbool crash);
void Host_WriteConfiguration (void);
const char *Host_Savegame (const char *in_savename, cbool user_initiated);
void Host_Stopdemo_f (lparse_t *unused);
void Host_Changelevel_Required_Msg (cvar_t *var);
void Host_Pause_f (lparse_t *unused);

int Host_Gamedir_Change (const char *gamedir_new, const char *new_hud_typestr, cbool liveswitch, const char** info_string);
cbool Read_Early_Cvars_For_File (const char *config_file_name, const cvar_t *list[]);

extern cbool isDedicated;

#define AUTO_SAVE_INTERVAL 90
#define AUTO_SAVE_COUNT 3
#define AUTO_SAVE_MINIMUM_TIME 120
#define AUTO_DEMO_NAME "automatic_demo"


enum plat_e
{
	platform_windows = 0,
	platform_osx = 1,
};

enum rend_e
{
	renderer_software = 0,	// Like WinQuake
	renderer_hardware = 1,	// OpenGL or Direct3D
};

enum host_e
{
	host_both = 0,			// Both
	host_server_only = 1,	// Can't client
	host_client_only = 2,	// Can't serve
};


typedef struct
{
	cbool		assembly_langauage;
	enum host_e	host_type;
	enum rend_e	renderer;
	cbool		direct3d;
//	cbool		input joystick, touchpad, mouse, whatever ...
	cbool		music_mp3;
	cbool		music_cd;
	cbool		video_avi_capture;
	cbool		video_vsync;

	enum plat_e	platform;
} build_t;

extern build_t build;

#define DEP_NONE					0
#define	DEP_AVI				(1U << 0)
#define DEP_GL				(1U << 1)
#define DEP_SW				(1U << 2)
#define DEP_GAMMA			(1U << 3) // Kill
#define DEP_MIRROR			(1U << 4)
#define DEP_VSYNC			(1U << 5)
#define DEP_FREQ			(1U << 6) //


#endif // ! __QUAKEDEF_H__

