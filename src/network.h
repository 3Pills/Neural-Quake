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
#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "genome.h"

typedef struct network_s
{
	genome_t* genotype;  // Allows Network to be matched with its Genome
	vector* neurons;  // A list of all the nodes.

} network_t; // The entire pool of species.

// This constructor allows the input and output lists to be supplied. Defaults to not using adaptation.
network_t* Network_Init(vector* neurons);

//// Same as previous constructor except the adaptibility can be set true or false with adaptval.
//network_t* Network_Init_Adaptable(vector* in, vector* out, vector* all, int netID, cbool adaptVal);
//
//// This constructs a net with empty input and output lists.
//network_t* Network_Init_Empty(int netID);
//
//// Same as previous constructor except the adaptibility can be set true or false with adaptval.
//network_t* Network_Init_Empty_Adaptable(int netID, cbool adaptVal);
//
// Copy Constructor
network_t* Network_Init_Copy(network_t* n);

void Network_Delete(network_t* network);

/*
void Network_Destroy(network_t* network);  // Kills all nodes and links within
void Network_Destroy_Helper(network_t* network, neuron_t *curnode, vector* seenlist); // helper for above

// Apply a name to the specified network.
void Network_Give_Name(network_t *network, char *newname);

// Puts the network back into an inactive state
void Network_Flush(network_t* network);

// Verify flushedness for debugging
void Network_Flush_Check(network_t* network);
*/

// Activates the net such that all outputs are active
cbool Network_Activate(network_t* network);

// Takes an array of sensor values and loads it into SENSOR inputs ONLY
void Network_Load_Sensors(network_t* network, double* sensvals);
/*
// Prints the values of its outputs
void Network_Show_Activation(network_t* network);

void Network_Show_Input(network_t* network);

// Add a new input node
void Network_Add_Input(network_t* network, neuron_t* neuron);

// Add a new output node
void Network_Add_Output(network_t* network, neuron_t* neuron);

// Takes and array of output activations and OVERRIDES the outputs' actual 
// activations with these values (for adaptation)
void Network_Override_Outputs(network_t* network, double* activals);


// Counts the number of nodes in the net if not yet counted
int Network_Node_Count(network_t* network);
void Network_Node_Count_Helper(network_t* network, neuron_t *curnode, int *counter, vector* seenlist);

// Counts the number of links in the net if not yet counted
int Network_Link_Count(network_t* network);
void Network_Link_Count_Helper(network_t* network, neuron_t *curnode, int *counter, vector* seenlist);

// This checks a POTENTIAL link between a potential in_node
// and potential out_node to see if it must be recurrent 
// Use count and thresh to jump out in the case of an infinite loop 
//cbool Network_Is_Recur(network_t* network, neuron_t *potin_node, neuron_t *potout_node, int *count, int thresh);

// Some functions to help GUILE input into Networks 
/*
int Network_Input_Start(network_t* network);
int Network_Load_In(network_t* network, double d);

// If all output are not active then return true
cbool Network_Outputs_Off(network_t* network);

// Just print connections weights with carriage returns
void Network_Print_Links_To_File(network_t* network, char *filename);
*/

int Network_Max_Depth(network_t* network);

#endif // !__NETWORK_H__