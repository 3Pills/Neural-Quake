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
#include "network.h"
#include "neural.h"
#include "neural_def.h"
#include <string.h>

// This constructor allows the input and output lists to be supplied. Defaults to not using adaptation.
network_t* Network_Init(vector* in, vector* out, vector* all, int netID)
{
	network_t* network = malloc(sizeof(network_t));

	network->inputs = in;
	network->outputs = out;
	network->all_nodes = all;
	network->name = 0;
	network->numnodes = -1;
	network->numlinks = -1;
	network->net_id = netID;
	network->adaptable = false;

	return network;
}

// Same as previous constructor except the adaptibility can be set true or false with adaptval.
network_t* Network_Init_Adaptable(vector* in, vector* out, vector* all, int netID, cbool adaptVal)
{
	network_t* network = malloc(sizeof(network_t));

	network->inputs = in;
	network->outputs = out;
	network->all_nodes = all;
	network->name = 0;
	network->numnodes = -1;
	network->numlinks = -1;
	network->net_id = netID;
	network->adaptable = adaptVal;

	return network;
}


// This constructs a net with empty input and output lists.
network_t* Network_Init_Empty(int netID)
{
	network_t* network = malloc(sizeof(network_t));

	network->inputs = vector_init();
	network->outputs = vector_init();
	network->all_nodes = vector_init();
	network->name = 0;
	network->numnodes = -1;
	network->numlinks = -1;
	network->net_id = netID;
	network->adaptable = false;

	return network;
}


// Same as previous constructor except the adaptibility can be set true or false with adaptval.
network_t* Network_Init_Empty_Adaptable(int netID, cbool adaptVal)
{
	network_t* network = malloc(sizeof(network_t));

	network->inputs = vector_init();
	network->outputs = vector_init();
	network->all_nodes = vector_init();
	network->name = 0;
	network->numnodes = -1;
	network->numlinks = -1;
	network->net_id = netID;
	network->adaptable = adaptVal;

	return network;
}


// Copy Constructor
network_t* Network_Init_Copy(network_t* n)
{
	network_t* network = malloc(sizeof(network_t));

	// Copy all the inputs
	for (int i = 0; i < n->inputs->count; ++i) {
		neuron_t* node = Neuron_Init_Copy(n->inputs->data[i]);

		vector_add(network->inputs, node);
		vector_add(network->all_nodes, node);
	}

	// Copy all the outputs
	for (int i = 0; i < n->outputs->count; ++i) {
		neuron_t* node = Neuron_Init_Copy(n->outputs->data[i]);

		vector_add(network->outputs, node);
		vector_add(network->all_nodes, node);
	}

	network->name = (n->name != 0) ? _strdup(n->name) : 0;

	network->numnodes = n->numnodes;
	network->numlinks = n->numlinks;
	network->net_id = n->net_id;
	network->adaptable = n->adaptable;

	return network;
}

void Network_Delete(network_t* network)
{
	if (network->name != 0) free(network->name);
	Network_Destroy(network);
	free(network);
}

void Network_Destroy(network_t* network)
{
	for (int i = 0; i < network->all_nodes->count; ++i)
		Neuron_Delete(network->all_nodes->data[i]);

	vector_free_all(network->inputs);
	vector_free_all(network->outputs);
	vector_free_all(network->all_nodes);
}

void Network_Destroy_Helper(network_t* network, neuron_t *curnode, vector* seenlist)
{
	vector* innodes = curnode->links_in;

	if (!((curnode->type) == NQ_SENSOR)) {
		for (int i = 0; i < innodes->count; i++)
		{
			nlink_t *curlink = innodes->data[i];
			neuron_t *location = 0;
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
				Network_Destroy_Helper(network, curlink->inode, seenlist);
			}
		}
	}
}

void Network_Give_Name(network_t *network, char *newname)
{
	if (network->name == 0)
		network->name = malloc(sizeof(char) * (strlen(newname) + 1));
	else 
		network->name = realloc(network->name, sizeof(char) * (strlen(newname) + 1));

	strcpy(network->name, newname);
}

// Puts the network back into an inactive state
void Network_Flush(network_t* network)
{
	for (int i = 0; i < network->outputs->count; i++)
		Neuron_Flushback(network->outputs->data[i]);
}

// Verify flushedness for debugging
void Network_Flush_Check(network_t* network)
{
	vector seenlist = { NULL, 0, 0 };

	for (int i = 0; i < network->outputs->count; i++)
	{
		neuron_t *curnode = network->outputs->data[i], *location = 0;
		for (int j = 0; j < seenlist.count; j++)
		{
			if (seenlist.data[j] == curnode)
			{
				location = seenlist.data[j];
				break;
			}
		}
		if (location == 0)
		{
			vector_add(&seenlist, curnode);
			Neuron_Flushback_Check(curnode, &seenlist);
		}
	}
}

// Activates the net such that all outputs are active
cbool Network_Activate(network_t* network)
{
	cbool onetime = false;
	int abortcount = 0;

	//Keep activating until all the outputs have become active 
	//(This only happens on the first activation, because after that they
	// are always active)
	while (Network_Outputs_Off(network) || !onetime)
	{
		++abortcount;
		if (abortcount == 20) return false;

		// For each node, compute the sum of its incoming activation 
		for (int i = 0; i < network->all_nodes->count; i++)
		{
			neuron_t* curnode = network->all_nodes->data[i];

			if (curnode->type != NQ_SENSOR)
			{
				curnode->activesum = 0;
				curnode->active_flag = false;

				// For each incoming connection, add the activity from the connection to the activesum 
				for (int j = 0; j < curnode->links_in->count; j++)
				{
					double add_amount = 0.0;
					nlink_t* curlink = curnode->links_in->data[j];

					//Handle possible time delays
					if (!curlink->time_delay)
					{
						add_amount = curlink->weight * Neuron_Get_Active_Out(curlink->inode);
						if (curlink->inode->active_flag || curlink->inode->type == NQ_SENSOR) curnode->active_flag = true;
					}
					else
					{
						add_amount = curlink->weight * Neuron_Get_Active_Out_TD(curlink->inode);
					}
					curnode->activesum += add_amount;
				}
			}
		}

		// Now activate all the non-sensor nodes off their incoming activation 
		for (int i = 0; i < network->all_nodes->count; i++)
		{
			neuron_t* curnode = network->all_nodes->data[i];
			if (curnode->type != NQ_SENSOR && curnode->active_flag)
			{
				curnode->last_activation2 = curnode->last_activation;
				curnode->last_activation = curnode->activation;

				if (curnode->override)
					Neuron_Activate_Override(curnode);
				else if (curnode->fType == NQ_SIGMOID)
					curnode->activation = Sigmoid(curnode->activesum);

				curnode->activation_count++;
			}
		}
		onetime = true;
	}

	if (network->adaptable)
	{
		for (int i = 0; i < network->all_nodes->count; i++)
		{
			neuron_t* curnode = network->all_nodes->data[i];

			if (curnode->type != NQ_SENSOR)
			{
				for (int j = 0; j < curnode->links_in->count; j++)
				{
					nlink_t* curlink = curnode->links_in->data[j];

					//if (curlink->trait_id == 2 ||
					//	curlink->trait_id == 3 ||
					//	curlink->trait_id == 4)
					//{
					//	curlink->weight = Hebbian(curlink->weight, network->maxweight,
					//		(curlink->recurrent) ? Neuron_Get_Active_Out_TD(curlink->inode) : Neuron_Get_Active_Out(curlink->inode),
					//		Neuron_Get_Active_Out(curlink->onode), 0, 0, 0);
					//}
				}
			}
		}
	}
	return true;
}

// Prints the values of its outputs
void Network_Show_Activation(network_t* network)
{

}

void Network_Show_Input(network_t* network)
{

}

// Add a new input node
void Network_Add_Input(network_t* network, neuron_t* inode)
{
	vector_add(network->inputs, inode);
}

// Add a new output node
void Network_Add_Output(network_t* network, neuron_t* onode)
{
	vector_add(network->inputs, onode);
}

// Takes an array of sensor values and loads it into SENSOR inputs ONLY
void Network_Load_Sensors(network_t* network, double* sensvals)
{
	for (int i = 0; i < network->inputs->count; i++) 
	{
		neuron_t *curnode = network->inputs->data[i];
		//only load values into SENSORS (not BIASes)
		if (curnode->type == NQ_SENSOR)
			Neuron_Sensor_Load(curnode, sensvals[i]);
	}
}

void Network_Load_Sensors_Vector(network_t* network, vector* sensvals)
{
	for (int i = 0; i < network->inputs->count && i < sensvals->count; i++)
	{
		neuron_t *curnode = network->inputs->data[i];
		float *sensval = sensvals->data[i];
		if (curnode->type == NQ_SENSOR)
			Neuron_Sensor_Load(curnode, *sensval);
	}
}

// Takes and array of output activations and OVERRIDES the outputs' actual 
// activations with these values (for adaptation)
void Network_Override_Outputs(network_t* network, double* activals)
{
	for (int i = 0; i < network->outputs->count; i++)
	{
		Neuron_Override_Output(network->outputs->data[i], activals[i]);
	}
}

// Counts the number of nodes in the net if not yet counted
int Network_Node_Count(network_t* network)
{
	vector seenlist = { NULL, 0, 0 };
	int counter = 0;

	for (int i = 0; i < network->outputs->count; i++)
	{
		neuron_t *curnode = network->outputs->data[i], *location = 0;
		for (int j = 0; j < seenlist.count; j++)
		{
			if (seenlist.data[j] == curnode)
			{
				location = seenlist.data[j];
				break;
			}
		}
		if (location == 0)
		{
			counter++; 
			vector_add(&seenlist, curnode);
			Network_Node_Count_Helper(network, curnode, &counter, &seenlist);
		}
	}
	network->numnodes = counter;
	return counter;
}

void Network_Node_Count_Helper(network_t* network, neuron_t *curnode, int *counter, vector* seenlist)
{
	if (curnode->type != NQ_SENSOR)
	{
		for (int i = 0; i < curnode->links_in->count; i++)
		{
			nlink_t *curlink = curnode->links_in->data[i];
			neuron_t *location = 0;
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
				(*counter)++;
				vector_add(seenlist, curlink->inode);
				Network_Node_Count_Helper(network, curlink->inode, counter, seenlist);
			}
		}
	}
}

// Counts the number of links in the net if not yet counted
int Network_Link_Count(network_t* network)
{
	vector seenlist = { NULL, 0, 0 };
	int counter = 0;

	for (int i = 0; i < network->outputs->count; i++)
		Network_Link_Count_Helper(network, network->outputs->data[i], &counter, &seenlist);

	network->numlinks = counter;
	return counter;
}


void Network_Link_Count_Helper(network_t* network, neuron_t *curnode, int *counter, vector* seenlist)
{
	neuron_t *location = 0;
	for (int i = 0; i < seenlist->count; i++)
	{
		if (seenlist->data[i] == curnode)
		{
			location = seenlist->data[i];
			break;
		}
	}
	if (location != 0 && curnode->type != NQ_SENSOR) 
	{
		vector_add(seenlist, curnode);

		for (int i = 0; i < curnode->links_in->count; i++)
		{
			counter++;
			Network_Link_Count_Helper(network, ((nlink_t*)curnode->links_in->data[i])->inode, counter, seenlist);
		}
	}
}

// This checks a POTENTIAL link between a potential in_node
// and potential out_node to see if it must be recurrent 
// Use count and thresh to jump out in the case of an infinite loop 
cbool Network_Is_Recur(network_t* network, neuron_t *potin_node, neuron_t *potout_node, int *count, int thresh)
{
	(*count)++;
	if (*count > thresh) return false;
	if (potin_node == potout_node) return true;

	for (int i = 0; i < potin_node->links_in->count; i++)
	{
		nlink_t* curlink = potin_node->links_in->data[i];
		if (curlink->recurrent)
			if (Network_Is_Recur(network, curlink->inode, potout_node, count, thresh))
				return true;
	}

	return false;
}

int Network_Input_Start(network_t* network)
{
	network->input_ptr = network->inputs->data[0];
	return 1;
}

int Network_Load_In(network_t* network, double d)
{
	Neuron_Sensor_Load(network->input_ptr, d);

}

// If all output are not active then return true
cbool Network_Outputs_Off(network_t* network)
{
	for (int i = 0; i < network->outputs->count; i++)
		if (((neuron_t*)network->outputs->data[i])->activation_count == 0) 
			return true;
	return false;
}

// Just print connections weights with carriage returns
void Network_Print_Links_To_File(network_t* network, char *filename)
{
}

int Network_Max_Depth(network_t* network)
{
	int cur_depth = 0, max = 0;

	for (int i = 0; i < network->outputs->count; i++)
	{
		cur_depth = Neuron_Depth(network->outputs->data[i], 0, network);
		if (cur_depth > max) max = cur_depth;
	}

	return max;
}
