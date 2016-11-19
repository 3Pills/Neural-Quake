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

#ifndef __TRAIT_H__
#define __TRAIT_H__

typedef struct trait_s
{
	int id;
	double params[NQ_TRAIT_NUM_PARAMS];
} trait_t; // Collection of Genomes that hold a specific trait.

// Base Constructor
trait_t *Trait_Init(void);

// Initialize all values in the trait. 
// The first argument should be the number of arguments following it, the second 
// should be the ID of the trait, then a list of trait params of size NQ_TRAIT_NUM_PARAMS.
trait_t *Trait_Init_Values(int num_args, ...);

// Copy Constructor
trait_t *Trait_Init_Copy(trait_t* t);

// Creates a trait which averages 2 existing traits.
trait_t *Trait_Init_Merge(trait_t* t1, trait_t* t2);

void Trait_Delete(trait_t* trait);

void Trait_Mutate(trait_t* trait); // Perturb trait parameters slightly.
#endif // !__TRAIT_H__
*/