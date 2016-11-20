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
#include "gene.h"
#include "genome.h"
#include "neural.h"
#include "neural_def.h"
#include <string.h>

// This constructor allows the input and output lists to be supplied. Defaults to not using adaptation.
network_t* Network_Init(vector* neurons)
{
	network_t* network = malloc(sizeof(network_t));

	network->neurons = neurons;

	return network;
}

/*
// Same as previous constructor except the adaptibility can be set true or false with adaptval.
network_t* Network_Init_Adaptable(vector* neurons, int id, cbool adaptVal)
{
	network_t* network = malloc(sizeof(network_t));

	network->neurons = neurons;

	return network;
}

// Same as previous constructor except the adaptibility can be set true or false with adaptval.
network_t* Network_Init_Empty_Adaptable(int id, cbool adaptVal)
{
	network_t* network = malloc(sizeof(network_t));

	network->inputs = vector_init();
	network->outputs = vector_init();
	network->all_nodes = vector_init();
	network->name = 0;
	network->numnodes = -1;
	network->numlinks = -1;
	network->id = id;
	network->adaptable = adaptVal;

	return network;
}
*/


// This constructs a net with empty input and output lists.
network_t* Network_Init_Empty(int id)
{
	network_t* network = malloc(sizeof(network_t));
	network->neurons = vector_init();
	return network;
}

// Copy Constructor
network_t* Network_Init_Copy(network_t* n)
{
	network_t* network = malloc(sizeof(network_t));

	for (int i = 0; i < n->neurons->count; ++i)
		vector_add(network->neurons, Neuron_Init_Derived(n->neurons->data[i]));

	return network;
}

void Network_Delete(network_t* network)
{
	for (int i = 0; i < network->neurons->count; ++i)
		Neuron_Delete(network->neurons->data[i]);

	vector_free_all(network->neurons);
	free(network);
}

/*
void Network_Destroy(network_t* network)
{
	for (int i = 0; i < network->neurons->count; ++i)
		Neuron_Delete(network->neurons->data[i]);

	vector_free_all(network->neurons);
}

void Network_Destroy_Helper(network_t* network, neuron_t *curnode, vector* seenlist)
{
	if (!((curnode->type) == NQ_SENSOR)) {
		for (int i = 0; i < curnode->ilinks->count; i++)
		{
			gene_t *gene = curnode->ilinks->data[i];
			neuron_t *location = 0;
			for (int j = 0; j < seenlist->count; j++)
			{
				if (seenlist->data[j] == gene->inode)
				{
					location = seenlist->data[j];
					break;
				}
			}
			if (location == 0)
			{
				vector_add(seenlist, gene->inode);
				Network_Destroy_Helper(network, gene->inode, seenlist);
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
*/

// Activates the net such that all outputs are active
cbool Network_Activate(network_t* network)
{
	//Keep activating until all the outputs have become active 

	// For each node, compute the sum of its incoming activation 
	for (int i = network->genotype->num_in; i < network->neurons->count; i++)
	{
		neuron_t* curnode = network->neurons->data[i];

		double sum = 0;

		// For each incoming connection, add the weight from the connection to the sum. 
		for (int j = 0; j < curnode->ilinks->count; j++)
		{
			gene_t *incoming_gene = curnode->ilinks->data[j];
			neuron_t *input_node = network->neurons->data[incoming_gene->inode];

			sum += incoming_gene->weight * input_node->value;
		}

		if (sum != 0) curnode->value = Sigmoid(curnode->value);
	}

	return true;
}

// Takes an array of sensor values and loads it into SENSOR inputs ONLY
void Network_Load_Sensors(network_t* network, double* sensvals)
{
	// -1 so we don't load values into the bias.
	for (int i = 0; i < network->genotype->num_in - 1; i++)
	{
		neuron_t *curnode = network->neurons->data[i];
		curnode->value = sensvals[i];
	}
}

/*
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
		for (int i = 0; i < curnode->ilinks->count; i++)
		{
			gene_t *gene = curnode->ilinks->data[i];
			neuron_t *location = 0;
			for (int j = 0; j < seenlist->count; j++)
			{
				if (seenlist->data[j] == gene->inode)
				{
					location = seenlist->data[j];
					break;
				}
			}
			if (location == 0)
			{
				(*counter)++;
				vector_add(seenlist, gene->inode);
				Network_Node_Count_Helper(network, gene->inode, counter, seenlist);
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

		for (int i = 0; i < curnode->ilinks->count; i++)
		{
			counter++;
			Network_Link_Count_Helper(network, ((gene_t*)curnode->ilinks->data[i])->inode, counter, seenlist);
		}
	}
}

// This checks a POTENTIAL link between a potential in_node
// and potential out_node to see if it must be recurrent 
// Use count and thresh to jump out in the case of an infinite loop 
/*
cbool Network_Is_Recur(network_t* network, neuron_t *potin_node, neuron_t *potout_node, int *count, int thresh)
{
	(*count)++;
	if (*count > thresh) return false;
	if (potin_node == potout_node) return true;

	for (int i = 0; i < potin_node->ilinks->count; i++)
	{
		gene_t* gene = potin_node->ilinks->data[i];
		if (gene->recurrent)
			if (Network_Is_Recur(network, gene->inode, potout_node, count, thresh))
				return true;
	}

	return false;
}
*/

/*
int Network_Input_Start(network_t* network)
{
	network-> = network->inputs->data[0];
	return 1;
}

int Network_Load_In(network_t* network, double d)
{
	return Neuron_Sensor_Load(network, d) ? 1 : 0;
}

// If all output are not active then return true
cbool Network_Outputs_Off(network_t* network)
{
	return false;
}
// Just print connections weights with carriage returns
void Network_Print_Links_To_File(network_t* network, char *filename)
{
}
*/


int Network_Neuron_Depth(vector* nodes, int node_index, int d)
{
	if (d > 100) return 10;

	int max = d;
	neuron_t *node = nodes->data[node_index];

	for (int i = 0; i < node->ilinks->count; i++)
	{
		gene_t* curlink = node->ilinks->data[i];
		int cur_depth = Network_Neuron_Depth(nodes, curlink->inode, d + 1);
		if (cur_depth > max) max = cur_depth;
	}

	return max;
}

int Network_Max_Depth(network_t* network)
{
	int cur_depth = 0, max = 0;

	for (int i = network->genotype->num_in; i < network->genotype->num_in + network->genotype->num_out; i++)
	{
		cur_depth = Network_Neuron_Depth(network->neurons, i, 0);
		if (cur_depth > max) max = cur_depth;
	}

	return max;
}

