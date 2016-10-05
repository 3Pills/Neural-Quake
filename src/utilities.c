/*
Copyright (C) 2013-2014 Baker

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
// utilities.c -- Baker: utilities with a layer of separation


#include "utilities.h" // This will include quakedef.h for us if applicable

#ifdef QUAKE_GAME
#include "quakedef.h"
#include "utilities.h"

#endif // QUAKE_GAME

void Utilities_Init (void)
{
//	Pak_Init (qfunction_set);

#if 0
	Wad2_Init (qfunction_set);
	Downloads_Init (qfunction_set);
	Zip_Init (qfunction_set);
#endif

	Cmd_AddCommands (Utilities_Init);
}

const char *cmds[] = {"add", "compress", "extract", "help", "list", "rem", "replace", "unzip", "zip", NULL};
enum arg_e {arg_add, arg_compress, arg_extract, arg_help, arg_list, arg_rem, arg_replace, arg_unzip, arg_zip, max_args};

const char *arg_msg[] =
{
	"<file.pak> <file in pak> <source url>", // add
	"<file.pak>", // compress
	"<file.pak> <file in pak> <extract destination>", // extract
	"", // help
	"<file.pak>", // list
	"<file.pak> <file in pak>", // remove
	"<file.pak> <file in pak> <replace with file path>", // replace
	"<file.pak> <destination dir>", //unzip
	"<file.pak to create> <folder to pak>", // zip
	NULL,
};


int args_required[] =
{
	3, // add pakurl pak_file_name filenameurl
	1, // compress pakurl
	3, // extract pakurl filename
	0, // help
	1, // list pakurl
	2, // remove pakurl filename
	3, // replace pakurl filename filenameurl
	2, // unzip pakurl workingdir
	2, // zip pakurl compressdir
	0,
};


// compress, extract, rem, replace will remain unlisted as to not confuse users.
void Pak_Command_f (lparse_t *line)
{
	// replace file, delete file, compress


	if (line->count >= 2)
	{
		const char *parm2 = line->args[1];
		const char *_pakurl = line->count >= 3 ? line->args[2] : NULL;
		const char *arg1 = line->count >= 4 ? line->args[3] : NULL;
		const char *arg2 = line->count >=5 ? line->args[4] : NULL;

		char pakurl[MAX_OSPATH];
		char fileurl[MAX_OSPATH];
		char destfolder[MAX_OSPATH];
		char srcfolder[MAX_OSPATH];
		char inside_pak_filename[MAX_OSPATH];

		int cmd_num = String_To_Array_Index (parm2, cmds);
		int numargs = line->count - 2;
		int i;
		cbool validnum = cmd_num < max_args;
		cbool meets_args = validnum ? args_required[cmd_num] == numargs : false;

		if (!validnum)
		{
			Con_Printf ("Invalid pak command %s\n", parm2);
			return;
		}

		if (!meets_args)
		{
			Con_Printf ("Wrong number of arguments for \"%s\": needs %i, have %i\n", parm2, args_required[cmd_num], numargs);
			Con_Printf ("Usage: %s %s %s\n", line->args[0], parm2, arg_msg[cmd_num]);
			return;
		}

		c_strlcpy (pakurl, _pakurl);
		File_URL_Edit_SlashesForward_Like_Unix (pakurl);

		if (cmd_num != arg_zip && !File_Exists (pakurl))
		{
			Con_Printf ("File \"%s\" doesn't exist\n", _pakurl); // Print the unmodified version
			return;
		}

		switch (cmd_num)
		{
		case arg_add:  // sorts and rewrites (where nulled entry space is recovered) (can just sort the header)
	
			c_strlcpy (inside_pak_filename, arg1); File_URL_Edit_SlashesForward_Like_Unix (inside_pak_filename);
			c_strlcpy (fileurl, arg2); File_URL_Edit_SlashesForward_Like_Unix (fileurl);

			// Make sure the new file exists
			if (!File_Exists (fileurl))
			{
				Con_Printf ("New file \"%s\" does not exist\n", arg2);
				return;
			}

			if (Pak_Has_File(pakurl, inside_pak_filename))
			{
				Con_Printf ("A file named \"%s\" is already in the pak\n", inside_pak_filename);
				return;
			}

			// We already know the pak exists so lets get to it
			Pak_Add_File (pakurl, inside_pak_filename, fileurl /*source*/);

			Recent_File_Set_FullPath (pakurl);
			return;


		case arg_compress:  // sorts and rewrites (where nulled entry space is recovered) (can just sort the header)

			if (Pak_Compress (pakurl))
				Con_Printf ("Pak compress successful\n");

			else Con_Printf ("Pak compress failed\n");

			Recent_File_Set_FullPath (pakurl);
			return;

		case arg_extract:	// extract file to <dest> (final arg is not folder but entire path of file to write!!!!)

			//File_URL_Edit_SlashesForward_Like_Unix (pakurl);
			c_strlcpy (inside_pak_filename, arg1); File_URL_Edit_SlashesForward_Like_Unix (inside_pak_filename);
			c_strlcpy (fileurl, arg2); File_URL_Edit_SlashesForward_Like_Unix (fileurl);

			// Validate destination folder, making sure it exists
			c_strlcpy (destfolder, fileurl);
			File_URL_Edit_Reduce_To_Parent_Path (destfolder);

			if (!File_Exists (destfolder))
			{
				Con_Printf ("Destination folder for \"%s\" does not exist\n", arg2);
				return;
			}

			if (!File_Is_Folder(destfolder))
			{
				Con_Printf ("Destination folder for \"%s\" is a file\n", arg2);
				return;
			}

			// Validate file to extract exists
			c_strlcpy (inside_pak_filename, arg1);
			File_URL_Edit_SlashesForward_Like_Unix (inside_pak_filename);

			if (!Pak_Has_File (pakurl, inside_pak_filename))
			{
				Con_Printf ("File \"%s\" does not exist in pak \"%s\"\n", inside_pak_filename, pakurl);
				return;
			}

			Pak_Extract_File (pakurl, inside_pak_filename, fileurl /*dest*/);

			Recent_File_Set_FullPath (fileurl);
			return;

		case arg_rem:		// remove a file <delete a file inside pak by zeroing out entry>

			// Validate file to zero out inside pak exists
			c_strlcpy (inside_pak_filename, arg1);
			File_URL_Edit_SlashesForward_Like_Unix (inside_pak_filename);

			if (!Pak_Has_File (pakurl, inside_pak_filename))
			{
				Con_Printf ("File \"%s\" does not exist in pak \"%s\"\n", inside_pak_filename, pakurl);
				return;
			}

			Pak_Remove_File (pakurl, inside_pak_filename);
			return;

		case arg_replace:	return; // replace a file in pak <if larger, null out entry and write another one>

			// Validate source
			c_strlcpy (fileurl, arg2);
			File_URL_Edit_SlashesForward_Like_Unix (fileurl);

			if (!File_Exists (fileurl))
			{
				Con_Printf ("File source \"%s\" does not exist\n", fileurl);
				return;
			}

			// Validate requested file to replace exists
			c_strlcpy (inside_pak_filename, arg1);
			File_URL_Edit_SlashesForward_Like_Unix (inside_pak_filename);

			if (!Pak_Has_File (pakurl, inside_pak_filename))
			{
				Con_Printf ("File \"%s\" does not exist in pak \"%s\"\n", inside_pak_filename, pakurl);
				return;
			}
#pragma message ("Baker: Has pak replace ever been tested?")
			Pak_Replace_File (pakurl, inside_pak_filename /*replaced*/ , fileurl);
			return;

		case arg_unzip: // arg1 is destination

			// Validate destination
			c_strlcpy (destfolder, arg1);
			File_URL_Edit_SlashesForward_Like_Unix (destfolder);

			if (!File_Exists (destfolder))
			{
				char parentpath[MAX_OSPATH];
				c_strlcpy (parentpath, destfolder);
				File_URL_Edit_Reduce_To_Parent_Path (parentpath);

				if (!File_Exists(parentpath))
				{
					Con_Printf ("Destination \"%s\" does not exist\n", destfolder);
					return;
				}
				// Ok to make just a subfolder
			}

			i = Pak_Unzip (pakurl, destfolder);

			Con_Printf ("%i files extracted\n", i);
			Recent_File_Set_FullPath (destfolder);
			return;

		case arg_zip: // arg1 is source

			// Validate source
			c_strlcpy (srcfolder, arg1);
			File_URL_Edit_SlashesForward_Like_Unix (srcfolder);

			if (!File_Exists (srcfolder))
			{
				Con_Printf ("Source folder \"%s\" does not exist\n", srcfolder);
				return;
			}

			Pak_Zip_Folder (pakurl, srcfolder);

			Recent_File_Set_FullPath (pakurl);
			return;

		case arg_help: // list current mappings

			Con_Printf ("\n");
			Con_Printf ("Usage: %s <0-99> <yourfile.mp3>\nwhere yourfile.mp3 is in [gamedir]/music folder\n", line->args[0]);
			Con_Printf ("\n");
			Con_Printf ("Usage: %s list  - print list of contents\n", line->args[0]);
			Con_Printf ("Usage: %s zip <folder_to_zip> <output.pak> - resets everything\n", line->args[0]);
			Con_Printf ("Usage: %s unzip <input.pak> <folder_unzip_location>\n", line->args[0]);
			Con_Printf ("\n");
			Con_Printf ("Examples:\n");
			Con_Printf ("\n");
			Con_Printf ("%s unzip c:\\quake\\id1\\pak.0.pak c:\\quake2\n", line->args[0]);
			Con_Printf ("%s zip c:\\quake\\id1\\maps c:\\quake\\id1\\mymaps.pak\n", line->args[0]);
			Con_Printf ("\n");
			Con_Printf ("Filenames should avoid spaces and only use alphanumeric\n");
			Con_Printf ("characters and the underscore '_'.\n");
			Con_Printf ("\n");
			Con_Printf ("Type 'folder' to access current gamedir.\n");
			Con_Printf ("\n");

			return;

		case arg_list: // list current mappings

			Con_Printf ("\n");
			Con_Printf ("Files in pak \"%s\":\n", _pakurl);
			Con_Printf ("\n");

			Pak_List_Print (pakurl);

			Con_Printf ("\n");

			Con_Printf ("Savings by compressing pak is %u\n", (unsigned int) Pak_Is_Compressable(pakurl));

			Recent_File_Set_FullPath (pakurl);
			return;

		} // End of switch

	} // End of args >=2

	// with no parameters or invalid parameters ends up displaying help
	Con_Printf ("\n");
	Con_Printf ("Usage: %s {help|list|unzip|zip}\n", line->args[0]);
	Con_Printf ("Type '%s help' for examples and detail\n", line->args[0]);
	Con_Printf ("\n");

}

// compress, extract, rem, replace will remain unlisted as to not confuse users.
// compress, extract, rem, replace will remain unlisted as to not confuse users.
void Zip_Command_f (lparse_t *line)
{
	// replace file, delete file, compress


	if (line->count >= 2)
	{
		const char *parm2 = line->args[1];
		const char *_zipurl = line->count >= 3 ? line->args[2] : NULL;
		const char *arg1 = line->count >= 4 ? line->args[3] : NULL;
		const char *arg2 = line->count >=5 ? line->args[4] : NULL;

		char zipurl[MAX_OSPATH];
		char fileurl[MAX_OSPATH];
		char destfolder[MAX_OSPATH];
		char srcfolder[MAX_OSPATH];
		char inside_zip_filename[MAX_OSPATH];

		int cmd_num = String_To_Array_Index (parm2, cmds);
		int numargs = line->count - 2;
		int i;
		cbool validnum = cmd_num < max_args;
		cbool meets_args = validnum ? args_required[cmd_num] == numargs : false;

		if (!validnum)
		{
			Con_Printf ("Invalid zip command %s\n", parm2);
			return;
		}

		if (!meets_args)
		{
			Con_Printf ("Wrong number of arguments for \"%s\": needs %i, have %i\n", parm2, args_required[cmd_num], numargs);
			Con_Printf ("Usage: %s %s %s\n", line->args[0], parm2, arg_msg[cmd_num]);
			return;
		}

		c_strlcpy (zipurl, _zipurl);
		File_URL_Edit_SlashesForward_Like_Unix (zipurl);

		if (cmd_num != arg_zip && !File_Exists (zipurl))
		{
			Con_Printf ("File \"%s\" doesn't exist\n", _zipurl); // Print the unmodified version
			return;
		}

		switch (cmd_num)
		{
		case arg_add:  // sorts and rewrites (where nulled entry space is recovered) (can just sort the header)
			Con_Printf ("Unsupported at this time\n");
			return;


		case arg_compress:  // sorts and rewrites (where nulled entry space is recovered) (can just sort the header)

			Con_Printf ("Unsupported at this time\n");
			return;

		case arg_extract:	// extract file to <dest> (final arg is not folder but entire path of file to write!!!!)

			//File_URL_Edit_SlashesForward_Like_Unix (zipurl);
			c_strlcpy (inside_zip_filename, arg1); File_URL_Edit_SlashesForward_Like_Unix (inside_zip_filename);
			c_strlcpy (fileurl, arg2); File_URL_Edit_SlashesForward_Like_Unix (fileurl);

			// Validate destination folder, making sure it exists
			c_strlcpy (destfolder, fileurl);
			File_URL_Edit_Reduce_To_Parent_Path (destfolder);

			if (!File_Exists (destfolder))
			{
				Con_Printf ("Destination folder for \"%s\" does not exist\n", arg2);
				return;
			}

			if (!File_Is_Folder(destfolder))
			{
				Con_Printf ("Destination folder for \"%s\" is a file\n", arg2);
				return;
			}

			// Validate file to extract exists
			c_strlcpy (inside_zip_filename, arg1);
			File_URL_Edit_SlashesForward_Like_Unix (inside_zip_filename);

			if (!Zip_Has_File (zipurl, inside_zip_filename))
			{
				Con_Printf ("File \"%s\" does not exist in zip \"%s\"\n", inside_zip_filename, zipurl);
				return;
			}

			Zip_Extract_File (zipurl, inside_zip_filename, fileurl /*dest*/);

			Recent_File_Set_FullPath (fileurl);
			return;

		case arg_rem:		// remove a file <delete a file inside zip by zeroing out entry>

			Con_Printf ("Unsupported at this time\n");
			return;

		case arg_replace:	return; // replace a file in zip <if larger, null out entry and write another one>

			Con_Printf ("Unsupported at this time\n");
			return;

		case arg_unzip: // arg1 is destination

			// Validate destination
			c_strlcpy (destfolder, arg1);
			File_URL_Edit_SlashesForward_Like_Unix (destfolder);

			if (!File_Exists (destfolder))
			{
				char parentpath[MAX_OSPATH];
				c_strlcpy (parentpath, destfolder);
				File_URL_Edit_Reduce_To_Parent_Path (parentpath);

				if (!File_Exists(parentpath))
				{
					Con_Printf ("Destination \"%s\" does not exist\n", destfolder);
					return;
				}
				// Ok to make just a subfolder
			}

			i = Zip_Unzip (zipurl, destfolder);

			Con_Printf ("%i files extracted\n", i);
			Recent_File_Set_FullPath (destfolder);
			return;

		case arg_zip: // arg1 is source

			// Validate source
			c_strlcpy (srcfolder, arg1);
			File_URL_Edit_SlashesForward_Like_Unix (srcfolder);

			if (!File_Exists (srcfolder))
			{
				Con_Printf ("Source folder \"%s\" does not exist\n", srcfolder);
				return;
			}

			Zip_Zip_Folder (zipurl, srcfolder);

			Recent_File_Set_FullPath (zipurl);
			return;

		case arg_help: // list current mappings

			Con_Printf ("\n");
			Con_Printf ("Usage: %s <0-99> <yourfile.mp3>\nwhere yourfile.mp3 is in [gamedir]/music folder\n", line->args[0]);
			Con_Printf ("\n");
			Con_Printf ("Usage: %s list  - print list of contents\n", line->args[0]);
			Con_Printf ("Usage: %s zip <folder_to_zip> <output.zip> - resets everything\n", line->args[0]);
			Con_Printf ("Usage: %s unzip <input.zip> <folder_unzip_location>\n", line->args[0]);
			Con_Printf ("\n");
			Con_Printf ("Examples:\n");
			Con_Printf ("\n");
			Con_Printf ("%s unzip c:\\quake\\id1\\zip.0.zip c:\\quake2\n", line->args[0]);
			Con_Printf ("%s zip c:\\quake\\id1\\maps c:\\quake\\id1\\mymaps.zip\n", line->args[0]);
			Con_Printf ("\n");
			Con_Printf ("Filenames should avoid spaces and only use alphanumeric\n");
			Con_Printf ("characters and the underscore '_'.\n");
			Con_Printf ("\n");
			Con_Printf ("Type 'folder' to access current gamedir.\n");
			Con_Printf ("\n");

			return;

		case arg_list: // list current mappings

			Con_Printf ("\n");
			Con_Printf ("Files in zip \"%s\":\n", _zipurl);
			Con_Printf ("\n");

			Zip_List_Print (zipurl);

			Con_Printf ("\n");

			Recent_File_Set_FullPath (zipurl);
			return;

		} // End of switch

	} // End of args >=2

	// with no parameters or invalid parameters ends up displaying help
	Con_Printf ("\n");
	Con_Printf ("Usage: %s {help|list|unzip|zip}\n", line->args [0]);
	Con_Printf ("Type '%s help' for examples and detail\n", line->args [0]);
	Con_Printf ("\n");

}


void Dir_Command_f (lparse_t *line)
{
	clist_t *dir_contents = File_List_Recursive_Alloc (com_gamedir);
	clist_t *cursor;
	double filesizes;
	size_t cursize;
	const char *comparestring = NULL;

	if (line->count == 2)
		comparestring = line->args[1];

	for (cursor = dir_contents, filesizes = 0; cursor; cursor = cursor->next)
	{
		// Skip disqualified matches
		if (comparestring && !wildmultcmp (comparestring, File_URL_SkipPath (cursor->name)) )
			continue;
		cursize = File_Length(cursor->name),
		Con_Printf ("%s %u\n", cursor->name, (unsigned int) cursize); // C89
		filesizes += cursize;
	}

	Con_Printf ("Sizes combined %f\n", filesizes);
	List_Free (&dir_contents);
}



void Install_Download_Before (const char *filename_any)
{
	memset (&cls.download, 0, sizeof(cls.download));

	c_strlcpy (cls.download.name, File_URL_SkipPath(filename_any));
	cls.download.percent = 0;
	cls.download.total_bytes = -1;
	cls.download.is_blocking = true;

	S_ClearBuffer ();		// so dma doesn't loop current sound

	key_count = -1;		// wait for a key down and up

	SCR_UpdateScreen ();
}

static cbool Install_Command_Progress (void *id, int old_total, int new_total)
{
	float percent = cls.download.total_bytes > 0 ?  (float)new_total / cls.download.total_bytes : 0;
	float newpct = CLAMP(0, percent, 1);
	
	if (newpct - cls.download.percent > 0.005)
	{
		// Every once in a while update the screen
		cls.download.percent = newpct;

		System_SendKeyEvents ();	// Check key events
		SCR_UpdateScreen ();		// Hmmm.
		key_count = -1;				// Necessary?

		if (key_lastpress == K_ESCAPE)
			return (cls.download.user_cancelled = true); // Bail!
	}

	return false; // Returns true to cancel, right?
}

void Install_Download_After (void)
{
//	int sizer = sizeof(cls.download);
	
	memset (&cls.download, 0, sizeof(cls.download));

	// make sure we don't ignore the next keypress
	if (key_count < 0)
		key_count = 0;
}

void UnInstall_Command_f (lparse_t *line)
{
	const char *subdir = NULL;
	int fail_reason = 0;
	char game_folder_url[MAX_OSPATH];
	char library_url[MAX_OSPATH];
	cbool dofull = false;
	cbool ret;
	
// What if game is in use?
	if (cmd_from_server || cls.demoplayback || cmd_source == src_client)
		return; // Not allowed

	if (cls.state == ca_connected || cls.demoplayback) // Demo playback + download screen updates causes R_RenderView to run out of stack space for WinQuake.
	{
		Con_Printf ("Disconnect first please\n");
		return;
	}

	if (strcasecmp(gamedir_shortname(), GAMENAME) || com_gametype != gametype_standard)
	{
		Con_Printf ("Set \"game id1\" first!  Currently in a gamedir\n");
		return;
	}


	dofull = (line->count == 3 && String_Does_Match_Caseless(line->args[2], "full"));

	if ( !(line->count == 2 || (line->count == 3 && dofull)))
	{
		Con_Printf ("Need the game to install or the entire URL with the http:// in it\n");
		Con_Printf ("Example: uninstall <modname> [full]\n");
		Con_Printf ("Note: Using 'full' will also remove zip from library\n");
		return;
	}

	subdir = line->args[1];
	if (!fail_reason && cmd_source == src_client) fail_reason = 1;
	if (!fail_reason && strstr(subdir, "..")) fail_reason = 2;
	if (!fail_reason && strstr(subdir, "/")) fail_reason = 3;
	if (!fail_reason && strstr(subdir, "\\")) fail_reason = 3;
	if (!fail_reason && String_Does_Match_Caseless(subdir, "id1")) fail_reason = 4;

	
	if (fail_reason)
	{
		const char *msgs[] = {"uninstall isn't permitted remotely", "relative path not allowed", "no path separators allowed", "can't remove id1"};
		const char *fail_msg = msgs[fail_reason - 1];
		Con_Printf ("%s: %s\n", line->args[0], fail_msg);
		return;
	}

	c_strlcpy (game_folder_url, basedir_to_url(subdir));

	if (File_Exists(game_folder_url) && !File_Is_Folder(game_folder_url))
	{
		Recent_File_Set_FullPath (game_folder_url);
		Con_Printf ("%s: \"%s\" is a file, you might consider removing it.\n", line->args[0], game_folder_url);
		Con_Printf ("Type 'showfile' to browse to that file.\n");  
		return;
	}

	if (!File_Exists(game_folder_url))
	{
		Con_Printf ("%s: folder \"%s\" does not exist\n", line->args[0], game_folder_url);
	}
	else
	{
		clist_t *file_list = NULL;
		
		Con_Printf ("Removing files in %s\n", subdir);
		file_list = File_List_Recursive_Alloc(game_folder_url);
		File_Delete_List (file_list);
		List_Free (&file_list);		

		Con_Printf ("Removing directories in %s\n", subdir);
		file_list = File_List_Dirs_Recursive_Alloc(game_folder_url);
		File_Rmdir_List (file_list);
		List_Free (&file_list);		

		Con_Printf ("Removing base folder in %s\n", subdir);

		ret = File_Rmdir (game_folder_url); 	
		if (!ret)
		{
			Recent_File_Set_FullPath (game_folder_url);				
			Con_Warning ("Couldn't remove base folder %s\n", subdir);
			Con_Warning ("Does the folder have other contents?\n");
			Con_Warning ("Is a file opened in another application?\n");
			Con_Warning ("Is the folder open in another window?\n");
			Con_Printf ("Type 'folder' to explore contents.\n");
			Con_Printf ("Try again when this is resolved.\n");
			return;
		} else Con_Printf ("Removed base folder in %s\n", subdir);

	}

	if (dofull)
	{
		c_strlcpy (library_url, downloads_folder_url(subdir));
		if (!String_Does_End_With_Caseless (library_url, ".zip"))
			c_strlcat (library_url, ".zip");
		
		if (!File_Exists(library_url))
			Con_Printf ("%s archive not present\n", library_url);
		else if (File_Is_Folder(library_url))
			Con_Printf ("%s is a folder!\n", library_url);
		else
		{
			Con_Printf ("Deleting archive %s\n", library_url);
			File_Delete (library_url);
		}
	}


//	
//	File_Mkdir_Recursive
//	File_Rmdir
	/*
	
	c_strlcat (".zip");
	clist_t * blah
	File_Delete_List
	File_Delete*/
// 
	Lists_Update_ModList ();
}

#if 0
void Http_Command_f (lparse_t *line)
{
//	char safedir_dem_url[MAX_OSPATH];	// c:\Users\ ....\Roaming\...\mydem.dem
	struct arg_buckets_64_s argbuckets = {0};
// Set user-agent
// URL
// Realloc buffer
// Timeout
// Fail how?
// Pass an update fn
	//const char *user_agent, update_fn, buffer, destfile, url

	c_snprintf (argbuckets.cmdline, "anything /h:%s", "http://www.quake-1.com/quakec-gallery/gyro2_21a.zip");
	String_To_Arg_Buckets (&argbuckets, argbuckets.cmdline);
	http_runner (argbuckets.argcount, argbuckets.argvs);

}
#endif

#define QUAKE_INJECTOR_USER_AGENT "(^Quakeinjector|^Java|DokuWiki HTTP Client)"
void Install_Command_f (lparse_t *line)
{
	char game_url[SYSTEM_STRING_SIZE_1024];

	if (cls.state == ca_connected || cls.demoplayback) // Demo playback + download screen updates causes R_RenderView to run out of stack space for WinQuake.
	{
		Con_Printf ("Disconnect first please\n");
		return;
	}
	
	if (line->count != 2)
	{
		Con_Printf ("Need the game to install or the entire URL with the http:// in it\n");
		Con_Printf ("Example: install travail or install [http://URL]\n");
		Con_Printf ("The version of libcurl used does not support https:// at this time\n");
		return;
	}
	else
	{
		const char *arg1 = line->args[1];
		if (!strstr(arg1, "/"))
		{
			// Be more flexible if someone types "install travail" (add .zip if necessary, fix case, etc.)
			c_snprintf2 (game_url, "%s/%s", install_depot_source.string, arg1);

			if (!String_Does_End_With_Caseless (game_url, ".zip"))
				c_strlcat (game_url, ".zip");

			// Convert to lower case.
			String_Edit_To_Lower_Case (game_url);
		}
		else 
		{
			if (!String_Does_End_With_Caseless (arg1, ".zip"))
			{
				Con_Printf ("Only .zip files are supported\n");
				return;
			}
			c_strlcpy (game_url, arg1);
		}
		
	}

	Con_DPrintf ("Source: %s\n", game_url);

	{ 
		const char *_library_folder_zip_url = downloads_folder_url(File_URL_SkipPath(game_url));
		const char *_install_game_folder_url = basedir_to_url (File_URL_SkipPath(game_url));
		char install_game_folder_url[MAX_OSPATH];
		char library_folder_zip_url[MAX_OSPATH];
		char download_cache_url[MAX_OSPATH];
		
		cbool did_download = false;

		c_strlcpy (library_folder_zip_url, _library_folder_zip_url); 
		c_snprintf2 (download_cache_url, "%s/%s", com_safedir, File_URL_SkipPath(game_url));

		// If folder already exists, leave it alone and do not download the file.
		// BUT the zip could just be maps and happen to have same name as a game folder?  Well, could rarely happen.  Tough.  Could stomp stuff.
		c_strlcpy (install_game_folder_url, _install_game_folder_url); 
		File_URL_Edit_Remove_Extension (install_game_folder_url); // If it is a game

		if (File_Exists(install_game_folder_url))
		{
			Recent_File_Set_FullPath (install_game_folder_url);

			if (File_Is_Folder(install_game_folder_url))
			{
				// Let the user examine the folder.
				Con_Printf ("It looks like this may already be installed.\n");
				Con_Printf ("Type 'folder' to explore contents.\n");
			}
			else
			{
				// Address only remotely possible extra stupid situation.  Delete file instead?
				Con_Printf ("Will not be able to make folder \"%s\" because a file by that name exists\n");
				Con_Printf ("Type 'showfile' to browse to that file.\n");  
			}
			return; // Get out
		}

		// If the archive exists, make sure it is valid.
		if (File_Exists(library_folder_zip_url))
		{		
			clist_t *file_list = Zip_List_Alloc (library_folder_zip_url);
			
			if (file_list == NULL)
			{
				Recent_File_Set_FullPath (library_folder_zip_url);
				Con_Printf ("There is an archive, but it contains no files or an invalid zip\n");
				Con_Printf ("Type 'showfile' to browse to that file.\n");
				return; // If file_list is null, don't need to free anything because there is nothing to free.
			}
			List_Free (&file_list);

			Con_Printf ("This archive exists in the library, no download required\n");
		}

		// Don't download if the file already exists, right?
		if (!File_Exists(library_folder_zip_url))
		{
			cbool is_success;
			cbool was_cancelled = false;
			int errorcode;
			Con_DPrintf ("Download: to %s\n", download_cache_url);
	
			// We don't need to make the folder or delete an existing file, the download proc does this.
//			Download_Set_User_Agent ("(^Quakeinjector|^Java|DokuWiki HTTP Client)");
			
			// Set download params
			Install_Download_Before (game_url); // set the cls.download stuff, stops sound and updates screen.

			is_success = Download_To_File(QUAKE_INJECTOR_USER_AGENT, game_url, download_cache_url, Install_Command_Progress, NULL, &cls.download.total_bytes, &errorcode);
#if 0
			{
				const char *mem = Download_To_Memory_Alloc (QUAKE_INJECTOR_USER_AGENT, game_url, Install_Command_Progress, NULL, &cls.download.total_bytes, &errorcode);
				if (mem)
				{
					File_Memory_To_File (download_cache_url, mem, cls.download.total_bytes);
					mem = core_free (mem);
				}
			}
#endif			
			
			was_cancelled = cls.download.user_cancelled;

			Install_Download_After ();  // clears cls.download stuff
			
			// Download to file will set is_success to false if the entire zip didn't download.
			if (!is_success)
			{
				Con_Printf ("download %s\n", was_cancelled ? "cancelled by user" : "failed or incomplete");
				File_Delete (download_cache_url); // If it is a partial, delete it.
				return; // Get out
			} else Con_Printf ("Download complete\n");
			did_download = true;
		}

		// If we are here, we have a zip to decompress
		{
			int zip_skipchars = -1; // It better not be that long.  Needs to contain either a map or a pak.
			enum found_what_e { found_none, found_game, found_map};
			enum found_what_e what_found = found_none; // Until we know otherwise.
			cbool contains_bsp = false, contains_pak = false, found_playable_content = false;
			const char *zip_check_url = did_download ? download_cache_url : library_folder_zip_url;


			// travail/pak0.pak vs pak0.pak  
			// travail/maps/mymap.bsp vs. maps/mymap.bsp
			// If we find a progs.dat or a pak0.pak we've got a mod.

			// Find basedir.
			clist_t *file_list = Zip_List_Alloc (zip_check_url);
			clist_t *cur;
			int count;

			for (cur = file_list, count = 0; cur; cur = cur->next, count ++)
			{
				const char *filename = File_URL_SkipPath(cur->name);
				cbool is_bsp = false, is_progs = false, is_pak = false, is_cruft = false;

				// If the zip is telling us about a folder, it has no length after the trailing forward slash, ignore + continue
				if (filename[0] == 0)
					continue;


				     if (String_Does_End_With_Caseless(filename, ".bsp"))		contains_bsp = is_bsp = true;
				else if (String_Does_Match_Caseless(filename, "progs.dat"))		is_progs = true;
				else if (String_Does_End_With_Caseless(filename, ".pak"))		contains_pak = is_pak = true;
				else															is_cruft = true; // Something else.

				if (found_playable_content == false && (is_bsp || is_progs || is_pak))
				{
					// Ok.  We have enough information to determine basedir.
			// travail/pak0.pak vs pak0.pak  
			// travail/maps/mymap.bsp vs. maps/mymap.bsp
					const char *thisname = cur->name;
					int slashlevel = is_bsp ? 1 : 0;
					int numslashes = String_Count_Char (thisname, '/');
					int nested_path_level = numslashes - slashlevel;
// Source and dest.
					found_playable_content = true;

					if ( is_bsp && numslashes == 0)
					{
						what_found = found_map;
						zip_skipchars = -1;
						break;
					}

					if (nested_path_level)
					{
						const char *begin_text;
						int i;
						for (i = 0, begin_text = cur->name; i < nested_path_level;  i ++)
							begin_text = strchr(begin_text, '/');
					
						zip_skipchars = begin_text - cur->name + 1;
						begin_text = &cur->name[zip_skipchars];
						Con_Printf ("%s ---> %s\n", cur->name, begin_text);
					}
				}

				if ( what_found != found_game && (is_progs || is_pak )  )
				{
					what_found = found_game;
					break;
				}

				if ( what_found == found_none && is_bsp)
					what_found = found_map;

				// If just a .bsp it will keep going.

			}

			
			if (what_found == found_none)
			{
				Con_Printf ("This archive has no maps, no paks and no progs.dat so appears to have no playable content.\n");
				if (did_download)
				{
					Con_Printf ("As we just downloaded this archive, removing the file from the library to prevent any confusion in the future.\n");
					File_Delete (download_cache_url); 
				}
				else  
				{
					Recent_File_Set_FullPath (library_folder_zip_url);
					Con_Printf ("File \"%s\" should not be in the library because of this and may cause interference.\n", File_URL_SkipPath(library_folder_zip_url));
					Con_Printf ("Type 'showfile' to browse to that file.\n");  
				}
			}
			else
			{
				const char *_dest_base[] = {install_game_folder_url, va("%s/" GAMENAME, com_basedir), va("%s/" GAMENAME "/maps", com_basedir)};
				char dest_base[MAX_OSPATH];
				int dnum = what_found == found_game ? 0 : what_found == found_map && zip_skipchars == -1 ? 2 : 1;

				c_strlcpy (dest_base, _dest_base[dnum]);

				if (did_download)
					File_Rename (download_cache_url, library_folder_zip_url);

				// Unpak time
				// 
				for (cur = file_list, count = 0; cur; cur = cur->next, count ++)
				{
					const char *dest_name;
					char lower_name[MAX_OSPATH];
					
					if (String_Does_End_With_Caseless(cur->name, ".exe"))
					{
						Con_Printf ("Note: Skipping an .exe file contained in the archive.\n");
						continue;  // No thanks!
					}

					if (String_Does_Match_Caseless(cur->name, "config.cfg"))
					{
						Con_Printf ("Warning:  Archive contained a config.cfg, but we are skipping that.\n");
						continue;  // No thanks!
					}

					if (String_Does_Match_Caseless(cur->name, "autoexec.cfg"))
					{
						// Allow but warn
						Con_Printf ("Warning:  This mod has an autoexec.cfg!\n");
					}

					if (String_Does_End_With (cur->name, "/") )
						continue; // Don't need bare folder

					if (cur->name[0] == 0)
						continue;

					// Let's have all extractions be lower case out of courtesy
					c_strlcpy (lower_name, cur->name);
					String_Edit_To_Lower_Case (lower_name);

					if (found_map && zip_skipchars == -1)
						dest_name = va("%s/%s", dest_base, lower_name); // Maps only
					else dest_name = va("%s/%s", dest_base, &lower_name[zip_skipchars]); // Other

					if (!Zip_Extract_File (library_folder_zip_url, cur->name, dest_name))
						Con_Printf ("Warning: Couldn't extract %s\n", cur->name);
					else Con_Printf ("Extracted: %s to %s\n", cur->name, dest_name);
				
				}
				// We either found a game, a map
				if (what_found == found_game)
					Lists_Update_ModList ();
				else Lists_Update_Maps ();
			}
			List_Free (&file_list);
		}

//		dest = basedir_to_url (va("id1/maps/%s", filename);


		// We don't know if it needs -quoth or something.  
		// We have no way of really knowing if it is an id1 map.
	} 
}

#if 0
void OpenD_Command_f (void)
{
//	Folder_Open
	Con_Printf ("%s\n", System_Dialog_Open_Type ("Open a file", "c:/quake/cda", ".pak,.png"));
}

void SaveD_Command_f (void)
{
}
#endif




