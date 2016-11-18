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
#ifndef __NEURON_H__
#define __NEURON_H__
#include "environment.h"
#include "gene.h"
#include "vector.h"

// The type of node.
enum nodetype_e {
	NQ_NEURON = 0,
	NQ_SENSOR = 1
};


// List of layers within the network. Bias is a special layer case.
enum nodeplace_e {
	NQ_HIDDEN = 0,
	NQ_INPUT = 1,
	NQ_OUTPUT = 2,
	NQ_BIAS = 3
};

// List of supported activation functions.
enum functype_e {
	NQ_SIGMOID = 0
};

typedef struct nlink_s nlink_t;
typedef struct network_s network_t;

typedef struct neuron_s
{
	int activation_count; // keeps track of which activation the node is currently in
	double last_activation; // Holds the previous step's activation for recurrency
	double last_activation2; // Holds the activation BEFORE the prevous step's

	//trait_t* trait; // Points to a trait of parameters
	//int trait_id; // identify the trait derived by this node

	neuron_t* dupe; // Used for Genome duplication
	neuron_t* analogue; // Used for Gene decoding

	cbool override; // The NNode cannot compute its own output- something is overriding it
	double override_value; // Contains the activation value that will override this node's activation

	vector* olinks; // Contains: nlink_t. Vector array of outgoing Links.
	vector* ilinks; // Contains: nlink_t. Vector array of incoming Links.

	int value; // The Value output by the Neuron.

	cbool frozen;

	enum nodetype_e type; // Neuron type. Either NEURON or SENSOR.
	enum functype_e fType; // Activation function type. Can be changed to support things other than sigmoid.

	double activesum; // The incoming activity before being processed 
	double activation; // The total activation entering the NNode 
	cbool active_flag; // To make sure outputs are active

	//double params[NQ_TRAIT_NUM_PARAMS];

	int node_id; // Identification for file output
	enum nodeplace_e node_label;
} neuron_t; // Genetic nodes which make up a neural network

neuron_t* Neuron_Init(enum nodetype_e type, int node_id);

neuron_t* Neuron_Init_Placement(enum nodetype_e type, int node_id, enum nodeplace_e placement);

// Construct a node using another as a base, for genome purposes.
neuron_t* Neuron_Init_Derived(neuron_t* other);

// Construct a node from a line of arguments loaded from a file.
// Called from Genome_Init_Load.
neuron_t* Neuron_Init_Load(char *argline);

// Copy constructor.
neuron_t* Neuron_Init_Copy(neuron_t* other);

// Delete object helper
void Neuron_Delete(neuron_t* node);

// If the node is a SENSOR, returns true and loads the value
cbool Neuron_Sensor_Load(neuron_t* node, double value);

// Just return activation for step
double Neuron_Get_Active_Out(neuron_t* node);

// Return activation from PREVIOUS time step
double Neuron_Get_Active_Out_TD(neuron_t* node);

// Adds a Link to a new NNode in the incoming List
void Neuron_Add_Incoming_Recurring(neuron_t* node, neuron_t* other, double w, cbool recur);

// Adds a NONRECURRENT Link to a new NNode in the incoming List
void Neuron_Add_Incoming(neuron_t* node, neuron_t* other, double w);

// Recursively deactivate backwards through the network
void Neuron_Flushback(neuron_t* node);

// Verify flushing for debugging
void Neuron_Flushback_Check(neuron_t* node, vector* seenlist);

// Have Neuron gain its properties from the trait
//void Neuron_Derive_Trait(neuron_t* node, trait_t *curtrait);

// Force an output value on the node
void Neuron_Override_Output(neuron_t* node, double new_output);

// Set activation to the override value and turn off override
void Neuron_Activate_Override(neuron_t* node);

//Find the greatest depth starting from this neuron at depth d
int Neuron_Depth(neuron_t* node, int d, network_t* net);

// Print neuron to file. Called from Genome_FPrint.
void Neuron_FPrint(neuron_t* node, FILE *f);

#endif // !__NEURON_H__