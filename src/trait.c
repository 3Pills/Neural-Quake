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
#include "neural.h"
#include "environment.h"

trait_t* Trait_Init()
{
	trait_t* trait = malloc(sizeof(trait_t));

	for (int i = 0; i<NQ_TRAIT_NUM_PARAMS; i++)
		trait->params[i] = 0;
	trait->id = 0;

	return trait;
}

trait_t* Trait_Init_Values(int num_args, ...)
{
	trait_t* trait = malloc(sizeof(trait_t));

	va_list var_args;
	va_start(var_args, num_args);

	//The trait's ID should always be the first argument.
	trait->id = va_arg(var_args, int);

	//Ignore all arguments past what we can contain. +1 because the ID is taking up a slot.
	num_args = fmin(num_args, NQ_TRAIT_NUM_PARAMS + 1);

	//Get all the params now, passing them into the trait.
	for (int i = 0; i < NQ_TRAIT_NUM_PARAMS; i++)
		trait->params[i] = (i+1 < num_args) ? va_arg(var_args, double) : 0;

	va_end(var_args);

	return trait;
}

trait_t* Trait_Init_Copy(trait_t* t) 
{
	trait_t* trait = malloc(sizeof(trait_t));

	for (int i = 0; i<NQ_TRAIT_NUM_PARAMS; i++)
		trait->params[i] = t->params[i];
	trait->id = 0;

	return trait;
}

trait_t* Trait_Init_Merge(trait_t* t1, trait_t* t2) 
{
	trait_t* trait = malloc(sizeof(trait_t));

	for (int i = 0; i<NQ_TRAIT_NUM_PARAMS; i++)
		trait->params[i] = (((t1->params)[i]) + ((t2->params)[i])) / 2.0;
	trait->id = 0;

	return trait;
}

void Trait_Delete(trait_t* trait)
{
	free(trait);
}

void Trait_Mutate(trait_t* trait) 
{
	for (int i = 0; i < NQ_TRAIT_NUM_PARAMS; i++) 
	{
		if (Random_Float() > NQ_TRAIT_PARAM_MUT_PROB) 
		{			
			trait->params[i] += (Random_Float() * Random_Sign()) * NQ_TRAIT_MUT_POWER;
			if (trait->params[i]<0.0) trait->params[i] = 0;
			if (trait->params[i]>1.0) trait->params[i] = 1.0;
		}
	}
}