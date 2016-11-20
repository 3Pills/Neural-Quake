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

#include "gene.h"

typedef struct neuron_s
{
	double value; // The Value output by the Neuron.

	vector* olinks; // Contains: gene_t. Vector array of outgoing genes.
	vector* ilinks; // Contains: gene_t. Vector array of incoming genes.

} neuron_t; // Genetic nodes which make up a neural network

neuron_t* Neuron_Init();

// Construct a node using another as a base, for genome duplication.
neuron_t* Neuron_Init_Derived(neuron_t* other);

// Construct a node from a line of arguments loaded from a file.
// Called from Genome_Init_Load.
//neuron_t* Neuron_Init_Load(char *argline);

// Delete object helper
void Neuron_Delete(neuron_t* node);

/*
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
void Neuron_Derive_Trait(neuron_t* node, trait_t *curtrait);

// Force an output value on the node
void Neuron_Override_Output(neuron_t* node, double new_output);

// Set activation to the override value and turn off override
void Neuron_Activate_Override(neuron_t* node);

//Find the greatest depth starting from this neuron at depth d
int Neuron_Depth(neuron_t* node, int d);

// Print neuron to file. Called from Genome_FPrint.
void Neuron_FPrint(neuron_t* node, FILE *f);
*/
#endif // !__NEURON_H__