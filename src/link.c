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

link_t Link_Init(double w, neuron_t* inode, neuron_t* onode, cbool recurring)
{
	link_t link;

	link.weight = w;
	link.inode = inode;
	link.onode = onode;
	link.recurrent = recurring;
	link.added_weight = 0;
	link.linktrait = 0;
	link.time_delay = false;
	link.trait_id = 1;

	return link;
}

link_t Link_Init_Trait(trait_t* trait, double w, neuron_t* inode, neuron_t* onode, cbool recurring)
{
	link_t link;

	link.weight = w;
	link.inode = inode;
	link.onode = onode;
	link.recurrent = recurring;
	link.added_weight = 0;
	link.linktrait = trait;
	link.time_delay = false;
	if (trait != 0)
		link.trait_id = trait->id;
	else link.trait_id = 1;

	return link;
}

link_t Link_Init_Unknown(double w) 
{
	link_t link;

	link.weight = w;
	link.inode = link.onode = 0;
	link.recurrent = false;
	link.linktrait = 0;
	link.time_delay = false;
	link.trait_id = 1;

	return link;
}

link_t Link_Init_Copy(const link_t l)
{
	link_t link;

	link.weight = l.weight;
	link.inode = l.inode;
	link.onode = l.onode;
	link.recurrent = l.recurrent;
	link.added_weight = l.added_weight;
	link.linktrait = l.linktrait;
	link.time_delay = l.time_delay;
	link.trait_id = l.trait_id;

	return link;
}

void Link_DeriveTrait(link_t* link, trait_t* curTrait) 
{
	if (curTrait != 0)
	{
		for (int count = 0; count < NQ_NUM_TRAIT_PARAMS; count++)
			link->params[count] = (curTrait->params)[count];
	}
	else {
		for (int count = 0; count< NQ_NUM_TRAIT_PARAMS; count++)
			link->params[count] = 0;
	}

	if (curTrait != 0)
		link->trait_id = curTrait->id;
	else link->trait_id = 1;
}
