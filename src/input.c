/*
Copyright (C) 2013 Baker

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// input.c -- input

#include "quakedef.h"

void Input_Force_CenterView_f (lparse_t *unnused) { cl.viewangles[PITCH] = 0; }


typedef enum
{
	input_none,
	input_have_keyboard,
	input_have_mouse_keyboard,
	input_have_windowskey,
} input_state_t;

typedef struct
{
	input_state_t	current_state;
	cbool		initialized, have_mouse, have_keyboard;
	cbool		disabled_windows_key;

// Internals
	mrect_t			mouse_clip_screen_rect;
	int				mouse_accum_x, mouse_accum_y;
	int				mouse_old_button_state;
} inp_info_t;


#define MRECT_PRINT(_x) _x.left, _x.top, _x.right, _x.bottom, _x.center_x, _x.center_y
enum { GET_IT = 1, LOSE_IT = 2 };


keyvalue_t input_state_text [] =
{
	KEYVALUE (input_none),
	KEYVALUE (input_have_keyboard),
	KEYVALUE (input_have_mouse_keyboard),
NULL, 0 };  // NULL termination

static inp_info_t inps;


void Input_Info_f (void)
{
	Con_Printf ("IN Info ...\n");
	Con_Printf ("%-25s :  %s\n", "current_state", KeyValue_GetKey (input_state_text, inps.current_state) );
	Con_Printf ("%-25s :  %i\n", "initialized", inps.initialized);
	Con_Printf ("%-25s :  %i\n", "have_mouse", inps.have_mouse);
	Con_Printf ("%-25s :  %i\n", "have_keyboard", inps.have_keyboard);
	Con_Printf ("%-25s :  %i\n", "disabled_windows_key", inps.disabled_windows_key);
	Con_Printf ("%-25s :  (%i, %i)-(%i, %i) center: %i, %i\n", "mouse_clip_screen_rect:", MRECT_PRINT(inps.mouse_clip_screen_rect) );
	Con_Printf ("%-25s :  %i\n", "mouse_accum_x", inps.mouse_accum_x);
	Con_Printf ("%-25s :  %i\n", "mouse_accum_y", inps.mouse_accum_y);
	Con_Printf ("%-25s :  %i\n", "mouse_old_button_state", inps.mouse_old_button_state);
}

#pragma message ("OS X mouse input has to be purely event oriented, we can't just nab the screen at any given time")
#ifdef PLATFORM_OSX
void Input_Think (void) { }
#else
void Input_Think (void)
{
	input_state_t	newstate = (inps.initialized && vid.ActiveApp && !vid.Minimized && !vid.Hidden) ? input_have_keyboard : input_none;
	cbool		windowed_mouse_grab = !cl.paused && !console1.forcedup && ( key_dest == key_game  || key_dest == key_message || (key_dest == key_menu && m_keys_bind_grab));
	cbool		mouse_grab = (vid.screen.type == MODE_FULLSCREEN || windowed_mouse_grab);

	cbool		disable_windows_key = input_have_keyboard && vid.screen.type == MODE_FULLSCREEN;
//	cbool		can_mouse_track = inps.initialized && !vid.Minimized && !vid.Hidden) && dont have mouse

	if (disable_windows_key != inps.disabled_windows_key)
	{
		switch (disable_windows_key)
		{
		case true:
			Input_Local_Keyboard_Disable_Windows_Key (true);
			break;
		case false:
			Input_Local_Keyboard_Disable_Windows_Key (false);
			break;
		}

		inps.disabled_windows_key = disable_windows_key;
	}

	// newstate upgrades from should have "keyboard" to should have "mouse"
	// If the key_dest is game or we are binding keys in the menu
	if (newstate == input_have_keyboard && mouse_grab && in_nomouse.value == 0)
		newstate = input_have_mouse_keyboard;

#if 0
	Con_Printf ("current_state: %s (init %i active %i mini %i)\n", Keypair_String (input_state_text, inps.current_state),
		inps.initialized, vid.ActiveApp, vid.Minimized);
#endif

	if (newstate != inps.current_state)
	{ // New state.
		char	mouse_action	= ( newstate == input_have_mouse_keyboard && inps.have_mouse == false) ? GET_IT :  (( newstate != input_have_mouse_keyboard && inps.have_mouse == true) ? LOSE_IT : 0);
		char	keyboard_action = ( newstate != input_none && inps.have_keyboard == false) ? GET_IT :  (( newstate == input_none && inps.have_keyboard == true) ? LOSE_IT : 0);

#if 0
		Con_Printf ("State change\n");
#endif

		switch (keyboard_action)
		{
		case GET_IT:
			// Sticky keys
			Input_Local_Keyboard_Disable_Sticky_Keys (true);

			inps.have_keyboard = true;
			break;

		case LOSE_IT:
			// Note we still need our key ups when entering the console
			// Sticky keys, Window key reenabled

			Input_Local_Keyboard_Disable_Sticky_Keys (false);			// Key ups

			Key_Release_Keys ();

			inps.have_keyboard = false;
			break;
		}

		switch (mouse_action)
		{
		case GET_IT:

			// Load window screen coords to mouse_clip_screen_rect
			// And clip the mouse cursor to that area
			Input_Local_Update_Mouse_Clip_Region_Think (&inps.mouse_clip_screen_rect);

			// Hide the mouse cursor and attach it
			Input_Local_Capture_Mouse (true);

			// Center the mouse on-screen
			Input_Local_Mouse_Cursor_SetPos (inps.mouse_clip_screen_rect.center_x, inps.mouse_clip_screen_rect.center_y);

			// Clear movement accumulation
			inps.mouse_accum_x = inps.mouse_accum_y = 0;

			inps.have_mouse = true;
			break;

		case LOSE_IT:
			// Baker: We have to release the mouse buttons because we can no longer receive
			// mouse up events.
			Key_Release_Mouse_Buttons ();

			// Release it somewhere out of the way
			Input_Local_Mouse_Cursor_SetPos (inps.mouse_clip_screen_rect.right - 80, inps.mouse_clip_screen_rect.top + 80);

			// Release the mouse and show the cursor.  Also unclips mouse.
			Input_Local_Capture_Mouse (false);

			// Clear movement accumulation and buttons
			inps.mouse_accum_x = inps.mouse_accum_y = inps.mouse_old_button_state = 0;

			inps.have_mouse = false;
			break;
		}
		inps.current_state = newstate;
	}

	if (inps.have_mouse && Input_Local_Update_Mouse_Clip_Region_Think (&inps.mouse_clip_screen_rect) == true)
	{
		// Re-center the mouse cursor and clear mouse accumulation
		Input_Local_Mouse_Cursor_SetPos (inps.mouse_clip_screen_rect.center_x, inps.mouse_clip_screen_rect.center_y);
		inps.mouse_accum_x = inps.mouse_accum_y = 0;
	}

	// End of function
}


void Input_Mouse_Button_Event (int mstate)
{
	if (inps.have_mouse /*|| (key_dest == key_menu && m_keys_bind_grab)*/ )
	{  // perform button actions
		int i;
		for (i = 0 ; i < INPUT_NUM_MOUSE_BUTTONS ; i ++)
		{
			int button_bit = (1 << i);
			cbool button_pressed  =  (mstate & button_bit) && !(inps.mouse_old_button_state & button_bit);
			cbool button_released = !(mstate & button_bit) &&  (inps.mouse_old_button_state & button_bit);

			if (button_pressed || button_released)
				Key_Event (K_MOUSE1 + i, button_pressed ? true : false);
		}
		inps.mouse_old_button_state = mstate;
	}
}

void Input_Mouse_Accumulate (void)
{
	static last_key_dest;
	int new_mouse_x, new_mouse_y;

	Input_Think ();


	if (inps.have_mouse)
	{
		cbool nuke_mouse_accum = false;

		// Special cases: fullscreen doesn't release mouse so doesn't clear accum
		// when entering/exiting the console.  I consider those input artifacts.  Also
		// we simply don't want accum from fullscreen if not key_dest == key_game.
		if (vid.screen.type == MODE_FULLSCREEN)
		{
			if (cl.paused)
				nuke_mouse_accum = true;
			else
			{
				cbool in_game_or_message = (key_dest == key_game || key_dest == key_message);
				cbool was_in_game_or_message = (last_key_dest == key_game || last_key_dest == key_message);
				cbool entered_game_or_message = in_game_or_message && !was_in_game_or_message;
				if (entered_game_or_message || !in_game_or_message)
					nuke_mouse_accum = true;
			}
		}

		Input_Local_Mouse_Cursor_GetPos (&new_mouse_x, &new_mouse_y); // GetCursorPos (&current_pos);

		inps.mouse_accum_x += new_mouse_x - inps.mouse_clip_screen_rect.center_x;
		inps.mouse_accum_y += new_mouse_y - inps.mouse_clip_screen_rect.center_y;

		// Re-center the mouse cursor
		Input_Local_Mouse_Cursor_SetPos (inps.mouse_clip_screen_rect.center_x, inps.mouse_clip_screen_rect.center_y);

		if (nuke_mouse_accum)
			inps.mouse_accum_x = inps.mouse_accum_y = 0;
	}
	last_key_dest = key_dest;
}

void Input_Mouse_Move (usercmd_t *cmd)
{
	Input_Mouse_Accumulate ();

	if (inps.mouse_accum_x || inps.mouse_accum_y)
	{
		int	mouse_x = inps.mouse_accum_x *= sensitivity.value;
		int mouse_y = inps.mouse_accum_y *= sensitivity.value;
	// add mouse X/Y movement to cmd
		if ( (in_strafe.state & 1) || (lookstrafe.value && MOUSELOOK_ACTIVE ))
			cmd->sidemove += m_side.value * mouse_x;
		else cl.viewangles[YAW] -= m_yaw.value * mouse_x;

		if (MOUSELOOK_ACTIVE)
			View_StopPitchDrift ();

		if ( MOUSELOOK_ACTIVE && !(in_strafe.state & 1))
		{
			cl.viewangles[PITCH] += m_pitch.value * mouse_y;

			CL_BoundViewPitch (cl.viewangles);
		}
		else
		{
			if ((in_strafe.state & 1) && cl.noclip_anglehack)
				cmd->upmove -= m_forward.value * mouse_y;
			else cmd->forwardmove -= m_forward.value * mouse_y;
		}
		inps.mouse_accum_x = inps.mouse_accum_y = 0;
	}
}
#endif // !PLATFORM_OSX

void Input_Move (usercmd_t *cmd)
{
    Input_Mouse_Move (cmd);
    Input_Joystick_Move (cmd);
}


cbool joy_avail;

/*
===========
IN_JoyMove
===========
*/

//#ifdef _WIN32
//#include "winquake.h"
//#endif // _WIN32

void Input_Joystick_Move (usercmd_t *cmd)
{
}

void Input_Commands (void)
{
#ifdef PLATFORM_OSX
	Key_Console_Repeats ();
#endif // PLATFORM_OSX
	Input_Local_Joystick_Commands ();

}

void Input_Joystick_Init (void)
{
	// joystick variables
 	// assume no joystick
	joy_avail = Input_Local_Joystick_Startup();

	if (joy_avail)
	{
		Cmd_AddCommands (Input_Joystick_Init);

	}
}

void Input_Init (void)
{
	Cmd_AddCommands (Input_Init);

#pragma message ("Baker: Implement m_filter on Windows")

	if (COM_CheckParm ("-nomouse"))
		Cvar_SetValueQuick (&in_nomouse, 1);

	if (!COM_CheckParm ("-nojoy"))
		Input_Joystick_Init ();

	Input_Local_Init (); // Mac

	inps.initialized = true;
	Input_Think ();
	Con_Printf ("Input initialized\n");
}

void Input_Shutdown (void)
{
	Input_Local_Shutdown (); // Mac

	inps.initialized = false;
	Input_Think (); // Will shut everything off
}

