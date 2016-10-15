/*
Copyright (C) 2016 Stephen Koren

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
#include "trait.h"

trait_t Trait_Init(void)
{
	trait_t trait;

	for (int i = 0; i<NQ_NUM_TRAIT_PARAMS; i++)
		trait.params[i] = 0;
	trait.id = 0;

	return trait;
}



trait_t Trait_Init_Copy(const trait_t t) {
	trait_t trait;

	for (int i = 0; i<NQ_NUM_TRAIT_PARAMS; i++)
		trait.params[i] = t.params[i];
	trait.id = 0;

	return trait;
}

trait_t Trait_Init_Merge(const trait_t t1, const trait_t t2) {

}