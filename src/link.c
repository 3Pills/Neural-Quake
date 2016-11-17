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
#include "link.h"
#include "neural_def.h"
#include <stdlib.h>

nlink_t* Link_Init(double w, neuron_t* inode, neuron_t* onode, cbool recurring)
{
	nlink_t* link = malloc(sizeof(nlink_t));
	if (link == 0) return ((void*)1);

	link->weight = w;
	link->inode = inode;
	link->onode = onode;
	link->recurrent = recurring;
	link->added_weight = 0;
	link->time_delay = false;
	//link->trait = 0;
	//link->trait_id = 1;

	return link;
}

//nlink_t* Link_Init_Trait(trait_t* trait, double w, neuron_t* inode, neuron_t* onode, cbool recurring)
nlink_t* Link_Init_Trait(double w, neuron_t* inode, neuron_t* onode, cbool recurring)
{
	nlink_t* link = malloc(sizeof(nlink_t));
	if (link == 0) return ((void*)1);

	link->weight = w;
	link->inode = inode;
	link->onode = onode;
	link->recurrent = recurring;
	link->added_weight = 0;
	link->time_delay = false;
	//link->trait = trait;
	//if (trait != 0)
	//	link->trait_id = trait->id;
	//else link->trait_id = 1;

	return link;
}

nlink_t* Link_Init_Unknown(double w) 
{
	nlink_t* link = malloc(sizeof(nlink_t));
	if (link == 0) return ((void*)1);

	link->weight = w;
	link->inode = link->onode = 0;
	link->recurrent = false;
	link->time_delay = false;
	//link->trait = 0;
	//link->trait_id = 1;

	return link;
}

nlink_t* Link_Init_Copy(nlink_t* l)
{
	nlink_t* link = malloc(sizeof(nlink_t));
	if (link == 0) return ((void*)1);

	link->weight		= l->weight;
	link->inode			= l->inode;
	link->onode			= l->onode;
	link->recurrent		= l->recurrent;
	link->added_weight	= l->added_weight;
	link->time_delay	= l->time_delay;
	//link->trait			= l->trait;
	//link->trait_id		= l->trait_id;

	return link;
}

void Link_Delete(nlink_t* link)
{
	//Trait_Delete(link->trait);
	free(link);
}

/*
void Link_Derive_Trait(nlink_t* link, trait_t* curTrait)
{
	if (curTrait != 0)
	{
		for (int i = 0; i < NQ_TRAIT_NUM_PARAMS; i++)
			link->params[i] = (curTrait->params)[i];
	}
	else {
		for (int i = 0; i < NQ_TRAIT_NUM_PARAMS; i++)
			link->params[i] = 0;
	}

	link->trait_id = (curTrait != 0) ? curTrait->id : 1;
}
*/