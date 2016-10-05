/*
Copyright (C) 2000	LordHavoc, Ender
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
// nehahra.h

#include <fmod/fmod.h>


#define NEHAHRA_VERSION	2.54
int	num_sfxorig;

void FMOD_Volume_Think (void);
void FMOD_Stop (void);
void FMOD_Close (void);
void FMOD_Pause (void);
void FMOD_Resume (void);

void Nehahra_Init (void);
void Nehahra_Shutdown (void);
void Neh_ResetSFX (void);
void Neh_Reset_Sfx_Count (void);

void SHOWLMP_drawall (void);
void SHOWLMP_clear (void);
void SHOWLMP_decodehide (void);
void SHOWLMP_decodeshow (void);

