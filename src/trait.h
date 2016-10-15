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
#ifndef __TRAIT_H__
#define __TRAIT_H__

#include "neural_def.h"

typedef struct trait_s
{
	int id;
	double params[NQ_NUM_TRAIT_PARAMS];
} trait_t; // Collection of Genomes that hold a specific trait.

trait_t Trait_Init(void); // Base Constructor
trait_t Trait_Init_Copy(const trait_t t); // Copy Constructor
trait_t Trait_Init_Merge(const trait_t t1, const trait_t t2); // Constructor which averages 2 existing traits.

void Trait_Mutate(); // Perturb trait parameters slightly.

#endif // !__TRAIT_H__