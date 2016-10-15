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
#include "neural_def.h"
#include "environment.h"
#include "gene.h"

// The type of node.
enum nodetype_e {
	NEURON = 0,
	SENSOR = 1
};

// List of layers within the network. Bias is a special layer case.
enum nodeplace_e {
	HIDDEN = 0,
	INPUT = 1,
	OUTPUT = 2,
	BIAS = 3
};

// List of supported activation functions.
enum functype_e {
	SIGMOID = 0
};

typedef struct link_s link_t;
typedef struct network_s network_t;

typedef struct neuron_s
{
	int activation_count; 
	double last_activation, last_activation2;

	trait_t* trait; 
	int trait_id;

	neuron_t* dupe;
	neuron_t* analogue;

	cbool override;
	double override_value;

	vector links_out; // Contains: link_t. Vector array of outgoing Links.
	vector links_in; // Contains: link_t. Vector array of incoming Links.

	int output_value; // The Value output by the Neuron.

	cbool frozen;

	enum functype_e fType; // Activation function type. Can be changed to support things other than sigmoid.
	enum nodetype_e nType; // Either NEURON or SENSOR.

	double activesum;
	double activation;

	cbool active_flag;

	double params[NQ_NUM_TRAIT_PARAMS];

	vector rowLevels; // Contains: double. Depth from output where this node appears.

	int row;
	int ypos;
	int xpos;

	int nodeID; // Identification for file output
	
	enum nodeplace_e node_label;
} neuron_t; // Genetic nodes which make up a neural network

neuron_t Neuron_Init(enum nodetype_e nType, int nodeID);

neuron_t Neuron_Init_Placement(enum nodetype_e nType, int nodeID, enum nodeplace_e placement);

// Construct a node using another as a base, for genome purposes.
neuron_t Neuron_Init_Derived(neuron_t* node, trait_t* trait);

// Copy constructor.
neuron_t Neuron_Init_Copy(neuron_t node);

void Neuron_Flushback();

void Neuron_Flushback_Check();

void Neuron_Derive_Trait(trait_t *curTrait);

void Neuron_Override_Output(double new_output);

void Neuron_Activate_Override();

void Neuron_Lamarck();

void Neuron_Depth(int d, network_t* net);

#endif // !__NEURON_H__