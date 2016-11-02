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
#include "neuron.h"



neuron_t* Neuron_Init(enum nodetype_e type, int node_id)
{
	neuron_t* neuron = malloc(sizeof(neuron_t));
	if (neuron == 0) return ((void*)1);

	neuron->active_flag = false;
	neuron->activesum = 0;
	neuron->activation = 0;
	neuron->value = 0;
	neuron->last_activation = 0;
	neuron->last_activation2 = 0;
	neuron->type = type; //NEURON or SENSOR type
	neuron->activation_count = 0; //Inactive upon creation
	neuron->node_id = node_id;
	neuron->fType = NQ_SIGMOID;
	neuron->trait = 0;
	neuron->node_label = NQ_HIDDEN;
	neuron->dupe = 0;
	neuron->analogue = 0;
	neuron->frozen = false;
	neuron->trait_id = 1;
	neuron->override = false;
	neuron->links_in = vector_init();
	neuron->links_out = vector_init();

	return neuron;
}

neuron_t* Neuron_Init_Placement(enum nodetype_e type, int node_id, enum nodeplace_e placement)
{
	neuron_t* neuron = malloc(sizeof(neuron_t));
	if (neuron == 0) return ((void*)1);

	neuron->active_flag = false;
	neuron->activesum = 0;
	neuron->activation = 0;
	neuron->value = 0;
	neuron->last_activation = 0;
	neuron->last_activation2 = 0;
	neuron->type = type; //NEURON or SENSOR type
	neuron->activation_count = 0; //Inactive upon creation
	neuron->node_id = node_id;
	neuron->fType = NQ_SIGMOID;
	neuron->trait = 0;
	neuron->node_label = placement;
	neuron->dupe = 0;
	neuron->analogue = 0;
	neuron->frozen = false;
	neuron->trait_id = 1;
	neuron->override = false;
	neuron->links_in = vector_init();
	neuron->links_out = vector_init();

	return neuron;
}

// Construct a node using another as a base, for genome purposes.
neuron_t* Neuron_Init_Derived(neuron_t* other, trait_t* trait)
{
	neuron_t* neuron = malloc(sizeof(neuron_t));
	if (neuron == 0) return ((void*)1);

	neuron->active_flag = false;
	neuron->activesum = 0;
	neuron->activation = 0;
	neuron->value = 0;
	neuron->last_activation = 0;
	neuron->last_activation2 = 0;
	neuron->type = other->type; //NEURON or SENSOR type
	neuron->activation_count = 0; //Inactive upon creation
	neuron->node_id = other->node_id;
	neuron->fType = NQ_SIGMOID;
	neuron->trait = trait;
	neuron->node_label = other->node_label;
	neuron->dupe = 0;
	neuron->analogue = 0;
	neuron->frozen = false;
	neuron->trait_id = (trait != 0) ? trait->id : 1;
	neuron->override = false;
	neuron->links_in = vector_init();
	neuron->links_out = vector_init();

	return neuron;
}

// Copy constructor.
neuron_t* Neuron_Init_Copy(neuron_t* other)
{
	neuron_t* neuron = malloc(sizeof(neuron_t));
	if (neuron == 0) return ((void*)1);

	neuron->active_flag			= other->active_flag;
	neuron->activesum			= other->activesum;
	neuron->activation			= other->activation;
	neuron->value				= other->value;
	neuron->last_activation		= other->last_activation;
	neuron->last_activation2	= other->last_activation2;
	neuron->type				= other->type; //NEURON or SENSOR type
	neuron->activation_count	= other->activation_count; //Inactive upon creation
	neuron->node_id				= other->node_id;
	neuron->fType				= other->fType;
	neuron->trait				= other->trait;
	neuron->node_label			= other->node_label;
	neuron->dupe				= other->dupe;
	neuron->analogue			= other->analogue;
	neuron->frozen				= other->frozen;
	neuron->trait_id			= other->trait_id;
	neuron->override			= other->override;
	neuron->links_in			= vector_init();
	neuron->links_out			= vector_init();

	return neuron;
}

double Neuron_Get_Active_Out(neuron_t* node)
{
	return (node->activation_count > 0) ? node->activation : 0.0;
}

double Neuron_Get_Active_Out_TD(neuron_t* node)
{
	return (node->activation_count > 0) ? node->last_activation : 0.0;
}

void Neuron_Delete(neuron_t* node)
{
	for (int i = 0; i < node->links_in; i++)
	{
		Link_Delete(node->links_in);
	}
	//Trait_Delete(node->trait);
	free(node);
}

cbool Neuron_Sensor_Load(neuron_t* node, double value)
{
	if (node->type == NQ_SENSOR)
	{
		node->last_activation2 = node->last_activation;
		node->last_activation = node->activation;

		node->activation_count++;
		node->activation = value;

		return true;
	}
	return false;
}

void Neuron_Add_Incoming_Recurring(neuron_t* node, neuron_t* other, double w, cbool recur)
{
	nlink_t* newlink = Link_Init(w, other, node, recur);
	vector_add(node->links_in, newlink);
	vector_add(other->links_out, newlink);
}

void Neuron_Add_Incoming(neuron_t* node, neuron_t* other, double w)
{
	nlink_t* newlink = Link_Init(w, other, node, false);
	vector_add(node->links_in, newlink);
	vector_add(other->links_out, newlink);
}


void Neuron_Flushback(neuron_t* node)
{
	if (node->type != NQ_SENSOR)
	{
		if (node->activation_count > 0)
		{
			node->activation_count = 0;
			node->activation = 0;
			node->last_activation = 0;
			node->last_activation2 = 0;
		}

		for (int i = 0; i < node->links_in->count; i++)
		{
			nlink_t* curlink = node->links_in->data[i];
			curlink->added_weight = 0;
			if (curlink->inode->activation_count > 0)
				Neuron_Flushback(curlink->inode);
		}
	}
	else
	{
		node->activation_count = 0;
		node->activation = 0;
		node->last_activation = 0;
		node->last_activation2 = 0;
	}
}

void Neuron_Flushback_Check(neuron_t* node, vector* seenlist)
{
	if (node->activation_count > 0)
	{
		printf("ALERT: Node %d has activation count %d\n", node->node_id, node->activation_count);
	}
	if (node->activation > 0)
	{
		printf("ALERT: Node %d has activation %d\n", node->node_id, node->activation);
	}
	if (node->last_activation > 0)
	{
		printf("ALERT: Node %d has last_activation %d\n", node->node_id, node->last_activation);
	}
	if (node->last_activation2 > 0)
	{
		printf("ALERT: Node %d has last_activation2 %d\n", node->node_id, node->last_activation2);
	}

	if (!(node->type == NQ_SENSOR)) {

		for (int i = 0; i < node->links_in->count; i++)
		{
			neuron_t* location = 0;
			nlink_t* curlink = node->links_in->data[i];
			for (int j = 0; j < seenlist->count; j++)
			{
				if (seenlist->data[j] == curlink->inode)
				{
					location = seenlist->data[j];
					break;
				}
			}
			if (location == 0)
			{
				vector_add(seenlist, curlink->inode);
				Neuron_Flushback_Check(curlink->inode, seenlist);
			}
			curlink->added_weight = 0;
			if (curlink->inode->activation_count > 0)
				Neuron_Flushback(curlink->inode);
		}

	}
}

void Neuron_Derive_Trait(neuron_t* node, trait_t *curtrait)
{
	if (curtrait != 0) {
		for (int i = 0; i < NQ_TRAIT_NUM_PARAMS; i++)
			node->params[i] = (curtrait->params)[i];
	}
	else {
		for (int i = 0; i < NQ_TRAIT_NUM_PARAMS; i++)
			node->params[i] = 0;
	}

	if (curtrait != 0)
		node->trait_id = curtrait->id;
	else node->trait_id = 1;
}

void Neuron_Override_Output(neuron_t* node, double new_output)
{
	node->override_value = new_output;
	node->override = true;
}

void Neuron_Activate_Override(neuron_t* node)
{
	node->activation = node->override_value;
	node->override = false;
}

int Neuron_Depth(neuron_t* node, int d, network_t* net)
{
	if (d > 100) return 10;
	if (node->type == NQ_SENSOR) return d;

	int max = d;

	for (int i = 0; i < node->links_in->count; i++)
	{
		nlink_t* curlink = node->links_in->data[i];
		int cur_depth = Neuron_Depth(curlink->inode, d + 1, net);
		if (cur_depth > max) max = cur_depth;
	}

	return max;
}