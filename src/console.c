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
// console.c

#include "environment.h"
#include "quakedef.h"

#ifndef _MSC_VER
#include <unistd.h>
#endif

console_t console1;

int	con_debuglog;
char	con_debuglog_url[MAX_OSPATH]; // Baker: from ProQuake -condebug, specify log file name

/*
================
Con_Quakebar -- johnfitz -- returns a bar of the desired length, but never wider than the console

includes a newline, unless len >= console1.buffer_columns.
================
*/
const char *Con_Quakebar (int len)
{
	static char bar[42];
	int i;

	len = c_min(len, (int)sizeof(bar) - 2);
	len = c_min(len, console1.buffer_columns);

	bar[0] = '\35';
	for (i = 1; i < len - 1; i++)
		bar[i] = '\36';
	bar[len-1] = '\37';

	if (len < console1.buffer_columns)
	{
		bar[len] = '\n';
		bar[len+1] = 0;
	}
	else bar[len] = 0;

	return bar;
}

/*
================
Con_ToggleConsole_f
================
*/


// Exiting the console ... set things.
void Con_Exit (void)
{
	History_Save (); // Baker: Keep history up-to-date.
	history_lines[edit_line][1] = 0;	// clear any typing
	Key_Console_Cursor_Move (0, cursor_reset); // Reset selection
	//key_linepos = 1; key_selstart = 0; key_sellength = 0;
	console1.backscroll = 0; //johnfitz -- toggleconsole should return you to the bottom of the scrollback
	history_line = edit_line; //johnfitz -- it should also return you to the bottom of the command history

	Partial_Reset ();  Con_Undo_Clear ();
}

void Con_ToggleConsole_f (lparse_t *unused)
{
	if (key_dest == key_console/* || (key_dest == key_game && console1.forcedup)*/)
	{
		Con_Exit ();
		if (cls.state == ca_connected)
			Key_SetDest (key_game);
		else M_Menu_Main_f (NULL);
	}
	else Key_SetDest (key_console);

	SCR_EndLoadingPlaque ();
	memset (console1.con_times, 0, sizeof(console1.con_times));
}

/*
================
Con_Clear_f
================
*/
void Con_Clear_f (void)
{
	if (console1.text)
		memset (console1.text, ' ', console1.buffer_size); //johnfitz -- console1.buffer_size replaces CON_TEXTSIZE
	console1.backscroll = 0; //johnfitz -- if console is empty, being scrolled up is confusing
}

/*
================
Con_Dump_f -- johnfitz -- adapted from quake2 source
================
*/
void Con_Dump_f (void)
{
	int		l, x;
	const char	*line;
	FILE	*f;
	char	buffer[1024];
	char	name[MAX_OSPATH];

#if 1
	//johnfitz -- there is a security risk in writing files with an arbitrary filename. so,
	//until stuffcmd is crippled to alleviate this risk, just force the default filename.
	FS_FullPath_From_QPath (name, "condump.txt");
#else
	if (line->count > 2)
	{
		Con_Printf ("usage: condump <filename>\n");
		return;
	}

	if (line->count > 1)
	{
		if (strstr(line->args[1], ".."))
		{
			Con_Printf ("Relative pathnames are not allowed.\n");
			return;
		}
		FS_FullPath_From_QPath (name, line->args[1]);
		File_URL_Edit_Force_Extension (name, ".txt", sizeof(name));
	}
	else
		FS_FullPath_From_QPath (name, "condump.txt");
#endif

	f = FS_fopen_write_create_path (name, "w");
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open file %s.\n", name);
		return;
	}

	// skip initial empty lines
	for (l = console1.cursor_row - console1.buffer_rows + 1 ; l <= console1.cursor_row ; l++)
	{
		line = console1.text + (l%console1.buffer_rows)*console1.buffer_columns;
		for (x=0 ; x<console1.buffer_columns ; x++)
			if (line[x] != ' ')
				break;
		if (x != console1.buffer_columns)
			break;
	}

	// write the remaining lines
	buffer[console1.buffer_columns] = 0;
	for ( ; l <= console1.cursor_row ; l++)
	{
		line = console1.text + (l%console1.buffer_rows)*console1.buffer_columns;
		strncpy (buffer, line, console1.buffer_columns);
		for (x = console1.buffer_columns - 1 ; x >= 0 ; x--)
		{
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}
		for (x=0; buffer[x]; x++)
			buffer[x] &= 0x7f;

		fprintf (f, "%s\n", buffer);
	}

	FS_fclose (f);
	Recent_File_Set_QPath (name);
	Con_Printf ("Dumped console text to %s.\n", name);
}

/*
================
Con_Copy_f -- Baker -- adapted from Con_Dump
================
*/
void Con_Copy_f (lparse_t *line)
{
	char	outstring[CONSOLE_TEXTSIZE]="";
	int		l, x;
//	char	*line;
	char	buffer[1024];

	if (line && line->count > 1) // More than just the command
	{
		cvar_t *var;

		if (strcasecmp (line->args[1], "ents") == 0)
		{
			extern char *entity_string;
			if (!sv.active || !entity_string)
			{
				Con_Printf ("copy ents: Not running a server or no entities");
				return;
			}

			Clipboard_Set_Text (entity_string);

			Con_Printf ("Entities copied to the clipboard (%i bytes)\n", (int)strlen(entity_string));
			return;
		}

		if (strcasecmp (line->args[1], "screen") == 0)
		{
			SCR_ScreenShot_Clipboard_f ();
			return;
		}

		// Try cvar
		var = Cvar_Find (line->args[1]);

		if (var)
		{
			Clipboard_Set_Text (var->string);
			Con_Printf ("Cvar %s string copied to clipboard\n", line->args[1]);
			return;
		}

		// Invalid args
		Con_Printf ("Usage: %s [ents | screen | <cvar name>]: copies console to clipboard\n\"copy ents\" copies map entities to clipboard.\n\"copy screen\" copies screen to clipboard.", line->args[0]);
		return;
	}

	// skip initial empty lines
	for (l = console1.cursor_row - console1.buffer_rows + 1 ; l <= console1.cursor_row ; l++)
	{
		char	*myline  = console1.text + (l%console1.buffer_rows)*console1.buffer_columns;
		for (x = 0 ; x < console1.buffer_columns ; x++)
			if (myline[x] != ' ')
				break;
		if (x != console1.buffer_columns)
			break;
	}

	// write the remaining lines
	buffer[console1.buffer_columns] = 0;
	for ( ; l <= console1.cursor_row ; l++)
	{
		char *myline = console1.text + (l%console1.buffer_rows)*console1.buffer_columns;
		strncpy (buffer, myline, console1.buffer_columns);
		for (x = console1.buffer_columns - 1 ; x >= 0 ; x--)
		{
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}
		for (x=0; buffer[x]; x++)
			buffer[x] &= 0x7f;

		c_strlcat (outstring, va("%s\r\n", buffer));

	}

	Clipboard_Set_Text (outstring);
	Con_Printf ("Copied console to clipboard\n");
}

/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
//	for (i = 0 ; i < scr_con_notifylines.value ; i++)
//		console1.con_times[i] = 0;

	memset (console1.con_times, 0, sizeof(console1.con_times));
}


/*
================
Con_MessageMode_f
================
*/
void Con_MessageMode_f (void)
{
	if (cls.state != ca_connected || cls.demoplayback)
		return;
	chat_team = false;
	Key_SetDest (key_message);
}

/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f (void)
{
	if (cls.state != ca_connected || cls.demoplayback)
		return;
	chat_team = true;
	Key_SetDest (key_message);
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	*tbuf; //johnfitz -- tbuf no longer a static array
	int mark; //johnfitz

	width = (vid.conwidth >> 3) - 2; //johnfitz -- use vid.conwidth instead of vid.width

	if (width == console1.buffer_columns)
		return;

	oldwidth = console1.buffer_columns;
	console1.buffer_columns = width;
	oldtotallines = console1.buffer_rows;
	console1.buffer_rows = console1.buffer_size / console1.buffer_columns; //johnfitz -- console1.buffer_size replaces CON_TEXTSIZE
	numlines = oldtotallines;

	if (console1.buffer_rows < numlines)
		numlines = console1.buffer_rows;

	numchars = oldwidth;

	if (console1.buffer_columns < numchars)
		numchars = console1.buffer_columns;

	mark = Hunk_LowMark (); //johnfitz
	tbuf = (char *) Hunk_Alloc (console1.buffer_size); //johnfitz

	memcpy (tbuf, console1.text, console1.buffer_size);//johnfitz -- console1.buffer_size replaces CON_TEXTSIZE
	memset (console1.text, ' ', console1.buffer_size);//johnfitz -- console1.buffer_size replaces CON_TEXTSIZE

	for (i=0 ; i<numlines ; i++)
	{
		for (j=0 ; j<numchars ; j++)
		{
			console1.text[(console1.buffer_rows - 1 - i) * console1.buffer_columns + j] =
					tbuf[((console1.cursor_row - i + oldtotallines) % oldtotallines) * oldwidth + j];
		}
	}

	Hunk_FreeToLowMark (mark); //johnfitz

	Con_ClearNotify ();

	console1.backscroll = 0;
	console1.cursor_row = console1.buffer_rows - 1;
}


/*
================
Con_Init
================
*/
void Con_Init (void)
{
	int i;

	con_debuglog = COM_CheckParm("-condebug");

	if (con_debuglog)
	{
		const char *logfile_relative = "qconsole"; // qconsole.log

		// If there is at least parameter after con_debuglog and it doesn't begin with a - or + then use it as the log filename
		if ( con_debuglog + 1 < com_argc && com_argv[con_debuglog + 1][0] != '-' && com_argv[con_debuglog + 1][0] != '+')
			logfile_relative = com_argv[con_debuglog + 1];

		FS_FullPath_From_QPath (con_debuglog_url, logfile_relative);

		if (isDedicated) // If dedicated, append ip and port #
		{
			const char *ip_address = "local";
			extern int DEFAULTnet_hostport; // 26000
			int portnum = DEFAULTnet_hostport;

			if ( (i = COM_CheckParm ("-ip")) && i + 1 < com_argc)
				ip_address = va("ip_%s", com_argv[i + 1]);

			c_strlcat (con_debuglog_url, va("_%s", ip_address));

			if ( (i = COM_CheckParm ("-port")) && i + 1 < com_argc && atoi(com_argv[i + 1])  )
				portnum = atoi(com_argv[i + 1]);

			c_strlcat (con_debuglog_url, va("_port_%i", portnum));
		}

		File_URL_Edit_Force_Extension (con_debuglog_url, ".log", sizeof(con_debuglog_url));
//		System_Alert (con_debuglog_url);

		File_Delete (con_debuglog_url);
	}

	if (isDedicated)
		return;

	//johnfitz -- user settable console buffer size
	i = COM_CheckParm("-consize");
	if (i && i < com_argc-1)
		console1.buffer_size = c_max(CONSOLE_MINSIZE, atoi(com_argv[i+1]) * 1024);
	else
		console1.buffer_size = CONSOLE_TEXTSIZE;
	//johnfitz

	console1.text = (char *) Hunk_AllocName (console1.buffer_size, "context");//johnfitz -- console1.buffer_size replaces CON_TEXTSIZE
	memset (console1.text, ' ', console1.buffer_size);//johnfitz -- console1.buffer_size replaces CON_TEXTSIZE

	//johnfitz -- no need to run Con_CheckResize here
	console1.buffer_columns = 78; // Baker: default the 640 width @ char_width 8 size = 640 / 8 = 80.  Subtract 2 for margin = 78
	console1.buffer_rows = console1.buffer_size / console1.buffer_columns;//johnfitz -- console1.buffer_size replaces CON_TEXTSIZE
	console1.backscroll = 0;
	console1.cursor_row = console1.buffer_rows - 1;
	console1.user_pct = 0.50; // Default console is 50%
	//johnfitz

	Con_SafePrintf ("Console initialized.\n");

	Cmd_AddCommands (Con_Init);

	console1.initialized = true;
}


/*
===============
Con_Linefeed
===============
*/
static void Con_Linefeed (void)
{
	//johnfitz -- improved scrolling
	if (console1.backscroll)
		console1.backscroll++;
	if (console1.backscroll > console1.buffer_rows - (clheight >> 3) - 1)
		console1.backscroll = console1.buffer_rows - (clheight >> 3) - 1;
	//johnfitz

	console1.cursor_column = 0;
	console1.cursor_row++;
	memset (&console1.text[(console1.cursor_row%console1.buffer_rows)*console1.buffer_columns], ' ', console1.buffer_columns);
}



/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the notify window will pop up.
================
*/
static void Con_Print (const char *txt)
{
	int		y;
	int		c, l;
	static int	cr;
	int		mask;


	//console1.backscroll = 0; //johnfitz -- better console scrolling

	if (txt[0] == 1)
	{
		mask = 128;		// go to colored text
		S_LocalSound ("misc/talk.wav"); // play talk wav
		txt++;
	}
	else if (txt[0] == 2)
	{
		mask = 128;		// go to colored text
		txt++;
	}
	else
	{
		mask = 0;
	}

	while ( (c = *txt) )
	{
	// count word length
		for (l = 0 ; l < console1.buffer_columns ; l++)
			if ( txt[l] <= ' ')
				break;

	// word wrap
		if (l != console1.buffer_columns && (console1.cursor_column + l > console1.buffer_columns) )
			console1.cursor_column = 0;

		txt++;

		if (cr)
		{
			console1.cursor_row--;
			cr = false;
		}

		if (!console1.cursor_column)
		{
			Con_Linefeed ();
		// mark time for transparent overlay
			if (console1.cursor_row >= 0)
				console1.con_times[console1.cursor_row % (int)CONSOLE_NOTIFY_TIMES] = realtime;
		}

		switch (c)
		{
		case '\n':
			console1.cursor_column = 0;
			break;

		case '\r':
			if (pq_remove_cr.value)	// JPG 3.20 - optionally remove '\r'
				c += 128;
			else
			{
				console1.cursor_column = 0;
				cr = 1;
			}
			break;

		default:	// display character and advance
			y = console1.cursor_row % console1.buffer_rows;
			console1.text[y*console1.buffer_columns+console1.cursor_column] = c | mask;
			console1.cursor_column++;
			if (console1.cursor_column >= console1.buffer_columns)
				console1.cursor_column = 0;
			break;
		}
	}
}


/*
================
Con_DebugLog
================
*/

static void Con_DebugLog (const char *fmt, ...)
{
    FILE *f;
	VA_EXPAND (msg, 1024, fmt)

	f = FS_fopen_write (con_debuglog_url, "a");
	fprintf (f, "%s", msg);
    FS_fclose(f);
}


/*
================
Con_Printf

Handles cursor positioning, line wrapping, etc
================
*/
#define	MAXPRINTMSG	4096



int Con_Printf (const char *fmt, ...)
{
	static cbool	inupdate;
	VA_EXPAND (msg, MAXPRINTMSG, fmt)

// also echo to debugging console
	Dedicated_Printf ("%s", msg); // also echo to debugging console

// log all messages to file
	if (con_debuglog)
		Con_DebugLog ("%s", msg);

	if (!console1.initialized)
		return 0;

	if (isDedicated)
		return 0;		// no graphics mode

// write it to the scrollable buffer
	Con_Print (msg);

// update the screen if the console is displayed
	if (cls.signon != SIGNONS && !scr_disabled_for_loading )
	{
	// protect against infinite loop if something in SCR_UpdateScreen calls
	// Con_Printf
		if (!inupdate)
		{
			inupdate = true;
			SCR_UpdateScreen ();
			inupdate = false;
		}
	}

    return 0;
}


/*
================
Con_Warning -- johnfitz -- prints a warning to the console
================
*/
int Con_Warning (const char *fmt, ...)
{
	VA_EXPAND (msg, MAXPRINTMSG, fmt)

	Con_SafePrintf ("%sWarning: %s", isDedicated ? "": "\x02", msg);
//	Con_Printf ("%s", msg);  Why was this here?
    return 0;
}

/*
================
Con_DPrintf

A Con_Printf that only shows up if the "developer" cvar is set
================
*/
int Con_DPrintf (const char *fmt, ...)
{
	// don't confuse non-developers with techie stuff...
	if (developer.value)
	{
		VA_EXPAND (msg, MAXPRINTMSG, fmt)

		Con_SafePrintf ("%s", msg); //johnfitz -- was Con_Printf
	}
	return 0;
}

/*
================
Con_DPrintf2 -- johnfitz -- only prints if "developer" >= 2

currently not used
================
*/
int Con_DPrintf2 (const char *fmt, ...)
{
	if (developer.value >= 2)
	{
		VA_EXPAND (msg, MAXPRINTMSG, fmt)

		Con_Printf ("%s", msg);
	}
	return 0;
}


/*
==================
Con_SafePrintf

Okay to call even when the screen can't be updated
==================
*/
int Con_SafePrintf (const char *fmt, ...)
{

	if (console1.initialized)
	{
        int			temp;
        VA_EXPAND (msg, MAXPRINTMSG, fmt)
        temp = scr_disabled_for_loading;
        scr_disabled_for_loading = true; // temped
        Con_Printf ("%s", msg);
        scr_disabled_for_loading = temp;
	}
	return 0;
}

/*
================
Con_CenterPrintf -- johnfitz -- pad each line with spaces to make it appear centered
================
*/
void Con_CenterPrintf (int linewidth, const char *fmt, ...) __attribute__((__format__(__printf__,2,3)));
void Con_CenterPrintf (int linewidth, const char *fmt, ...)
{
//	char	msg[MAXPRINTMSG]; //the original message
	char	line[MAXPRINTMSG]; //one line from the message
	char	spaces[21]; //buffer for spaces
	char	*src, *dst;
	int		len, s;
	VA_EXPAND (msg, MAXPRINTMSG, fmt)

	linewidth = c_min (linewidth, console1.buffer_columns);
	for (src = msg; *src; )
	{
		dst = line;
		while (*src && *src != '\n')
			*dst++ = *src++;
		*dst = 0;
		if (*src == '\n')
			src++;

		len = strlen(line);
		if (len < linewidth)
		{
			s = (linewidth-len)/2;
			memset (spaces, ' ', s);
			spaces[s] = 0;
			Con_Printf ("%s%s\n", spaces, line);
		}
		else
			Con_Printf ("%s\n", line);
	}
}

/*
==================
Con_LogCenterPrint -- johnfitz -- echo centerprint message to the console
==================
*/
void Con_LogCenterPrint (const char *str)
{

	if (!strcmp(str, cl.lastcenterstring))
		return; //ignore duplicates

	if (cl.gametype == GAME_DEATHMATCH && scr_logcenterprint.value != 2)
		return; //don't log in deathmatch

	c_strlcpy (cl.lastcenterstring, str);

	if (scr_logcenterprint.value)
	{
		Con_Printf ("%s", Con_Quakebar(40));
		Con_CenterPrintf (40, "%s\n", str);
		Con_Printf ("%s", Con_Quakebar(40));
		Con_ClearNotify ();
	}
}

/*
==============================================================================

	TAB COMPLETION

==============================================================================
*/

//johnfitz -- tab completion stuff
//unique defs

cbool key_inpartial;
int key_completetype;
char *key_partial_start;
char *key_partial_end;


typedef struct tab_s
{
	const char	*name;
	const char	*type;
	struct tab_s	*next;
	struct tab_s	*prev;
} tab_t;


//defs from elsewhere

/*
============
AddToTabList -- johnfitz

tablist is a doubly-linked loop, alphabetized by name
============
*/
tab_t	*mgtablist;
static void AddToTabList_Hunk_Alloc (const char *name, const char *type)
{
	tab_t	*t,*insert;

	t = (tab_t *) Hunk_Alloc(sizeof(tab_t));
	t->name = name;
	t->type = type;

	if (!mgtablist) //create list
	{
		mgtablist = t;
		t->next = t;
		t->prev = t;
	}
	else if (strcmp(name, mgtablist->name) < 0) //insert at front
	{
		t->next = mgtablist;
		t->prev = mgtablist->prev;
		t->next->prev = t;
		t->prev->next = t;
		mgtablist = t;
	}
	else //insert later
	{
		insert = mgtablist;
		do
		{
			if (strcmp(name, insert->name) < 0)
				break;
			insert = insert->next;
		} while (insert != mgtablist);

		t->next = insert;
		t->prev = insert->prev;
		t->next->prev = t;
		t->prev->next = t;
	}
}

static void BuildMediaList_Hunk_Alloc (clist_t* headnode, const char *tagstr, const char *partial)
{
	clist_t	*item;
	int				len = strlen(partial);

	mgtablist = NULL;

	for (item = headnode ; item ; item=item->next)
		if (!strncmp (partial,item->name, len))
			AddToTabList_Hunk_Alloc (item->name, "");
}

/*
============
BuildTabList -- johnfitz
============
*/
static void BuildTabList_Hunk_Alloc (const char *partial)
{
	extern	cmd_alias_t		*cmd_alias;
	extern	cmd_function_t	*cmd_functions;

	cmd_alias_t		*alias;
	cvar_t			*cvar;
	cmd_function_t	*cmd;
	int				len;

	mgtablist = NULL;
	len = strlen(partial);

	cvar = Cvar_FindAfter ("", CVAR_NONE);
	for ( ; cvar ; cvar=cvar->next)
	{
		if (cvar->flags & CVAR_COURTESY) // Baker: These don't show
			continue;

		if (!strncmp (partial, cvar->name, len))
			AddToTabList_Hunk_Alloc (cvar->name, "cvar");
	}

	for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
		if (!strncmp (partial,cmd->name, len))
			AddToTabList_Hunk_Alloc (cmd->name, "command");

	for (alias = cmd_alias ; alias ; alias = alias->next)
		if (!strncmp (partial, alias->name, len))
			AddToTabList_Hunk_Alloc (alias->name, "alias");
}



/*
============
Con_TabComplete -- Baker
============
*/




// Pass this function:  GetPartial (&workline[1], key_linepos - 1, false, &match_start, &match_end,
// Finds semi-colon, honors quotes
// Check unterminated quote, trailing spaces.
const char *GetPartial (const char *text, int cursor_pos, cbool should_complete_blank, const char **cmdname_out, int *startpos_out, int *endpos_out, int *argnum_out)
{
	static char	cmdname_buffer[CONSOLE_MAX_CMDLINE_256];
	static char	partial_buffer[CONSOLE_MAX_CMDLINE_256];
	int partial_offset = 0, last_arg_num = -1;
	const char *start_text = &text[0];
	const char *end_text = &text[cursor_pos-1];

	// Reset the buffer
	cmdname_buffer[0] = partial_buffer[0] = 0;

	if (end_text >= start_text)
	{
		char cmd_buffer_spc[CONSOLE_MAX_CMDLINE_256]; // Remember, could be spaces before command.
		const char *find_semicolon   = String_Range_Find_Char_Reverse(start_text, end_text, ';');
		const char *find_doublequote = String_Range_Find_Char_Reverse(start_text, end_text, '\"');
		const char *higher_of_two    = find_doublequote > find_semicolon ? find_doublequote : find_semicolon;
		const char *start_statement  = higher_of_two  ? higher_of_two + 1 : start_text;
		size_t statement_to_cursor_len = end_text - start_statement + 1;
		const char *line_partial     = String_Length_Copy (cmd_buffer_spc, sizeof(cmd_buffer_spc), start_statement, statement_to_cursor_len);
		lparse_t * line	       = Line_Parse_Alloc (line_partial, true);
		cbool is_lastarg_blank = (line->args[line->count - 1][0] == 0);

		last_arg_num = (is_lastarg_blank == false || should_complete_blank) ? line->count - 1 : line->count - 2;

		c_strlcpy (cmdname_buffer, line->args[0]);

		if (last_arg_num >= 0)  // Convert to offset into original string, then copy into return buffer
		{
			partial_offset = line->args[last_arg_num] - line->chopped + (start_statement - start_text);
			c_strlcpy (partial_buffer, line->args[last_arg_num]);
		}

		line = Line_Parse_Free (line);
	}

	if (cmdname_out)	*cmdname_out  = cmdname_buffer;
	if (startpos_out)	*startpos_out = partial_offset;
	if (endpos_out)		*endpos_out   = partial_offset + strlen(partial_buffer) - 1;
	if (argnum_out)		*argnum_out	  = last_arg_num;

	return (const char *)partial_buffer;
}



ssize_t Con_AutoComplete (char *text, size_t s_size, ssize_t cursor, cbool force_completion, cbool *in_completion,
					   int *complete_type, const char **match_start, const char **match_end, cbool do_reverse)
{
//	const char *autocomplete_text = NULL;
	const char *partial;
	char partial_buffer[CONSOLE_MAX_CMDLINE_256];
	cbool	first_time_through = false;

	if ( (text[cursor] > 32) || (text[0] == 0 && !force_completion)  ) // If cursor on letter or zero length without force, get out.
		return 0;

	if ( cursor >= (ssize_t)s_size - 1) // Last char reserved for null
		return 0; // No room!

	if (*in_completion)
		String_Range_Copy (partial_buffer, sizeof(partial_buffer), *match_start, *match_end);
	else
	{
		// Examine everything ...
		const char *cmdname;
		int partial_start, partial_end, argnum, i;

		partial = GetPartial (text, cursor, force_completion, &cmdname, &partial_start, &partial_end, &argnum);

		if (partial[0] == 0 && !force_completion)
			return 0;

		c_strlcpy (partial_buffer, partial);

		*in_completion = true;
		*match_start = &text[partial_start];
		*match_end = &text[partial_end];

		// Determine completion type
		     if (argnum == 2 && !strcmp(cmdname, "game"))		*complete_type = list_type_game_hud;
		else if (argnum == 2 && !strcmp(cmdname, "setmusic"))	*complete_type = list_type_mp3;
		else if (argnum != 1)									*complete_type = list_type_none;
		else
		{
			for (i = 1, *complete_type = 0; i < MAX_LIST_TYPES && !*complete_type; i++)
				if (list_info[i].associated_commands && COM_ListMatch (list_info[i].associated_commands, cmdname))
					*complete_type = i;
		}

		Con_DPrintf ("Complete type is %s with extension %s\n", list_info[*complete_type].description, list_info[*complete_type].extension);
		first_time_through = true;
	}


//	System_Alert (partial_buffer); // What is in the partial buffer?  We may have a complete type, but we need the partial to know what to complete.


	// Right now we are inefficiently reconstructing the tab list each time.  First time through prints stuffs.
	{
		int mark = Hunk_LowMark();

		// Anything that is a known type has an associated list so use that.
		if (*complete_type != list_type_none)
			BuildMediaList_Hunk_Alloc (list_info[*complete_type].plist, list_info[*complete_type].description, partial_buffer);
		else BuildTabList_Hunk_Alloc (partial_buffer); // This fills us with commands/aliases/cvars;

		if (!mgtablist) // Print this every attempt, only takes 1 line.
		{
			Con_Printf ("No %s starting with %s%s\n", list_info[*complete_type].description, partial_buffer, (*complete_type == list_type_map) ? " in current gamedir" : "");
			return 0;
		}

		{
			char completion_buffer[CONSOLE_MAX_CMDLINE_256];
			const char		*match = first_time_through ? mgtablist->name : NULL;
			tab_t			*t;
			//char			*addspace = (text[cursor] == 0) && first_time_through ? " " : "";
			size_t			curlen;
			ssize_t			sizechange;

			if (!first_time_through)
			{
				// We've been in autocomplete before, find the length of last autocomplete so we can find.
				const char *cur;
				int n;
				for (cur = *match_start, n = 0; *cur && *cur != 32; cur ++, n ++)
					completion_buffer[n] = *cur;
				completion_buffer[n] = 0;
				curlen = n;
			}

			// If first time through, print the list.  If not, find out match.


			for (t = mgtablist;   ; t = t->next)
			{
				if (first_time_through)
				{
					if (t->type[0]) // Type is a pointer to "command", "alias", "cvar"
						Con_SafePrintf("   %s (%s)\n", t->name, t->type);
					else Con_SafePrintf("   %s\n", t->name);
				}
				else if (!strcmp(t->name, completion_buffer))
				{
					match = do_reverse ? t->prev->name : t->next->name;
					break;
				}
				if ( t->next == mgtablist) // Can't be easily put in the for loop, we'd need a counter because a one-length wrapped list would trip.
					break;
			}

			// We have the new completion
			Hunk_FreeToLowMark(mark); // it's okay to free it here because match doesn't point to the hunk
			// match;

			// Either delete the partial (first time) or delete the old match
			//if (first_time_through)
			curlen = &text[cursor] - *match_start;

			String_Edit_Range_Delete (text, s_size, *match_start, &(*match_start)[curlen - 1] );

			// Insert the new - if cursor is at end of line, try to add a space in at the end for convenience (may not fit)
			sizechange = String_Edit_Insert_At (text, s_size, va("%s ", match), *match_start - text);

			return sizechange - curlen;
		}
	}
}

/*
==============================================================================

DRAWING

==============================================================================
*/

#define char_width 8
#define char_height 8

void Con_Draw_TextBox (const char *text, int cursor, int cursor_length, const char *bronze_start, const char *bronze_end,
					   int cursor_width, int x, int y, int chars_wide_disp)
{

	int w = chars_wide_disp * char_width;
	int left_offset_into_text = cursor >= chars_wide_disp ? cursor - chars_wide_disp + 1 : 0; // 10-19 = 19-10 + 1

	int i;
	const char *cur;

	// Draw background first if applicable
	if (cursor_length)
	{
		int highlight_start  = cursor - (cursor_length > 0 ? cursor_length : 0);
//		int highlight_end    = highlight_start + abs(cursor_length) - 1;

		int pixel_x0_		 = (highlight_start - left_offset_into_text) * char_width + x;
		int pixel_x1_		 = pixel_x0_ + abs(cursor_length) * char_width;

		int pixel_x0		 = pixel_x0_ >= x ?      pixel_x0_ : x;  // Clamp low to x
		int pixel_x1		 = pixel_x1_ <= x + w ?  pixel_x1_ : x + w;
		int pixel_width		 = pixel_x1 - pixel_x0;

		// Add +1 to width and height, looks nicer.  254 is white, 1 is alpha.
		Draw_Fill (pixel_x0, y, pixel_width + 1, char_height + 1, 254, 1);
	}

	// Draw the line starting at the left offset

	for (cur = &text[left_offset_into_text], i = 0; i < chars_wide_disp && cur[0]; i++, cur ++) //only write enough letters to go from *text to cursor
	{
		// We toggle the 128 bit to create the bronzing effect during draw.
		int bronze_addition = (bronze_start && bronze_start <= cur && cur <= bronze_end) ? 128 : 0;
		Draw_Character ( x + i * char_width, y, *cur | bronze_addition);
	}

// johnfitz -- new cursor handling.  Baker: modified.  Does make any sense to draw if there is selection??
	if (cursor_width)
	{
		Draw_Fill ( (cursor - left_offset_into_text) * char_width + x, y, cursor_width, char_height, 254, 1);
	}
}

/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/

void Con_DrawNotify (void)
{
	int	i, x, v;
	const char	*text;
	float	time;

	Draw_SetCanvas (CANVAS_CONSOLE); //johnfitz
	v = vid.conheight; //johnfitz

	for (i = console1.cursor_row - scr_con_notifylines.value + 1 ; i <= console1.cursor_row ; i++)
	{
		if (i < 0)
			continue;
		time = console1.con_times[i % CONSOLE_NOTIFY_TIMES];
		if (time == 0)
			continue;
		time = realtime - time;
		if (time > scr_con_notifytime.value)
			continue;
		text = console1.text + (i % console1.buffer_rows)*console1.buffer_columns;

		for (x = 0 ; x < console1.buffer_columns ; x++)
			Draw_Character ( (x + 1) << 3, v, text[x]);

		v += 8;
	}

	if (key_dest == key_message)
	{
		const char *say_prompt = chat_team ? "(say_team):" : "say:";
		int text_start = strlen(say_prompt) + 1;

		Draw_String (8, v, say_prompt); //johnfitz
		{
			extern char chat_buffer[];
			int x				= text_start * 8;
			int chars_wide		= console1.buffer_columns - text_start;
			int draw_cursor		= !((int)((realtime-key_blinktime) * CONSOLE_CURSOR_SPEED) & 1);
			int cursor_width    = draw_cursor ? char_width / 4 : 0 ;

			Con_Draw_TextBox (chat_buffer, strlen(chat_buffer), 0, NULL, NULL,
								cursor_width, x, v, chars_wide);  // cursor_width, x, y, chars wide

		}

		v += 8;
	}

	if (v != vid.conheight)
	{
#ifdef GLQUAKE // Baker: Eliminate in future
		glquake_scr_tileclear_updates = 0; //johnfitz
#else
		winquake_scr_copytop = 1;
#endif // GLQUAKE vs. WINQUAKE
		clearnotify = 0;
	}
}

/*
================
Con_DrawInput -- johnfitz -- modified to allow insert editing

The input line scrolls horizontally if typing goes beyond the right edge
================
*/


void Con_DrawInput (void)
{
	int draw_cursor		  = (console1.visible_lines > 16 && !((int)((realtime-key_blinktime) * CONSOLE_CURSOR_SPEED) & 1));
	int cursor_width      = draw_cursor ? (key_insert ?  char_width / 4 : char_width) : 0 ;

	Con_Draw_TextBox (&history_lines[edit_line][0], key_linepos, key_sellength, key_partial_start, key_partial_end,
						cursor_width, 8, vid.conheight - 16, console1.buffer_columns);  // cursor_width, x, y, chars wide
}

/*
================
Con_DrawConsole -- johnfitz -- heavy revision

Draws the console with the solid background
The typing input line at the bottom should only be drawn if typing is allowed
================
*/
void Con_DrawConsole (float pct, cbool drawinput)
{
	int				i, j, x, y, rows;
	int				sb;
	int				ytop = vid.conheight - 8;
	const char	*text;
	char			ver[32];

	if (pct <= 0)
		return; // Baker: This should never happen.  SCR_DrawConsole checks this.

// draw the background
	Draw_SetCanvas (CANVAS_CONSOLE);
	Draw_ConsoleBackground ();


	// Baker: Don't draw the console text if menu is being drawn.
	if (key_dest == key_menu && m_state)
		return;

// draw the text
	rows = (console1.visible_lines_conscale + 7)/8;
	y = vid.conheight - rows * 8;
	rows -= 2; //for input and version lines
	sb = (console1.backscroll) ? 2 : 0;

	for (i = console1.cursor_row - rows + 1 ; i <= console1.cursor_row - sb ; i++, y+=8)
	{
		j = i - console1.backscroll;
		if (j<0)
			j = 0;
		text = console1.text + (j % console1.buffer_rows) * console1.buffer_columns;

		for (x=0 ; x < console1.buffer_columns ; x++)
			Draw_Character ( (x+1)<<3, y, text[x]);
	}

// draw scrollback arrows
	if (console1.backscroll)
	{
		y+=8; // blank line
		for (x=0 ; x < console1.buffer_columns ; x+=4)
			Draw_Character ((x+1)<<3, y, '^');
		y+=8;
	}

// draw the input prompt, user text, and cursor
	if (drawinput)
		Con_DrawInput ();

//draw version number in bottom right
	y+=8;
	c_snprintf2 (ver, "%s %1.2f"/*" beta2"*/, ENGINE_NAME, (float)ENGINE_VERSION);
	for (x=0; x < (int) strlen(ver); x++)
		Draw_Character ((console1.buffer_columns-strlen(ver)+x+2)<<3, ytop, ver[x] /*+ 128*/);
}


/*
==================
Con_NotifyBox
==================
*/
void Con_NotifyBox (const char *text)
{
	double		t1, t2;

// during startup for sound / cd warnings
	Con_Printf ("\n\n%s", Con_Quakebar(40)); //johnfitz
	Con_Printf ("%s", text);
	Con_Printf ("Press a key.\n");
	Con_Printf ("%s", Con_Quakebar(40)); //johnfitz

	key_count = -2; // wait for a key down and up
	Key_SetDest (key_console);

	do
	{
		t1 = System_DoubleTime ();
		SCR_UpdateScreen ();
		System_SendKeyEvents ();
		t2 = System_DoubleTime ();
		realtime += t2 - t1; // make the cursor blink
	} while (key_count < 0);

	Con_Printf ("\n");
	Key_SetDest (key_game);
	realtime = 0; // put the cursor back to invisible
}

