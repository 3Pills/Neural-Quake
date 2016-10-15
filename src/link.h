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
#ifndef __LINK_H__
#define __LINK_H__

#include "environment.h"
#include "neural_def.h"
#include "trait.h"

typedef struct neuron_s neuron_t;

typedef struct link_s
{
	double weight;

	neuron_t* inode; // Input neuron.
	neuron_t* onode; // Output neuron.

	cbool recurrent;
	cbool time_delay;

	trait_t *linktrait;
	int trait_id; // identify the trait derived by this link.

	double added_weight;
	double params[NQ_NUM_TRAIT_PARAMS];

} link_t; // Link between two neural nodes.

link_t Link_Init(double w, neuron_t* inode, neuron_t* onode, cbool recurring);
link_t Link_Init_Trait(trait_t* trait, double w, neuron_t* inode, neuron_t* onode, cbool recurring);
link_t Link_Init_Unknown(double w);
link_t Link_Init_Copy(const link_t link);

void Link_DeriveTrait(link_t* link, trait_t* curTrait);

#endif // !__LINK_H__