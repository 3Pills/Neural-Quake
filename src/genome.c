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
#include "genome.h"
#include "network.h"
#include "neural.h"
#include "neural_def.h"
#include <string.h>

//Constructor which takes full genome specs and puts them into the new one
genome_t* Genome_Init(int id, vector* neurons, vector* genes)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return NULL;

	genome->id = id;
	//genome->traits = traits;
	genome->neurons = neurons;
	genome->genes = genes;
	genome->fitness = -1;
	genome->num_in = -1;
	genome->num_out = -1;

	memset(genome->final_pos, 0, sizeof(float) * 3);

	return genome;
}

genome_t* Genome_Init_Empty()
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return NULL;

	genome->id = -1;
	genome->neurons = vector_init();
	genome->genes = vector_init();
	genome->fitness = -1;
	memset(genome->final_pos, 0, sizeof(float) * 3);

	return genome;
}

//Constructor which takes in links (not genes) and creates a Genome
genome_t* Genome_Init_Links(int id, vector* neurons, vector* links)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return NULL;

	genome->id = id;
	genome->neurons = neurons;
	genome->genes = vector_init();
	genome->fitness = -1;
	memset(genome->final_pos, 0, sizeof(float) * 3);

	//We go through the links and turn them into original genes
	for (int i = 0; i < links->count; i++) {
		gene_t* curlink = links->data[i];
		//vector_add(genome->genes, Gene_Init_Trait(curlink->trait, curlink->weight, curlink->inode, curlink->onode, curlink->recurrent, 1.0, 0.0));
		vector_add(genome->genes, Gene_Init(curlink->weight, curlink->inode, curlink->onode, 1.0, 0.0));
	}

	return genome;
}

genome_t *Genome_Duplicate(genome_t *genome, int new_id)
{
	//Collections for the new Genome
	vector *nodes = vector_init(), *genes = vector_init();

	//Duplicate NNodes
	for (int i = 0; i < genome->neurons->count; i++)
		vector_add(nodes, Neuron_Init_Derived(genome->neurons->data[i]));

	//Duplicate Genes
	for (int i = 0; i < genome->genes->count; i++)
	{
		gene_t *curgene = genome->genes->data[i];
		vector_add(genes, Gene_Init_Dupe(curgene, curgene->inode, curgene->onode));
	}

	//Finally, initialize basic values and return the new genome
	genome_t *newgenome = Genome_Init(new_id, nodes, genes);

	newgenome->num_in = genome->num_in;
	newgenome->num_out = genome->num_out;
	newgenome->fitness = genome->fitness;

	VectorCopy(genome->final_pos, newgenome->final_pos);

	return newgenome;
}

genome_t* Genome_Init_Load(int id, FILE *f)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return NULL;

	genome->id = id;
	genome->neurons = vector_init();
	genome->genes = vector_init();
	genome->fitness = atoi(strtok(NULL, " \n"));

	vec3_t final_pos = { atoi(strtok(NULL, " \n")), atoi(strtok(NULL, " \n")), atoi(strtok(NULL, " \n")) };
	genome->final_pos[0] = final_pos[0];
	genome->final_pos[1] = final_pos[1];
	genome->final_pos[2] = final_pos[2];

	char* curword;
	char curline[1024]; //max line size of 1024 characters

	cbool done = false;
	while (fgets(curline, sizeof(curline), f) && !done)
	{
		char lineCopy[1024];
		strcpy(lineCopy, curline);

		curword = strtok(lineCopy, " \n");
		
		if (curword != NULL)
		{
			// Genome end reached. 
			if (strcmp(curword, "gnome_e") == 0)
			{
				// Load the ID.
				int idcheck;
				sscanf(strtok(NULL, " \n"), "%d", &idcheck);

				if (idcheck != genome->id) Con_Printf("ERROR: id mismatch in genome [%d : %d]\n", genome->id, idcheck);

				// We've read the genome completely.
				done = true;
			}

			//Print any comments to the console.
			else if (strcmp(curword, "/*") == 0)
			{
				char metadata[128];
				cbool md = false;
				strcpy(metadata, "");
				curword = strtok(NULL, " ");
				while (curword != NULL && strcmp(curword, "*/") != 0)
				{
					if (md) strncat(metadata, " ", 128 - strlen(metadata));
					md = true;

					strncat(metadata, curword, 128 - strlen(metadata));
					curword = strtok(NULL, " \n");
				}
				Con_Printf(metadata);
			}
			else if (strcmp(curword, "nodes") == 0)
			{
				//Read in node totals.
				sscanf(strtok(NULL, " \n"), "%d", &genome->num_in);
				sscanf(strtok(NULL, " \n"), "%d", &genome->num_out);

				int total;
				sscanf(strtok(NULL, " \n"), "%d", &total);

				for (int i = 0; i < total; i++)
					vector_add(genome->neurons, Neuron_Init());
			}

			//Read in a gene
			else if (strcmp(curword, "g") == 0) {

				char argline[1024];
				strcpy(argline, curline);

				gene_t *gene = Gene_Init_Load(argline, genome->neurons);
				if (gene == 0) return genome;

				//Add the gene to the genome
				vector_add(genome->genes, gene);
			}
		}
	}

	return genome;
}

// This special constructor creates a Genome with random connectivity. 
// The last input is a bias.
// Linkprob is the probability of a link.
genome_t* Genome_Init_Structure(int new_id, int num_in, int num_out, int num_hidden, int hidden_max, cbool recurrent, double linkprob)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return ((void*)1);

	genome->id = new_id;
	genome->neurons = vector_init();
	genome->genes = vector_init();

	int totalnodes = num_in + num_out + hidden_max;
	
	int matrixdim = totalnodes*totalnodes;
	int count;

	int maxnode; //No nodes above this number for this genome

	cbool *cm = malloc(sizeof(cbool) * matrixdim);  //Dimension the connection matrix
	cbool *cmp; //Connection matrix pointer
	maxnode = num_in + num_hidden;

	int first_output = totalnodes - num_out + 1;

	//For creating the new genes
	//trait_t *newtrait;
	int inode;
	int onode;

	//cout<<"Assigned id "<<genome_id<<endl;

	//Step through the connection matrix, randomly assigning bits
	cmp = cm;
	for (count = 0; count < matrixdim; count++) {
		if (Random_Float() < linkprob)
			*cmp = true;
		else *cmp = false;
		cmp++;
	}

	//Create a dummy trait (this is for future expansion of the system)
	//newtrait = Trait_Init_Values(10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	//vector_add(genome->traits, newtrait);

	//Build the input nodes
	for (int i = 0; i < num_in; i++)
	{
		neuron_t *newnode = Neuron_Init();
		//newnode->trait = newtrait;

		//Add the node to the list of nodes
		vector_add(genome->neurons, newnode);
	}

	//Build the hidden nodes
	for (int i = num_in; i < num_in + num_hidden; i++)
	{
		neuron_t *newnode = Neuron_Init();
		//newnode->trait = newtrait;
		vector_add(genome->neurons, newnode);
	}

	//Build the output nodes
	for (int i = first_output; i < totalnodes; i++)
	{
		neuron_t *newnode = Neuron_Init();
		vector_add(genome->neurons, newnode);
	}

	//Step through the connection matrix, creating connection genes
	cmp = cm;
	count = 0;
	for (int col = 0; col <= totalnodes; col++)
	{
		for (int row = 0; row <= totalnodes; row++)
		{
			//Only try to create a link if it is in the matrix and not leading into a sensor.
			if ((*cmp == true) && (col > num_in) &&
				((col <= maxnode) || (col >= first_output)) &&
				((row <= maxnode) || (row >= first_output))) {
				//If it isn't recurrent, create the connection no matter what
				if (col > row) {

					//Find the input node.
					int node_iter = 0;
					for (; node_iter < totalnodes; node_iter++)
					{
						if (node_iter == row) {
							inode = node_iter;
							break;
						}
					}
					//Find the output node. No need to reset node_iter; output node will always be after the input.
					for (; node_iter < totalnodes; node_iter++)
					{
						if (node_iter == col) {
							onode = node_iter;
							break;
						}
					}

					//Create the gene
					double new_weight = (Random_Float() - 0.5) * 2;

					//Add the gene to the genome
					//vector_add(genome->genes, Gene_Init_Trait(newtrait, new_weight, in_node, out_node, false, count, new_weight));
					vector_add(genome->genes, Gene_Init(new_weight, inode, onode, count, new_weight));
				}
			}
			count++; //increment gene counter	    
			cmp++;
		}
	}
	free(cm);
	return genome;
}


//Special constructor that creates a Genome of 3 possible types:
//0 - Fully linked, no hidden nodes
//1 - Fully linked, one hidden node splitting each link
//2 - Fully connected with a hidden layer, recurrent 
//num_hidden is only used in type 2
genome_t* Genome_Init_Auto(int num_in, int num_out, int num_hidden, int type)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return ((void*)1);

	genome->id = 0;
	genome->num_in = ++num_in; // Add the bias.
	genome->num_out = num_out;

	if (type == 0)
		num_hidden = 0;
	else if (type == 1)
		num_hidden = genome->num_in*genome->num_out;

	genome->neurons = vector_init();
	genome->genes = vector_init();

	//Build the nodes
	for (int i = 0; i < num_in + num_out + num_hidden; i++)
		vector_add(genome->neurons, Neuron_Init());

	//Create the links depending on the type
	if (type == 0) 
	{
		int count = 1;
		for (int i = genome->num_in; i < genome->num_in + genome->num_out; i++)
		{
			for (int j = 0; j < genome->num_in; j++)
			{
				vector_add(genome->genes, Gene_Init(0, j, i, count, 0));
				count++;
			}
		}

	} //end type 0
	//A split link from each input to each output
	else if (type == 1) {
		int count = 1;

		//Initialize links between layers.
		for (int i = genome->num_in; i < genome->num_in + genome->num_out; i++)
		{
			for (int j = 0; j < genome->num_in; j++)
			{
				for (int k = genome->num_in + genome->num_out; k < genome->neurons->count; k++)
				{
					// Connect Input to hidden.
					vector_add(genome->genes, Gene_Init(0, j, k, count, 0));
					count++;
					// Connect hidden to output.
					vector_add(genome->genes, Gene_Init(0, k, i, count, 0));
					count++;
				}
			}
		}
	}//end type 1
	//Fully connected 
	/*
	else if (type == 2) {
		count = 1; //Start gene counter at 1


		//Connect all inputs to all hidden nodes
		for (int i = 0; i < hidden->count; i++)
		{
			for (int j = 0; j < inputs->count; j++)
			{
				// Connect each input to each hidden.
				vector_add(genome->genes, Gene_Init(0, inputs->data[j], hidden->data[i], count, 0));
				count++;
			}
		}

		//Connect all hidden units to all outputs
		for (int i = 0; i < outputs->count; i++)
		{
			for (int j = 0; j < hidden->count; j++)
			{
				vector_add(genome->genes, Gene_Init(0, hidden->data[j], outputs->data[i], count, 0));
				count++;
			}
		}

		//Connect the bias to all outputs
		for (int i = 0; i < outputs->count; i++)
		{
			vector_add(genome->genes, Gene_Init(0, bias, outputs->data[i], count, 0));
			count++;
		}

		//Recurrently connect the hidden nodes
		for (int i = 0; i < hidden->count; i++)
		{
			for (int j = 0; j < hidden->count; j++)
			{
				vector_add(genome->genes, Gene_Init(0, hidden->data[j], hidden->data[i], count, 0));
				count++;
			}
		}

	}//end type 2
	*/

	return genome;
}

/*
//Special constructor that creates a Genome of 3 possible types:
//0 - Fully linked, no hidden nodes
//1 - Fully linked, one hidden node splitting each link
//2 - Fully connected with a hidden layer, recurrent 
//num_hidden is only used in type 2
genome_t* Genome_Init_Auto(int num_in, int num_out, int num_hidden, int type)
{
	genome_t* genome = malloc(sizeof(genome_t));
	if (genome == 0) return ((void*)1);

	genome->id = 0;
	genome->neurons = vector_init();
	genome->genes = vector_init();

	//Temporary lists of nodes
	vector *inputs = vector_init(), *outputs = vector_init(), *hidden = vector_init();
	neuron_t *bias; //Remember the bias

	//For creating the new genes
	neuron_t *newnode;
	//trait_t *newtrait;

	int count;
	
	//Create a dummy trait (this is for future expansion of the system)
	//newtrait = Trait_Init_Values(1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	//vector_add(genome->traits, newtrait);

	//Adjust hidden number
	if (type == 0)
		num_hidden = 0;
	else if (type == 1)
		num_hidden = num_in*num_out;

	//Create the inputs and outputs

	//Build the input nodes
	for (int i = 0; i < num_in; i++) {
		newnode = Neuron_Init_Placement(NQ_SENSOR, i, NQ_INPUT);

		//Add the node to the list of nodes
		vector_add(genome->neurons, newnode);
		vector_add(inputs, newnode);
	}

	//Build the output nodes
	for (int i = num_in; i < num_in + num_out; i++) {
		newnode = Neuron_Init_Placement(NQ_NEURON, i, NQ_OUTPUT);
		//newnode->nodetrait=newtrait;
		//Add the node to the list of nodes
		vector_add(genome->neurons, newnode);
		vector_add(outputs, newnode);
	}

	//Add input bias
	newnode = Neuron_Init_Placement(NQ_SENSOR, num_in + num_out, NQ_BIAS);
	bias = newnode;

	vector_add(genome->neurons, newnode);
	vector_add(outputs, newnode);

	//Build the hidden nodes
	for (int i = num_in + num_out + 1; i <= num_in + num_out + num_hidden; i++) {
		newnode = Neuron_Init(NQ_NEURON, i);
		//newnode->nodetrait=newtrait;
		//Add the node to the list of nodes
		vector_add(genome->neurons, newnode);
		vector_add(hidden, newnode);
	}

	//Create the links depending on the type
	if (type == 0) {
		//Just connect inputs straight to outputs

		count = 1;

		//Initialize inputs and outputs.
		for (int i = 0; i < outputs->count; i++)
		{
			for (int j = 0; j < inputs->count; j++)
			{
				//Connect each input to each output
				vector_add(genome->genes, Gene_Init(0, inputs->data[j, outputs->data[i], count, 0));

				count++;
			}
		}
	} //end type 0
	//A split link from each input to each output
	else if (type == 1) {
		count = 1;

		//Initialize links between layers.
		for (int i = 0; i < outputs->count; i++)
		{
			for (int j = 0; j < inputs->count; j++)
			{
				for (int k = 0; k < hidden->count; k++)
				{
					// Connect Input to hidden.
					vector_add(genome->genes, Gene_Init(0, inputs->data[j], hidden->data[k], count, 0));
					count++;

					// Connect hidden to output.
					vector_add(genome->genes, Gene_Init(0, hidden->data[k], outputs->data[i], count, 0));
					count++;
				}
			}
		}

	}//end type 1
	//Fully connected 
	else if (type == 2) {
		count = 1; //Start gene counter at 1


		//Connect all inputs to all hidden nodes
		for (int i = 0; i < hidden->count; i++)
		{
			for (int j = 0; j < inputs->count; j++)
			{
				// Connect each input to each hidden.
				vector_add(genome->genes, Gene_Init(0, inputs->data[j], hidden->data[i], count, 0));
				count++;
			}
		}
		
		//Connect all hidden units to all outputs
		for (int i = 0; i < outputs->count; i++)
		{
			for (int j = 0; j < hidden->count; j++)
			{
				vector_add(genome->genes, Gene_Init(0, hidden->data[j], outputs->data[i], count, 0));
				count++;
			}
		}

		//Connect the bias to all outputs
		for (int i = 0; i < outputs->count; i++)
		{
			vector_add(genome->genes, Gene_Init(0, bias, outputs->data[i], count, 0));
			count++;
		}

		//Recurrently connect the hidden nodes
		for (int i = 0; i < hidden->count; i++)
		{
			for (int j = 0; j < hidden->count; j++)
			{
				vector_add(genome->genes, Gene_Init(0, hidden->data[j], hidden->data[i], count, 0));
				count++;
			}
		}

	}//end type 2

	return genome;
}*/

// Loads a new Genome from a file (doesn't require knowledge of Genome's id)
static genome_t *new_Genome_load(char *filename);

//Destructor kills off all lists (including the trait vector)
void Genome_Delete(genome_t *genome)
{
	vector_free_all(genome->neurons);
	vector_free_all(genome->genes);
	Network_Delete(genome->phenotype);

	free(genome);
}

//Generate a network phenotype from this Genome with specified id
network_t *Genome_Genesis(genome_t *genome, int id)
{
	vector* neurons = vector_init();

	for (int i = 0; i < genome->neurons->count; ++i)
		vector_add(neurons, Neuron_Init_Derived(genome->neurons->data[i]));

	Quicksort(0, genome->genes->count-1, genome->genes->data, Genome_Quicksort_Genes);

	//Create any non-existant nodes by iterating through the genes.
	for (int i = 0; i < genome->genes->count; ++i) 
	{
		gene_t* curgene = genome->genes->data[i];
		//Only create the link if the gene is enabled
		if (curgene->enabled == true) 
		{
			if (curgene->inode >= genome->genes->count || genome->genes->data[curgene->inode] == NULL)
				vector_insert(neurons, curgene->inode, Neuron_Init(0, 0));

			neuron_t *node = neurons->data[curgene->inode];
			vector_add(node->olinks, curgene);

			if (curgene->onode >= genome->genes->count || genome->genes->data[curgene->onode] == NULL)
				vector_insert(neurons, curgene->onode, Neuron_Init(0, 0));

			node = neurons->data[curgene->onode];
			vector_add(node->ilinks, curgene);
		}
	}

	//Create the new network
	network_t *network = Network_Init(neurons);

	//Attach genotype and phenotype together
	network->genotype = genome;
	genome->phenotype = network;

	return network;
}

cbool Genome_Verify(genome_t *genome)
{
	// Check all gene nodes.
	for (int i = 0; i < genome->genes->count; i++) 
	{
		gene_t *curgene = genome->genes->data[i];

		int inode = curgene->inode;
		int onode = curgene->onode;
		
		int cur_inode = 0, cur_onode = 0;
		for (int j = 0; j < genome->neurons->count; j++)
		{
			if (cur_inode != inode) cur_inode = j;
			if (cur_onode != onode) cur_onode = j;
			if (cur_onode == onode && cur_inode == inode) break;
		}

		if (cur_inode == genome->neurons->count-1 || 
			cur_onode == genome->neurons->count-1) 
			return false;
	}

	return true;
}

int Genome_Get_Last_Node_ID(genome_t *genome)
{
	return genome->neurons->count - 1;
}

double Genome_Get_Last_Gene_Innovnum(genome_t *genome)
{
	return ((gene_t*)genome->genes->data[genome->genes->count - 1])->innovation_num + 1;
}

void Genome_Print_Genome(genome_t *genome)
{

}

void Genome_Mutate_Random_Trait(genome_t *genome)
{
	//Trait_Mutate((trait_t*)genome->traits->data[Random_Int(0, genome->traits->count)]);
}

void Genome_Mutate_Link_Trait(genome_t *genome, int times)
{
	if (genome->genes->count == 0) return;

	for (int i = 0; i < times; i++) {
		//Choose a random gene.
		gene_t* gene = genome->genes->data[Random_Int(0, genome->genes->count - 1)];

		//Do not alter frozen genes
		//if (!gene->frozen)
		//	gene->link->trait = genome->traits->data[Random_Int(0, genome->traits->count - 1)];
	}
}

void Genome_Mutate_Node_Trait(genome_t *genome, int times)
{
	if (genome->neurons->count == 0) return;

	for (int i = 0; i < times; i++) {
		//Choose a random node.
		neuron_t* node = genome->neurons->data[Random_Int(0, genome->neurons->count - 1)];

		//Do not alter frozen genes
		//if (!node->frozen)
		//	node->trait = genome->traits->data[Random_Int(0, genome->traits->count - 1)];
	}
}

void Genome_Mutate_Link_Weights(genome_t *genome, double power, double rate, enum mutator_e mut_type)
{	
	//The power of mutation will rise farther into the genome
	//on the theory that the older genes are more fit since
	//they have stood the test of time

	double num = 0.0; //counts gene placement
	double endpart = genome->genes->count*0.8; //Signifies the last part of the genome

	//Go through all the Genes and perturb their link's weights
	for (int i = 0; i < genome->genes->count; i++) {
		gene_t *curgene = genome->genes->data[i];

		double gausspoint = 1.0 - rate;
		double coldgausspoint = 1.0 - rate;

		//Once in a while really shake things up
		if (Random_Float() > 0.5)
		{
			gausspoint = 0.3;
			coldgausspoint = 0.1;
		}
		else if ((genome->genes->count >= 10) && (num > endpart))
		{
			gausspoint = 0.5;  //Mutate by modification % of connections
			coldgausspoint = 0.3; //Mutate the rest by replacement % of the time
		}
		else if (Random_Float() > 0.5)
		{
			coldgausspoint -= 0.1;
		}

		//Possible methods of setting the perturbation:
		//randnum=gaussrand()*powermod;
		//randnum=gaussrand();

		double randnum = ((Random_Float() - 0.5) * 2) * power;
		//std::cout << "RANDOM: " << randnum << " " << randposneg() << " " << randfloat() << " " << power << " " << powermod << std::endl;
		if (mut_type == NQ_GAUSSIAN) {
			double randchoice = Random_Float();
			if (randchoice > gausspoint)
				curgene->weight += randnum;
			else if (randchoice > coldgausspoint)
				curgene->weight = randnum;
		}
		else if (mut_type == NQ_COLDGAUSSIAN)
			curgene->weight = randnum;

		//Cap the weights at 8.0 (experimental)
		curgene->weight = fmin(fmax(-8.0, curgene->weight), 8.0);

		//Record the innovation
		curgene->mutation_num = curgene->weight;
		num += 1.0;
	}
}

// toggle genes on or off 
void Genome_Mutate_Toggle_Enable(genome_t *genome, int times)
{
	// Mutate a number of times.
	for (int i = 0; i < times; i++) {
		//Choose a random gene
		gene_t* gene = genome->genes->data[Random_Int(0, genome->genes->count - 1)];

		//Toggle the enable on this gene
		if (gene->enabled) {

			//We need to make sure that another gene connects out of the in-node
			//Because if not a section of network will break off and become isolated
			for (int j = 0; j < genome->genes->count; j++)
			{
				gene_t* other = genome->genes->data[j];
				if (other->inode == gene->inode &&
					other->enabled &&
					other->innovation_num != gene->innovation_num)
				{
					gene->enabled = false;
					break;
				}	
			}
		}
		else gene->enabled = true;
	}
}

// Find first disabled gene and enable it 
void Genome_Mutate_Gene_Reenable(genome_t *genome)
{
	for (int i = 0; i < genome->genes->count; i++)
	{
		gene_t *gene = genome->genes->data[i];
		if (!gene->enabled)
		{
			gene->enabled = true;
			break;
		}
	}
}

// These last kinds of mutations return false if they fail
//   They can fail under certain conditions,  being unable
//   to find a suitable place to make the mutation.
//   Generally, if they fail, they can be called again if desired. 

cbool Genome_Mutate_Add_Node(genome_t *genome, vector *innovs, int curnode_id, int curinnov)
{
	//double randmult;  //using a gaussian to find the random gene

	cbool found = false;
	gene_t *gene = 0;

	// Gene selection which skews towards older genes, whilst still retaining some randomness.
	if (true) {
		for (int i = 0; i < genome->genes->count; i++) 
		{
			gene = genome->genes->data[i];
			// - 1 from num_in to ignore the bias node. We don't want a mutation of 1!
			if (gene->enabled && gene->inode != genome->num_in - 1)
			{
				for (int j = i; j < genome->genes->count; j++)
				{
					gene = genome->genes->data[j];
					// Add some randomisation in, with a higher chance of getting older nodes.
					// This encourages splitting to distribute evenly.
					if (Random_Float() > 0.3 && gene->inode != genome->num_in - 1)
					{
						if (!gene->enabled) return false;
						found = true;
						break;
					}
				}
				break;
			}
		}
	}
	// Obsolated totally random gene selection method.
	else 
	{
		for (int i = 0; (i < 20 && !found); i++) 
		{
			gene = genome->genes->data[Random_Int(0, genome->genes->count - 1)];
			if (gene->enabled && gene->inode != genome->num_in - 1)
				found = true;
		}
	}

	//If we couldn't find anything say goodbye
	if (!found) return false;

	//Disabled the gene
	gene->enabled = false;

	//The weight of the original link
	double oldweight = gene->weight;

	//Extract the nodes
	int inode = gene->inode;
	int onode = gene->onode;

	//The new Genes
	gene_t *newgene1 = 0;
	gene_t *newgene2 = 0;

	//The new node
	neuron_t *newnode = 0;

	//Check to see if this innovation has already been done   
	//in another genome
	//Innovations are used to make sure the same innovation in
	//two separate genomes in the same generation receives
	//the same innovation number.
	for (int i = 0; i <= innovs->count; i++) 
	{
		if (i == innovs->count) // We couldn't find a related innovation. Make a totally novel one!
		{
			//Get the old link's trait
			//trait_t *traitptr = link->trait;

			//Create the new NNode
			//By convention, it will point to the first trait
			newnode = Neuron_Init();
			curnode_id++;
			//newnode->trait = genome->traits->data[0];

			//Create the new Genes
			newgene1 = Gene_Init(1.0, inode, curnode_id, curinnov, 0);
			newgene2 = Gene_Init(oldweight, curnode_id, onode, curinnov + 1, 0);
			curinnov += 2.0;

			//Add the innovations (remember what was done)
			vector_add(innovs, Innovation_Init(inode, onode, curinnov - 2.0, curinnov - 1.0, curnode_id, gene->innovation_num));
			break;
		}
		else 
		{
			innovation_t* innovation = innovs->data[i];
			if (innovation->innovation_type == NQ_NEWNODE &&
				innovation->node_in_id == inode &&
				innovation->node_out_id == onode &&
				innovation->old_innov_num == gene->innovation_num)
			{
				//Here, the innovation has been done before

				//Get the old link's trait
				//trait_t *traitptr = link->trait;

				//Create the new NNode
				newnode = Neuron_Init();
				//By convention, it will point to the first trait
				//Note: In future may want to change this
				//newnode->trait = genome->traits->data[0];

				//Create the new Genes
				newgene1 = Gene_Init(1.0, inode, innovation->new_node_id, innovation->innovation_num1, 0);
				newgene2 = Gene_Init(oldweight, innovation->new_node_id, onode, innovation->innovation_num2, 0);
				break;
			}
		}
	}

	//Now add the new NNode and new Genes to the Genome
	//genes.push_back(newgene1);   //Old way to add genes- may result in genes becoming out of order
	//genes.push_back(newgene2);
	Genome_Add_Gene(genome, genome->genes, newgene1);  //Add genes in correct order
	Genome_Add_Gene(genome, genome->genes, newgene2);

	vector_add(genome->neurons, newnode);
	//Genome_Node_Insert(genome, genome->neurons, newnode);

	return true;
}

cbool Genome_Mutate_Add_Link(genome_t *genome, vector *innovs, int curinnov, int tries)
{
	//random node indexs
	int nodenum1, nodenum2;

	//Storage for an existing link.
	gene_t *gene = 0;

	//Loop to find any existing links
	for (int i = 0; i < tries; i++) 
	{
		// Randomly pick nodes. 
		nodenum1 = Random_Int(0, genome->neurons->count - 1);
		nodenum2 = Random_Int(genome->num_in, genome->neurons->count - 1);

		//See if a link already exists
		for (int j = 0; j < genome->genes->count; j++)
		{
			gene_t* potential_gene = genome->genes->data[j];
			if (potential_gene->inode == nodenum1 && 
				potential_gene->onode == nodenum2)
			{
				gene = genome->genes->data[j];
				break;
			}
		}
	} //End of normal link finding loop

	//Continue only if an open link was found
	if (gene != 0) {

		gene_t *newgene = 0;  //The new Gene
		for (int i = 0; i <= innovs->count; i++)
		{
			if (i == innovs->count) // We couldn't find a related innovation. Make a totally novel one!
			{
				//Choose the new weight, between -1 and 1.
				double newweight = (Random_Float() - 0.5) * 2;
				newgene = Gene_Init(newweight, nodenum1, nodenum2, curinnov, newweight);

				//Add the innovation
				vector_add(innovs, Innovation_Init_Link(nodenum1, nodenum2, curinnov, newweight));

				curinnov = curinnov + 1.0;
				break;
			}
			else 
			{
				innovation_t* innovation = innovs->data[i];
				if (innovation->innovation_type == NQ_NEWLINK &&
					innovation->node_in_id == nodenum1 &&
					innovation->node_out_id == nodenum2)
				{
					//Here, the innovation has been done before

					//Create new gene
					//newgene = Gene_Init_Trait(genome->traits->data[innovation->new_trait_num], innovation->new_weight, nodep1, nodep2, recurflag, innovation->innovation_num1, 0);
					newgene = Gene_Init(innovation->new_weight, nodenum1, nodenum2, innovation->innovation_num1, 0);
					break;
				}
			}
		}
		Genome_Add_Gene(genome, genome->genes, newgene); //Adds the gene in correct order
		return true;
	}

	return false;
}

/*
void Genome_Mutate_Add_Sensor(genome_t *genome, vector *innovs, int curinnov)
{
	// Count how many nodes are connected.
	for (int i = 0; i < genome->num_in; i++)
	{
		neuron_t *sensor = genome->neurons->data[i];

		int outputConnections = 0;
		for (int i = 0; i < genome->genes->count; i++)
		{
			gene_t *gene = genome->genes->data[i];
			if (gene->onode >= genome->num_in && gene->onode < genome->num_in + genome->num_out)
				outputConnections++;
		}


		if (outputConnections == genome->num_out) 
		{
			vector_delete(sensors, i);
		}
	}

	//If all sensors are connected, quit
	if (sensors->count == 0) return;

	//Pick randomly from remaining sensors
	int input = Random_Int(0, sensors->count - 1);

	//Add new links to chosen sensor, avoiding redundancy


	for (int output = 0; output < outputs->count; output++)
	{
		cbool found = false;

		for (int j = 0; j < genome->genes->count; j++) 
		{
			gene_t *gene = genome->genes->data[j];
			if (gene->inode == input && gene->onode == output)
				found = true;
		}

		//Record the innovation
		if (!found) {
			gene_t* newgene = 0;
			for (int j = 0; j <= innovs->count; j++)
			{
				if (j == innovs->count) // We couldn't find a related innovation. Make a totally novel one!
				{

					//Choose a random trait
					//int trait_num = Random_Int(0, genome->traits->count - 1);

					//Choose the new weight
					//newweight=(gaussrand())/1.5;  //Could use a gaussian
					double newweight = ((Random_Float() - 0.5) * 2) * 3.0; //used to be 10.0

					//Create the new gene
					//newgene = Gene_Init_Trait(genome->traits->data[trait_num], newweight, sensor, output, false, curinnov, newweight);
					newgene = Gene_Init(newweight, input, output, curinnov, newweight);

					//Add the innovation
					vector_add(innovs, Innovation_Init_Link(input, output, curinnov, newweight));

					curinnov = curinnov + 1.0;
				}
				else
				{
					innovation_t* innovation = innovs->data[j];
					if (innovation->innovation_type == NQ_NEWLINK &&
						innovation->node_in_id == input &&
						innovation->node_out_id == output &&
						innovation->recur_flag == false)
					{
						//Create new gene
						//newgene = Gene_Init_Trait(genome->traits->data[innovation->new_trait_num], innovation->new_weight, sensor, output, false, innovation->innovation_num1, 0);
						newgene = Gene_Init(innovation->new_weight, input, output, innovation->innovation_num1, 0);
						break;
					}
				}
			}
			Genome_Add_Gene(genome, genome->genes, newgene);  //adds the gene in correct order
		} 
	}
}
*/

genome_t *Genome_Mate_Multipoint(genome_t *genome, genome_t *other, int genomeid, double fitness1, double fitness2, cbool interspec_flag)
{
	//The baby Genome will contain these new Traits, NNodes, and Genes
	vector *newnodes = vector_init(), *newgenes = vector_init();

	//Figure out which genome is better
	//The worse genome should not be allowed to add extra structural baggage
	//If they are the same, use the smaller one's disjoint and excess genes only
	cbool p1better = (fitness1 > fitness2) ? true : (fitness1 == fitness2) ? (genome->genes->count < other->genes->count) : false;

	// Add all inputs and outputs
	for (int i = 0; i < other->num_in + other->num_out; i++)
	{
		neuron_t *curnode = other->neurons->data[i];
		vector_add(newnodes, Neuron_Init_Derived(curnode));
	}


	//Now move through the Genes of each parent until both genomes end
	int loop_length = fmax(genome->genes->count, other->genes->count);
	for (int i = 0; i < loop_length; i++)
	{
		// Should we skip the excess genes in the larger of the two genomes?
		cbool skip = false;
		cbool disable = false; //Set to true if we want to disabled a chosen gene
		gene_t *chosengene = 0;  //Gene chosen for baby to inherit

		// Pick a gene to pass on to the child.
		if (i >= genome->genes->count) // We've run out of genes from out first genome. Use the second gene if it's better.
		{
			chosengene = other->genes->data[i];
			if (p1better) skip = true;  //Skip excess from the worse genome.
		}
		else if (i >= other->genes->count) // Vice versa
		{
			chosengene = genome->genes->data[i];
			if (!p1better) skip = true;  //Skip excess from the worse genome.
		}
		else
		{
			gene_t* p1gene = genome->genes->data[i];
			gene_t* p2gene = other->genes->data[i];

			double p1innov = p1gene->innovation_num;
			double p2innov = p2gene->innovation_num;

			if (p2innov == p1innov)
			{
				//Both innovations are the same; randomly pick one.
				chosengene = (Random_Float() < 0.5) ? p1gene : p2gene;

				//If one of the genes is disabled, the new one will likely also be.
				if ((!p1gene->enabled || !p2gene->enabled) && Random_Float(0.75)) disable = true;
			}
			else if (p1innov < p2innov)
			{
				chosengene = p1gene;
				if (!p1better) skip = true;
			}
			else if (p2innov < p1innov)
			{
				chosengene = p2gene;
				if (p1better) skip = true;
			}
		}

		// Check to see if the chosengene conflicts with an already chosen gene; Do they represent the same link?
		gene_t* checkedgene = 0;
		for (int j = 0; j < newgenes->count; j++) {
			checkedgene = newgenes->data[j];

			//Check if they either share the same node IDs, or if their inputs and outputs are recursive and link between each other.
			if ((checkedgene->inode == chosengene->inode &&
				checkedgene->onode == chosengene->onode) ||
				(checkedgene->inode == chosengene->onode &&
				checkedgene->onode == chosengene->inode))
			{
				skip = true;
				break;
			}
		}


		//Now add the chosengene to the baby
		if (!skip)
		{
			//Next check for the nodes, add them if not in the baby Genome already.
			if (newnodes->count <= chosengene->inode)
				vector_insert(newnodes, chosengene->inode, Neuron_Init());

			if (newnodes->count <= chosengene->onode)
				vector_insert(newnodes, chosengene->onode, Neuron_Init());

			//Add the Gene
			gene_t* newgene = Gene_Init_Dupe(chosengene, chosengene->inode, chosengene->onode);
			if (disable) {
				newgene->enabled = false;
				disable = false;
			}

			vector_add(newgenes, newgene);
		}
	}

	//Return the baby Genome
	return (Genome_Init(genomeid, newnodes, newgenes));
}

genome_t *Genome_Mate_Multipoint_Avg(genome_t *genome, genome_t *other, int genomeid, double fitness1, double fitness2, cbool interspec_flag)
{
	//The baby Genome will contain these new Traits, NNodes, and Genes
	vector *newnodes = vector_init(), *newgenes = vector_init();

	//Figure out which genome is better
	//The worse genome should not be allowed to add extra structural baggage
	//If they are the same, use the smaller one's disjoint and excess genes only
	cbool p1better = (fitness1 > fitness2) ? true : (fitness1 == fitness2) ? (genome->genes->count < other->genes->count) : false;

	// Add all inputs and outputs
	for (int i = 0; i < other->num_in + other->num_out; i++)
	{
		neuron_t *curnode = other->neurons->data[i];
		vector_add(newnodes, Neuron_Init_Derived(curnode));
	}

	gene_t *avgene = Gene_Init(0, 0, 0, 0, 0);

	//Now move through the Genes of each parent until both genomes end
	int loop_length = fmax(genome->genes->count, other->genes->count);
	for (int i = 0; i < loop_length; i++)
	{
		// Should we skip the excess genes in the larger of the two genomes?
		cbool skip = false;
		cbool disable = false; //Set to true if we want to disabled a chosen gene
		gene_t *chosengene = 0;  //Gene chosen for baby to inherit
		avgene->enabled = true;

		// Pick a gene to pass on to the child.
		if (i >= genome->genes->count) // We've run out of genes from out first genome. Use the second gene if it's better.
		{
			chosengene = other->genes->data[i];
			if (p1better) skip = true;  //Skip excess from the worse genome.
		}
		else if (i >= other->genes->count) // Vice versa
		{
			chosengene = genome->genes->data[i];
			if (!p1better) skip = true;  //Skip excess from the worse genome.
		}
		else
		{
			gene_t* p1gene = genome->genes->data[i];
			gene_t* p2gene = other->genes->data[i];

			double p1innov = p1gene->innovation_num;
			double p2innov = p2gene->innovation_num;

			if (p2innov == p1innov)
			{
				//if (Random_Float() > 0.5) avgene->trait = p1gene->trait;
				//else avgene->trait = p2gene->trait;

				//WEIGHTS AVERAGED HERE
				avgene->weight = (p1gene->weight + p2gene->weight) / 2.0;

				if (Random_Float() > 0.5) avgene->inode = p1gene->inode;
				else avgene->inode = p2gene->inode;

				if (Random_Float() > 0.5) avgene->onode = p1gene->onode;
				else avgene->onode = p2gene->onode;

				avgene->innovation_num = p1gene->innovation_num;
				avgene->mutation_num = (p1gene->mutation_num + p2gene->mutation_num) / 2.0;

				//If one of the genes is disabled, the new one will likely also be.
				if ((!p1gene->enabled || !p2gene->enabled) && Random_Float(0.75)) avgene->enabled = false;

				chosengene = avgene;
			}
			else if (p1innov < p2innov)
			{
				chosengene = p1gene;
				if (!p1better) skip = true;
			}
			else if (p2innov < p1innov)
			{
				chosengene = p2gene;
				if (p1better) skip = true;
			}
		}
		/*
		//Uncomment this line to let growth go faster (from both parents excesses)
		skip=false;

		//For interspecies mating, allow all genes through:
		if (interspec_flag)
		skip=false;
		*/


		//Check to see if the chosengene conflicts with an already chosen gene
		//i.e. do they represent the same link    

		gene_t* checkedgene = 0;
		for (int j = 0; j < newgenes->count; j++) {
			checkedgene = newgenes->data[j];

			//Check if they either share the same node IDs, or if their inputs and outputs are recursive and link between each other.
			if ((checkedgene->inode == chosengene->inode &&
				 checkedgene->onode == chosengene->onode) ||
				(checkedgene->inode == chosengene->onode &&
				 checkedgene->onode == chosengene->inode))
			{
				skip = true;
				break;
			}
		}



		//Now add the chosengene to the baby
		if (!skip)
		{
			//Next check for the nodes, add them if not in the baby Genome already.
			if (newnodes->count <= chosengene->inode)
				vector_insert(newnodes, chosengene->inode, Neuron_Init());

			if (newnodes->count <= chosengene->onode)
				vector_insert(newnodes, chosengene->onode, Neuron_Init());

			//Add the Gene
			gene_t* newgene = Gene_Init_Dupe(chosengene, chosengene->inode, chosengene->onode);
			if (disable) {
				newgene->enabled = false;
				disable = false;
			}

			vector_add(newgenes, newgene);
		}
	}

	Gene_Delete(avgene);

	//Return the baby Genome
	return (Genome_Init(genomeid, newnodes, newgenes));
}

genome_t *Genome_Mate_Singlepoint(genome_t *genome, genome_t *other, int genomeid)
{
	//The baby Genome will contain these new Traits, NNodes, and Genes
	vector *newnodes = vector_init(), *newgenes = vector_init();

	//int nodetraitnum;  //Trait number for a NNode

	//First, average the Traits from the 2 parents to form the baby's Traits
	//It is assumed that trait lists are the same length
	//In the future, may decide on a different method for trait mating
	//for (int i = 0; i < genome->traits->count; i++)
	//	vector_add(newtraits, Trait_Init_Merge(genome->traits->data[i], other->traits->data[i]));

	gene_t *avgene = Gene_Init(0, 0, 0, 0, 0);

	// Add all inputs and outputs
	for (int i = 0; i < other->num_in + other->num_out; i++)
	{
		neuron_t *curnode = other->neurons->data[i];
		vector_add(newnodes, Neuron_Init_Derived(curnode));
	}

	int crosspoint = Random_Int(0, fmin(genome->genes->count, other->genes->count) - 1);
	int geneCount = 0;

	//Now move through the Genes of each parent until both genomes end
	int loop_length = fmax(genome->genes->count, other->genes->count);
	for (int i = 0; i < loop_length; i++)
	{
		// Should we skip the excess genes in the larger of the two genomes?
		cbool skip = false;
		cbool disable = false; //Set to true if we want to disabled a chosen gene
		gene_t *chosengene = 0;  //Gene chosen for baby to inherit
		avgene->enabled = true;

		// Pick a gene to pass on to the child.
		if (i >= genome->genes->count) // We've run out of genes from out first genome. Use the second gene if it's better.
		{
			chosengene = other->genes->data[i];
		}
		else if (i >= other->genes->count) // Vice versa
		{
			chosengene = genome->genes->data[i];
		}
		else
		{
			gene_t* p1gene = genome->genes->data[i];
			gene_t* p2gene = other->genes->data[i];

			double p1innov = p1gene->innovation_num;
			double p2innov = p2gene->innovation_num;

			if (i == crosspoint && p2innov == p1innov)
			{
				//WEIGHTS AVERAGED HERE
				avgene->weight = (p1gene->weight + p2gene->weight) / 2.0;

				if (Random_Float() > 0.5) avgene->inode = p1gene->inode;
				else avgene->inode = p2gene->inode;

				if (Random_Float() > 0.5) avgene->onode = p1gene->onode;
				else avgene->onode = p2gene->onode;

				avgene->innovation_num = p1gene->innovation_num;
				avgene->mutation_num = (p1gene->mutation_num + p2gene->mutation_num) / 2.0;

				//If one of the genes is disabled, the new one will likely also be.
				if ((!p1gene->enabled || !p2gene->enabled) && Random_Float(0.75)) avgene->enabled = false;

				chosengene = avgene;
			}
			else if (p1innov < p2innov)
			{
				chosengene = (i < crosspoint) ? p1gene : p2gene;
			}
			else if (p2innov < p1innov)
			{
				skip = true;
			}
		}

		//Check to see if the chosengene conflicts with an already chosen gene
		//i.e. do they represent the same link    

		gene_t* checkedgene = 0;
		for (int j = 0; j < newgenes->count; j++) {
			checkedgene = newgenes->data[j];

			//Check if they either share the same node IDs, or if their inputs and outputs are recursive and link between each other.
			if (checkedgene->inode == chosengene->inode &&
				checkedgene->onode == chosengene->onode)
			{
				skip = true;
				break;
			}
		}

		//Now add the chosengene to the baby
		if (!skip && chosengene != NULL)
		{
			//Next check for the nodes, add them if not in the baby Genome already.
			if (newnodes->count <= chosengene->inode)
				vector_insert(newnodes, chosengene->inode, Neuron_Init());

			if (newnodes->count <= chosengene->onode)
				vector_insert(newnodes, chosengene->onode, Neuron_Init());

			//Add the Gene
			gene_t* newgene = Gene_Init_Dupe(chosengene, chosengene->inode, chosengene->onode);
			if (disable) {
				newgene->enabled = false;
				disable = false;
			}

			vector_add(newgenes, newgene);
		}
	}

	Gene_Delete(avgene);

	//Return the baby Genome
	return (Genome_Init(genomeid, newnodes, newgenes));
}

/*
genome_t *Genome_Mate_Crossover(genome_t *x, genome_t *y)
{
	if (y->fitness > x->fitness)
	{
		genome_t *temp = x;
		x = y;
		y = temp;
	}
	Genome_Init_Empty();

	vector* innovation_genes = vector_init();
	for (int i = 0; i < y->genes->count; i++)
	{
		gene_t *gene = y->genes->data[i];
		vector_insert(innovation_genes, gene->innovation_num, gene);
	}

	for (int i = 0; i < x->genes->count; i++)
	{
		gene_t *gene1 = x->genes->data[i];
		gene_t *gene2 = innovation_genes->data[gene1->innovation_num];
		vector_add( (gene2 != NULL && gene2->enabled && Random_Float() > 0.5)

	}
	Genome_Init()
	genome_t *child = Genome_Init
}
*/

// Figure out the difference between innovations in both genes
double Genome_Disjoint(genome_t *x, genome_t *y)
{
	vector *innov1 = vector_init(), *innov2 = vector_init();

	for (int i = 0; i < x->genes->count; i++)
	{
		gene_t *gene = x->genes->data[i];
		vector_insert(innov1, gene->innovation_num, (void*)1);
	}

	for (int i = 0; i < y->genes->count; i++)
	{
		gene_t *gene = y->genes->data[i];
		vector_insert(innov2, gene->innovation_num, (void*)1);
	}

	// Check for the number of disjointed genes; genes whose innovations are unique to a single genome.
	int disjointGenes = 0;
	for (int i = 0; i < x->genes->count; i++)
	{
		gene_t *gene = x->genes->data[i];
		if (innov2->count <= gene->innovation_num || innov2->data[gene->innovation_num] != (void *)1)
			disjointGenes++;
	}

	for (int i = 0; i < y->genes->count; i++)
	{
		gene_t *gene = y->genes->data[i];
		if (innov1->count <= gene->innovation_num || innov1->data[gene->innovation_num] != (void *)1)
			disjointGenes++;
	}

	vector_free_all(innov1);
	vector_free_all(innov2);

	return (disjointGenes / fmax(x->genes->count, y->genes->count));
}

double Genome_Weights(genome_t *x, genome_t *y)
{
	vector *innov2 = vector_init();

	for (int i = 0; i < y->genes->count; i++)
	{
		gene_t *gene = y->genes->data[i];
		vector_insert(innov2, gene->innovation_num, gene);
	}

	double sum = 0.0;
	double coincident = 0.0;
	for (int i = 0; i < x->genes->count; i++)
	{
		gene_t *gene = x->genes->data[i];
		if (innov2->data[gene->innovation_num] != 0)
		{
			gene_t *gene2 = innov2->data[gene->innovation_num];
			sum += fabs(gene->weight - gene2->weight);
			coincident++;
		}
	}

	return sum / coincident;
}

double Genome_Compatibility(genome_t *x, genome_t *y)
{
	return (NQ_DELTA_DISJOINT * Genome_Disjoint(x, y) + NQ_DELTA_WEIGHTS * Genome_Weights(x, y));
}

/*
double Genome_Compatibility(genome_t *genome, genome_t *other)
{
	//Set up the counters
	double num_disjoint = 0.0;
	double num_excess = 0.0;
	double mut_diff_total = 0.0;
	double num_matching = 0.0;  //Used to normalize mutation_num differences
	
	int loop_length = fmax(genome->genes->count, other->genes->count);
	for (int i = 0; i < loop_length; i++)
	{
		if (i >= genome->genes->count || i >= other->genes->count) {
			num_excess += 1.0;
		}
		else {
			gene_t *p1gene = genome->genes->data[i];
			gene_t *p2gene = other->genes->data[i];

			//Extract current innovation numbers
			double p1innov = p1gene->innovation_num;
			double p2innov = p2gene->innovation_num;

			if (p1innov == p2innov) 
			{
				num_matching += 1.0;
				//mut_diff+=trait_compare((*p1gene)->lnk->linktrait,(*p2gene)->lnk->linktrait); //CONSIDER TRAIT DIFFERENCES
				mut_diff_total += fabs(p1gene->mutation_num - p2gene->mutation_num);
			}
			else if (p1innov < p2innov || p2innov < p1innov)
			{
				num_disjoint += 1.0;
			}
		}
	}

	return (NQ_DISJOINT_COEFF * (num_disjoint / 1.0) +
		NQ_EXCESS_COEFF * (num_excess / 1.0) +
		NQ_MUTDIFF_COEFF * (mut_diff_total / num_matching));
}

double Genome_Trait_Compare(genome_t *genome, trait_t *t1, trait_t *t2)
{
	if ((t1->id == 1 && t2->id >= 2) || (t2->id == 1 && t1->id >= 2))
		return 0.5;
	else if (t1->id >= 2) 
	{
		double params_diff = 0.0;
		for (int i = 0; i <= 2; i++) {
			params_diff += fabs(t1->params[i] - t2->params[i]);
		}
		return params_diff / 4.0;
	}
	return 0.0;
}
*/

int Genome_Extrons(genome_t *genome)
{
	int total = 0;
	for (int i = 0; i < genome->genes->count; i++)
		if (((gene_t*)genome->genes->data[i])->enabled)
			total++;
	return total;
}

/*
void Genome_Randomize_Traits(genome_t *genome)
{
	for (int i = 0; i < genome->neurons->count; i++)
	{
		neuron_t *curnode = genome->neurons->data[i];
		int trait_num = Random_Int(1, genome->traits->count);
		curnode->trait_id = trait_num;

		trait_t *curtrait = 0;
		for (int j = 0; j < genome->traits->count; j++)
		{
			curtrait = genome->traits->data[j];
			if (curtrait->id == trait_num) break;
		}
		curnode->trait = curtrait;
	}

	for (int i = 0; i < genome->genes->count; i++)
	{
		gene_t *curgene = genome->genes->data[i];
		int trait_num = Random_Int(1, genome->traits->count);
		curgene->trait_id = trait_num;

		trait_t *curtrait = 0;
		for (int j = 0; j < genome->traits->count; j++)
		{
			curtrait = genome->traits->data[j];
			if (curtrait->id == trait_num) break;
		}
		curgene->trait = curtrait;
	}
}
*/

void Genome_Add_Gene(genome_t *genome, vector *glist, gene_t *g)
{
	for (int i = 0; i < glist->count; i++)
	{
		gene_t* curgene = glist->data[i];
		if (curgene->innovation_num >= g->innovation_num)
			vector_insert(glist, i, g);
	}
}

void Genome_FPrint(genome_t* genome, FILE *f)
{
	fprintf(f, "gnome_s %d %f %f %f %f\n", genome->id, genome->fitness, genome->final_pos[0], genome->final_pos[1], genome->final_pos[2]);

	fprintf(f, "nodes %d %d %d\n", genome->num_in, genome->num_out, genome->neurons->count);

	for (int i = 0; i < genome->genes->count; i++)
		Gene_FPrint(genome->genes->data[i], f);

	fprintf(f, "gnome_e %d\n\n", genome->id);
}

cbool Genome_Quicksort_Genes(gene_t* x, gene_t* y)
{
	return(x->onode <= y->onode);
}