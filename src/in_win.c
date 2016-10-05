/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2012 John Fitzgibbons and others
Copyright (C) 2009-2014 Baker and others

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
// in_win.c -- windows 95 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.


#include "quakedef.h"
#include "winquake.h"


int Input_Local_Capture_Mouse (cbool bDoCapture)
{
	static cbool captured = false;

	if (bDoCapture && !captured)
	{
		ShowCursor (FALSE); // Hides mouse cursor
		SetCapture (sysplat.mainwindow);	// Captures mouse events
		Con_DPrintf ("Mouse Captured\n");
		captured = true;
	}

	if (!bDoCapture && captured)
	{
		ShowCursor (TRUE); // Hides mouse cursor
		ReleaseCapture ();
		ClipCursor (NULL); // Can't hurt
		Con_DPrintf ("Mouse Released\n");
		captured = false;
	}

	return 1; // Accepted
}


cbool Input_Local_Update_Mouse_Clip_Region_Think (mrect_t* mouseregion)
{
	mrect_t oldregion = *mouseregion;
	WINDOWINFO windowinfo;
	windowinfo.cbSize = sizeof (WINDOWINFO);
	GetWindowInfo (sysplat.mainwindow, &windowinfo);	// client_area screen coordinates

	// Fill in top left, bottom, right, center
	mouseregion->left = windowinfo.rcClient.left;
	mouseregion->right = windowinfo.rcClient.right;
	mouseregion->bottom = windowinfo.rcClient.bottom;
	mouseregion->top = windowinfo.rcClient.top;

	if (memcmp (mouseregion, &oldregion, sizeof(mrect_t) ) != 0)
	{  // Changed!
		mouseregion->width = mouseregion->right - mouseregion->left;
		mouseregion->height = mouseregion->bottom - mouseregion->top;
		mouseregion->center_x = (mouseregion->left + mouseregion->right) / 2;
		mouseregion->center_y = (mouseregion->top + mouseregion->bottom) / 2;
		ClipCursor (&windowinfo.rcClient);
		return true;
	}
	return false;
}

void Input_Local_Mouse_Cursor_SetPos (int x, int y)
{
	SetCursorPos (x, y);
}

void Input_Local_Mouse_Cursor_GetPos (int *x, int *y)
{
	POINT current_pos;
	GetCursorPos (&current_pos);

	*x = current_pos.x;
	*y = current_pos.y;
}


STICKYKEYS StartupStickyKeys = {sizeof (STICKYKEYS), 0};
TOGGLEKEYS StartupToggleKeys = {sizeof (TOGGLEKEYS), 0};
FILTERKEYS StartupFilterKeys = {sizeof (FILTERKEYS), 0};


void AllowAccessibilityShortcutKeys (cbool bAllowKeys)
{
	static cbool initialized = false;

	if (!initialized)
	{	// Save the current sticky/toggle/filter key settings so they can be restored them later
		SystemParametersInfo (SPI_GETSTICKYKEYS, sizeof (STICKYKEYS), &StartupStickyKeys, 0);
		SystemParametersInfo (SPI_GETTOGGLEKEYS, sizeof (TOGGLEKEYS), &StartupToggleKeys, 0);
		SystemParametersInfo (SPI_GETFILTERKEYS, sizeof (FILTERKEYS), &StartupFilterKeys, 0);
		Con_DPrintf ("Accessibility key startup settings saved\n");
		initialized = true;
	}

	if (bAllowKeys)
	{
		// Restore StickyKeys/etc to original state
		// (note that this function is called "allow", not "enable"; if they were previously
		// disabled it will put them back that way too, it doesn't force them to be enabled.)
		SystemParametersInfo (SPI_SETSTICKYKEYS, sizeof (STICKYKEYS), &StartupStickyKeys, 0);
		SystemParametersInfo (SPI_SETTOGGLEKEYS, sizeof (TOGGLEKEYS), &StartupToggleKeys, 0);
		SystemParametersInfo (SPI_SETFILTERKEYS, sizeof (FILTERKEYS), &StartupFilterKeys, 0);

		Con_DPrintf ("Accessibility keys enabled\n");
	}
	else
	{
		// Disable StickyKeys/etc shortcuts but if the accessibility feature is on,
		// then leave the settings alone as its probably being usefully used
		STICKYKEYS skOff = StartupStickyKeys;
		TOGGLEKEYS tkOff = StartupToggleKeys;
		FILTERKEYS fkOff = StartupFilterKeys;

		if ((skOff.dwFlags & SKF_STICKYKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
			skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;

			SystemParametersInfo (SPI_SETSTICKYKEYS, sizeof (STICKYKEYS), &skOff, 0);
		}

		if ((tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
			tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;

			SystemParametersInfo (SPI_SETTOGGLEKEYS, sizeof (TOGGLEKEYS), &tkOff, 0);
		}

		if ((fkOff.dwFlags & FKF_FILTERKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
			fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;

			SystemParametersInfo (SPI_SETFILTERKEYS, sizeof (FILTERKEYS), &fkOff, 0);
		}

		Con_DPrintf ("Accessibility keys disabled\n");
	}
}



LRESULT CALLBACK LLWinKeyHook(int Code, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT p;
	p = (PKBDLLHOOKSTRUCT) lParam;

	if (vid.ActiveApp)
	{
		switch(p->vkCode)
		{
		case VK_LWIN:	// Left Windows Key
		case VK_RWIN:	// Right Windows key
		case VK_APPS: 	// Context Menu key

			return 1; // Ignore these keys
		}
	}

	return CallNextHookEx(NULL, Code, wParam, lParam);
}



void AllowWindowsShortcutKeys (cbool bAllowKeys)
{
	static cbool WinKeyHook_isActive = false;
	static HHOOK WinKeyHook;

	if (!bAllowKeys)
	{
		// Disable if not already disabled
		if (!WinKeyHook_isActive)
		{
			if (!(WinKeyHook = SetWindowsHookEx(13, LLWinKeyHook, sysplat.hInstance, 0)))
			{
				Con_Printf("Failed to install winkey hook.\n");
				Con_Printf("Microsoft Windows NT 4.0, 2000 or XP is required.\n");
				return;
			}

			WinKeyHook_isActive = true;
			Con_DPrintf ("Windows and context menu key disabled\n");
		}
	}

	if (bAllowKeys)
	{	// Keys allowed .. stop hook
		if (WinKeyHook_isActive)
		{
			UnhookWindowsHookEx(WinKeyHook);
			WinKeyHook_isActive = false;
			Con_DPrintf ("Windows and context menu key enabled\n");
		}
	}
}

void Input_Local_Keyboard_Disable_Sticky_Keys (cbool bDoDisable)
{
	if (bDoDisable)
	{
		AllowAccessibilityShortcutKeys (false);
	}
	else
	{
		AllowAccessibilityShortcutKeys (true);
	}
}

void Input_Local_Keyboard_Disable_Windows_Key (cbool bDoDisable)
{
	if (bDoDisable)
	{
		AllowWindowsShortcutKeys (false);
	}
	else
	{
		AllowWindowsShortcutKeys (true);
	}
}

#if 0
/*
=======
MapKey

Map from windows to quake keynums
=======
*/
int Input_Local_MapKey (int windowskey)
{
	int key = Input_Local_MapKey_ (windowskey); 
	const char *keyname = key ? Key_KeynumToString (key, key_local_name) : "ZERO";
	
	Con_Printf ("Key #%i = %s\n", key, keyname);
	return key;
}
#endif

int Input_Local_MapKey (int windowskey)
{
	static byte scantokey[128] =
	{
		0 ,	27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', K_BACKSPACE,
		9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', K_ENTER ,
		/*left*/ K_CTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
		/*left*/ K_SHIFT,'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', /*right*/ K_SHIFT, K_KP_STAR, /*left*/ K_ALT,' ', 0 /*K_CAPSLOCK*/ ,
		K_F1, K_F2, K_F3, K_F4, K_F5,K_F6, K_F7, K_F8, K_F9, K_F10, K_PAUSE, /*K_SCROLLLOCK*/ 0,
		K_HOME, K_UPARROW,K_PGUP, K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW, K_KP_PLUS ,K_END,
		K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0, 0, K_F11, K_F12,0 , 0 , /*K_LWIN*/ 0, /*K_RWIN*/ 0, /*K_MENU*/ 0, 0 , 0,
	};

	cbool extended = (windowskey >> 24) & 1;
	int key = (windowskey >> 16) & 255;

//	Con_Printf ("Winkey %i (%4x) Extended %i ... ", windowskey, windowskey, extended);

	if (key > 127)
		return 0;
	
	key = scantokey[key];
	if (in_keypad.value) 
	{
		if (extended) 
		{
			switch (key) 
			{
			case K_ENTER:		return K_KP_ENTER;
			case '/':			return K_KP_SLASH;
			case K_PAUSE:		return K_KP_NUMLOCK;
			}
		}
		else
		{
			switch (key) 
			{
			case K_HOME:		return K_KP_HOME;
			case K_UPARROW:		return K_KP_UPARROW;
			case K_PGUP:		return K_KP_PGUP;
			case K_LEFTARROW:	return K_KP_LEFTARROW;
			case K_RIGHTARROW:	return K_KP_RIGHTARROW;
			case K_END:			return K_KP_END;
			case K_DOWNARROW:	return K_KP_DOWNARROW;
			case K_PGDN:		return K_KP_PGDN;
			case K_INS:			return K_KP_INS;
			case K_DEL:			return K_KP_DEL;
			}
		}
	} 
	else 
	{
		// cl_keypad 0, compatibility mode
		switch (key) {
			case K_KP_STAR:		return '*';
			case K_KP_MINUS:	return '-';
			case K_KP_5:		return '5';
			case K_KP_PLUS:		return '+';
		}
	}
	
	return key;
}

cbool WIN_IN_ReadInputMessages (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int button_bits = 0;

	switch (Msg)
	{
	case WM_MOUSEWHEEL:
		if ((short) HIWORD(wParam) > 0)
		{
			Key_Event(K_MWHEELUP, true);
			Key_Event(K_MWHEELUP, false);
		}
		else
		{
			Key_Event(K_MWHEELDOWN, true);
			Key_Event(K_MWHEELDOWN, false);
		}
		return true;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		if (wParam & MK_LBUTTON)	button_bits |= 1;
		if (wParam & MK_RBUTTON)	button_bits |= 2;
		if (wParam & MK_MBUTTON)	button_bits |= 4;
		if (wParam & MK_XBUTTON1)	button_bits |= 8;
		if (wParam & MK_XBUTTON2)	button_bits |= 16;
		Input_Mouse_Button_Event (button_bits);

		return true;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		Key_Event (Input_Local_MapKey (lParam), true);
		return true;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		Key_Event (Input_Local_MapKey(lParam), false);
		return true;

	default:
		return false;
	}
}

/*

  Joystick

*/

// joystick defines and variables
// where should defines be moved?
#define JOY_ABSOLUTE_AXIS	0x00000000		// control like a joystick
#define JOY_RELATIVE_AXIS	0x00000010		// control like a mouse, spinner, trackball
#define	JOY_MAX_AXES		6			// X, Y, Z, R, U, V
#define JOY_AXIS_X			0
#define JOY_AXIS_Y			1
#define JOY_AXIS_Z			2
#define JOY_AXIS_R			3
#define JOY_AXIS_U			4
#define JOY_AXIS_V			5



DWORD dwAxisFlags[JOY_MAX_AXES] =
{
	JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ, JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
};

DWORD	joyAxisMap[JOY_MAX_AXES];
DWORD	dwControlMap[JOY_MAX_AXES];
PDWORD	pdwRawValue[JOY_MAX_AXES];

// none of these cvars are saved over a session.
// this means that advanced controller configuration needs to be executed each time.
// this avoids any problems with getting back to a default usage or when changing from one controller to another.
// this way at least something works.

cbool joy_advancedinit;
static cbool	joy_haspov;
static DWORD	joy_oldbuttonstate, joy_oldpovstate;

static int		joy_id;
static DWORD	joy_flags;
static DWORD	joy_numbuttons;


static	JOYINFOEX	ji;

PDWORD RawValuePointer (int axis)
{
	switch (axis)
	{
		case JOY_AXIS_X: return &ji.dwXpos;
		case JOY_AXIS_Y: return &ji.dwYpos;
		case JOY_AXIS_Z: return &ji.dwZpos;
		case JOY_AXIS_R: return &ji.dwRpos;
		case JOY_AXIS_U: return &ji.dwUpos;
		case JOY_AXIS_V: return &ji.dwVpos;
	}

	return NULL;	// shut up compiler
}

void Input_Local_Joy_AdvancedUpdate_f (lparse_t *unused)
{

	// called once by IN_ReadJoystick and by user whenever an update is needed
	// cvars are now available
	int	i;
	DWORD	dwTemp;

	// initialize all the maps
	for (i = 0 ; i < JOY_MAX_AXES ; i++)
	{
		joyAxisMap[i] = eAxisNone;
		dwControlMap[i] = JOY_ABSOLUTE_AXIS;
		pdwRawValue[i] = RawValuePointer(i);
	}

	if (!joy_advanced.value)
	{
		// default joystick initialization
		// 2 axes only with joystick control
		joyAxisMap[JOY_AXIS_X] = eAxisTurn;
		// dwControlMap[JOY_AXIS_X] = JOY_ABSOLUTE_AXIS;
		joyAxisMap[JOY_AXIS_Y] = eAxisForward;
		// dwControlMap[JOY_AXIS_Y] = JOY_ABSOLUTE_AXIS;
	}
	else
	{
		if (strcmp (joy_name.string, "joystick"))
		{
			// notify user of advanced controller
			Con_Printf ("\n%s configured\n\n", joy_name.string);
		}

		// advanced initialization here
		// data supplied by user via joy_axisn cvars
		dwTemp = (DWORD) joy_advaxisx.value;
		joyAxisMap[JOY_AXIS_X] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_X] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisy.value;
		joyAxisMap[JOY_AXIS_Y] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Y] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisz.value;
		joyAxisMap[JOY_AXIS_Z] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Z] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisr.value;
		joyAxisMap[JOY_AXIS_R] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_R] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisu.value;
		joyAxisMap[JOY_AXIS_U] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_U] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisv.value;
		joyAxisMap[JOY_AXIS_V] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_V] = dwTemp & JOY_RELATIVE_AXIS;
	}

	// compute the axes to collect from DirectInput
	joy_flags = JOY_RETURNCENTERED | JOY_RETURNBUTTONS | JOY_RETURNPOV;
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		if (joyAxisMap[i] != eAxisNone)
			joy_flags |= dwAxisFlags[i];
	}
}

cbool Input_Local_Joystick_Startup (void)
{
	int		numdevs;
	JOYCAPS		jc;
	MMRESULT	mmr;

	// verify joystick driver is present
	if ((numdevs = joyGetNumDevs ()) == 0)
	{
		Con_Printf ("\njoystick not found -- driver not present\n\n");
		return false;
	}

	// cycle through the joystick ids for the first valid one
	for (joy_id = 0 ; joy_id < numdevs ; joy_id++)
	{
		memset (&ji, 0, sizeof(ji));
		ji.dwSize = sizeof(ji);
		ji.dwFlags = JOY_RETURNCENTERED;

		if ((mmr = joyGetPosEx(joy_id, &ji)) == JOYERR_NOERROR)
			break;
	}

	// abort startup if we didn't find a valid joystick
	if (mmr != JOYERR_NOERROR)
	{
		Con_Printf ("joystick not found -- no valid joysticks (%x)\n", mmr);
		return false;
	}

	// get the capabilities of the selected joystick
	// abort startup if command fails
	memset (&jc, 0, sizeof(jc));
	if ((mmr = joyGetDevCaps(joy_id, &jc, sizeof(jc))) != JOYERR_NOERROR)
	{
		Con_Printf ("joystick not found -- invalid joystick capabilities (%x)\n", mmr);
		return false;
	}

	// save the joystick's number of buttons and POV status
	joy_numbuttons = jc.wNumButtons;
	joy_haspov = jc.wCaps & JOYCAPS_HASPOV;

	// old button and POV states default to no buttons pressed
	joy_oldbuttonstate = joy_oldpovstate = 0;

	// mark the joystick as available and advanced initialization not completed
	// this is needed as cvars are not available during initialization

	joy_advancedinit = false;
#pragma message ("Command rogues")
	Cmd_AddCommands ((voidfunc_t)Input_Local_Joystick_Startup); // Warning because Input_Local_Joystick_Startup is cbool return not void

	Con_Printf ("\njoystick detected\n\n");
	return true;
}



void Input_Local_Joystick_Commands (void)
{
	int	i, key_index;
	DWORD	buttonstate, povstate;

	if (!joy_avail)
		return;

	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for each state change
	buttonstate = ji.dwButtons;
	for (i = 0 ; i < (int)joy_numbuttons ; i++)
	{
		if ((buttonstate & (1<<i)) && !(joy_oldbuttonstate & (1<<i)))
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, true);
		}

		if (!(buttonstate & (1<<i)) && (joy_oldbuttonstate & (1<<i)))
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, false);
		}
	}
	joy_oldbuttonstate = buttonstate;

	if (joy_haspov)
	{
		// convert POV information into 4 bits of state information
		// this avoids any potential problems related to moving from one
		// direction to another without going through the center position
		povstate = 0;
		if(ji.dwPOV != JOY_POVCENTERED)
		{
			if (ji.dwPOV == JOY_POVFORWARD)
				povstate |= 0x01;
			if (ji.dwPOV == JOY_POVRIGHT)
				povstate |= 0x02;
			if (ji.dwPOV == JOY_POVBACKWARD)
				povstate |= 0x04;
			if (ji.dwPOV == JOY_POVLEFT)
				povstate |= 0x08;
		}
		// determine which bits have changed and key an auxillary event for each change
		for (i=0 ; i<4 ; i++)
		{
			if ((povstate & (1<<i)) && !(joy_oldpovstate & (1<<i)))
				Key_Event (K_AUX29 + i, true);

			if (!(povstate & (1<<i)) && (joy_oldpovstate & (1<<i)))
				Key_Event (K_AUX29 + i, false);

		}
		joy_oldpovstate = povstate;
	}
}

cbool Input_Local_Joystick_Read (void)
{
	memset (&ji, 0, sizeof(ji));

	ji.dwSize = sizeof(ji);
	ji.dwFlags = joy_flags;

	if (joyGetPosEx(joy_id, &ji) == JOYERR_NOERROR)
	{
		// this is a hack -- there is a bug in the Logitech WingMan Warrior DirectInput Driver
		// rather than having 32768 be the zero point, they have the zero point at 32668
		// go figure -- anyway, now we get the full resolution out of the device
		if (joy_wwhack1.value != 0.0)
			ji.dwUpos += 100;

		return true;
	}
	else
	{
		// read error occurred
		// turning off the joystick seems too harsh for 1 read error,\
		// but what should be done?
		// Con_Printf ("IN_ReadJoystick: no response\n");
		// joy_avail = false;
		return false;
	}
}



void Input_Local_Joystick_Shutdown (void)
{
	// We don't have to do anything here

}


void Input_Local_Init (void)
{

}

void Input_Local_Shutdown (void)
{

}


// Baker: On Windows these might not only be key events.
void Input_Local_SendKeyEvents (void)
{
    MSG        msg;

	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
	// we always update if there are any event, even if we're paused
		scr_skipupdate = 0;

		if (!GetMessage (&msg, NULL, 0, 0))
			System_Quit ();

      	TranslateMessage (&msg);
      	DispatchMessage (&msg);
	}
}

// Baker: Stops drag flag on Mac (when activation is received by a mouseclick on title bar and user drags it.
//  On Windows do this too.
void Input_Local_Deactivate (void)
{


}

