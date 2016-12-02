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
#include <string.h>

#include "genome.h"
#include "neural.h"
#include "population.h"

//Constructor which takes full genome specs and puts them into the new one
genome_t* Genome_Init(vector* genes, unsigned short num_in, unsigned short num_out)
{
	genome_t* genome = (genome_t*)malloc(sizeof(genome_t));
	if (genome == 0) return NULL;

	genome->neurons = vector_init();
	genome->genes = genes;
	genome->fitness = 0;
	genome->global_rank = 0;
	genome->num_in = num_in;
	genome->num_out = num_out;

	return genome;
}

genome_t *Genome_Init_Copy(genome_t *genome)
{
	//Collections for the new Genome
	vector *genes = vector_init();

	//Duplicate Genes
	for (unsigned int i = 0; i < genome->genes->count; i++)
	{
		gene_t *curgene = genome->genes->data[i];
		vector_push(genes, Gene_Init_Copy(curgene));
	}

	//Finally, initialize basic values and return the new genome
	genome_t *newgenome = Genome_Init(genes, genome->num_in, genome->num_out);
	newgenome->fitness = genome->fitness;
	newgenome->global_rank = genome->global_rank;

	return newgenome;
}

//Special constructor that creates a Genome of 2 possible types:
//0 - Fully linked, no hidden nodes.
//1 - Fully linked, num hidden node splitting input and output.
genome_t* Genome_Init_Auto(unsigned short num_in, unsigned short num_out, unsigned short num_hidden, unsigned char type)
{
	vector *genes = vector_init();
	++num_in; // Add the bias.

	//Create the links depending on the type

	// Type 0 links each input to each output.
	if (type == 0)
	{
		unsigned int count = 1;

		for (unsigned int i = num_in; i < (unsigned int)(num_in + num_out); i++)
		{
			for (unsigned int j = 0; j < num_in; j++)
			{
				vector_push(genes, Gene_Init(0, j, i, count));
				count++;
			}
		}

	}
	// Type 1 creates a number of hidden nodes and links inputs between them and the output.
	else
	{
		unsigned int count = 1;

		//Initialize links between layers.
		for (unsigned short i = num_in; i < (unsigned short)(num_in + num_out); i++)
		{
			for (unsigned short j = 0; j < num_in; j++)
			{
				for (unsigned short k = (unsigned short)(num_in + num_out); k < (unsigned short)(num_in + num_out + num_hidden); k++)
				{
					// Connect Input to hidden.
					vector_push(genes, Gene_Init(0, j, k, count));
					count++;
					// Connect hidden to output.
					vector_push(genes, Gene_Init(0, k, i, count));
					count++;
				}
			}
		}
	}

	return Genome_Init(genes, num_in, num_out);
}

genome_t* Genome_Init_Load(FILE *f, char *argline)
{
	char wordbuf[1024];
	strcpy(wordbuf, argline);

	vector *genes = vector_init();
	unsigned short num_in, num_out;

	// Read in the header first.
	char *curword = strtok(wordbuf, " \t\n");
	if (strcmp(curword, "gn_s") != 0)
	{
		Con_Printf("Erroneus genome loaded!\n");
		return 0;
	}

	curword = strtok(NULL, " \t\n");
	// Error handling.
	if (curword == NULL)
	{
		Con_Printf("Error loading num_in from genome!\n");
		return 0;
	}

	// Convert each argument to its decimal equivalent.
	sscanf(curword, "%d", &num_in);

	curword = strtok(NULL, " \t\n");
	if (curword == NULL)
	{
		Con_Printf("Error loading num_out from genome!\n");
		return 0;
	}
	sscanf(curword, "%d", &num_out);

	char curline[1024]; //max line size of 1024 characters

	while (fgets(curline, sizeof(curline), f))
	{
		// Create a copy of the curline so we can cut it up with strtok.
		char lineCopy[1024];
		strcpy(lineCopy, curline);

		curword = strtok(lineCopy, "\t\n");
		
		if (curword != NULL)
		{
			// Genome end reached. 
			if (strcmp(curword, "gn_e") == 0)
			{
				// Return the completed genome.
				return Genome_Init(genes, num_in, num_out);
			}
			//Print any comments to the console.
			else if (strcmp(curword, "/*") == 0)
			{
				char metadata[128];
				cbool md = false;
				strcpy(metadata, "");
				curword = strtok(NULL, " \t\n");
				while (curword != NULL && strcmp(curword, "*/") != 0)
				{
					if (md) strncat(metadata, " ", 128 - strlen(metadata));
					md = true;

					strncat(metadata, curword, 128 - strlen(metadata));
					curword = strtok(NULL, " \n\t");
				}
				Con_Printf(metadata);
			}
			//Read in a gene
			else
			{
				char argline[1024];
				strcpy(argline, curline);
				vector_push(genes, Gene_Init_Load(argline));
			}
		}
	}

	return Genome_Init(genes, num_in, num_out);
}

// Destructor kills off all lists
void Genome_Delete(genome_t *genome)
{
	// A neuron may be null if it is a hidden node in a different genome.
	for (unsigned int i = 0; i < genome->neurons->count; ++i)
		if (genome->neurons->data[i] != NULL)
			Neuron_Delete(genome->neurons->data[i]);

	vector_free_all(genome->neurons);

	for (unsigned int i = 0; i < genome->genes->count; i++)
		Gene_Delete(genome->genes->data[i]);

	vector_free_all(genome->genes);

	free(genome);
}

void Genome_Save(genome_t* genome, FILE *f)
{
	fprintf(f, "gn_s\t%d\t%d\n", genome->num_in, genome->num_out);

	for (unsigned int i = 0; i < genome->genes->count; i++)
		Gene_Save(genome->genes->data[i], f);

	fprintf(f, "gn_e\n\n");
}

// Feeds values through the neural network and activates all outputs with values.
void Genome_Activate(genome_t* genome)
{
	// For each node, compute the sum of its incoming activation 
	for (unsigned int i = genome->num_in; i < genome->neurons->count; i++)
	{
		neuron_t* curnode = genome->neurons->data[i];

		// Hidden nodes may not exist in particular genomes, 
		// but they still take up a slot with a NULL.
		if (curnode != NULL)
		{
			double sum = 0;

			// For each incoming connection, add the weight from the connection to the sum. 
			for (unsigned int j = 0; j < curnode->incoming_genes->count; j++)
			{
				gene_t *incoming_gene = curnode->incoming_genes->data[j];
				neuron_t *input_node = genome->neurons->data[incoming_gene->inode];

				if (input_node != NULL)
					sum += incoming_gene->weight * input_node->value;
			}

			if (sum != 0) curnode->value = Sigmoid(sum);
		}
	}
}

// Takes an array of sensor values and loads it into inputs ONLY
void Genome_Load_Inputs(genome_t* genome, double* sensvals)
{
	// -1 so we don't load values into the bias.
	for (int i = 0; i < genome->num_in - 1; i++)
	{
		neuron_t *curnode = genome->neurons->data[i];
		curnode->value = sensvals[i];
	}

	// Load value of 1 into the bias.
	((neuron_t*)genome->neurons->data[genome->num_in])->value = 1.0;
}

unsigned int Genome_Neuron_Depth(vector* nodes, unsigned short node_index, unsigned char d)
{
	if (d > 100) return 10;

	int max = d;
	neuron_t *node = nodes->data[node_index];

	for (unsigned int i = 0; i < node->incoming_genes->count; i++)
	{
		gene_t *incoming_gene = node->incoming_genes->data[i];
		int cur_depth = Genome_Neuron_Depth(nodes, incoming_gene->inode, d + 1);
		if (cur_depth > max) max = cur_depth;
	}

	return max;
}

unsigned int Genome_Max_Depth(genome_t* genome)
{
	unsigned int cur_depth = 0, max = 0;

	for (unsigned int i = genome->num_in; i < (unsigned int)(genome->num_in + genome->num_out); i++)
	{
		cur_depth = Genome_Neuron_Depth(genome->neurons, i, 0);
		if (cur_depth > max) max = cur_depth;
	}

	return max;
}

//Generate a network of neurons from the data within the genome with specified ID
void Genome_Genesis(genome_t *genome)
{
	// Erase any existing nodes.
	for (unsigned int i = 0; i < genome->neurons->count; i++)
	{
		free(genome->neurons->data[genome->neurons->count - (i+1)]);
		vector_pop(genome->neurons);
	}

	// Initialize the input and output nodes in the network.
	for (unsigned int i = 0; i < (unsigned int)(genome->num_in + genome->num_out); ++i)
		vector_push(genome->neurons, Neuron_Init());

	Quicksort(0, genome->genes->count-1, genome->genes->data, Gene_Quicksort_By_OutputID);

	for (unsigned int i = 0; i < genome->genes->count; ++i)
	{
		gene_t* curgene = genome->genes->data[i];
		//Only create the link if the gene is enabled
		if (curgene->enabled == true)
		{
			//Create any non-existant nodes required by the genes.
			if (curgene->inode >= genome->neurons->count || genome->neurons->data[curgene->inode] == NULL)
				vector_insert(genome->neurons, curgene->inode, Neuron_Init());

			if (curgene->onode >= genome->neurons->count || genome->neurons->data[curgene->onode] == NULL)
				vector_insert(genome->neurons, curgene->onode, Neuron_Init());

			// Add the gene to the incoming link to the onode.
			neuron_t *node = genome->neurons->data[curgene->onode];
			vector_push(node->incoming_genes, curgene);
		}
	}
}

void Genome_Clear_Nodes(genome_t *genome)
{
	for (int i = genome->neurons->count - 1; i >= 0; i--)
	{
		free(genome->neurons->data[i]);
		vector_pop(genome->neurons);
	}
	vector_free(genome->neurons);
}

unsigned int Genome_Neuron_Total(genome_t *genome)
{
	unsigned int count = 0;

	// Find the highest node id within the genes. This is the count.
	for (unsigned int i = 0; i < genome->genes->count; i++)
	{
		gene_t *gene = genome->genes->data[i];
		count = fmax(count, fmax(gene->inode, gene->onode));
	}

	// Add 1 because node ids start at 0.
	return count + 1;
}

double Genome_Get_Last_Gene_Innovnum(genome_t *genome)
{
	return ((gene_t*)genome->genes->data[genome->genes->count - 1])->innovation_num + 1;
}

void Genome_Mutate(genome_t *genome, population_t *pop)
{
	// Do the mutation depending on probabilities of various mutations
	if (Random_Float() < NQ_MUTATE_ADD_NODE_PROB)
	{
		Genome_Mutate_Add_Node(genome, pop);
	}
	else if (Random_Float() < NQ_MUTATE_ADD_LINK_PROB)
	{
		Genome_Mutate_Add_Link(genome, pop);
	}
	else
	{
		//If we didn't do a structural mutation, we do the other kinds
		if (Random_Float() < NQ_MUTATE_LINK_WEIGHTS_PROB)
			Genome_Mutate_Link_Weights(genome, NQ_WEIGHT_MUT_POWER, 1.0, NQ_GAUSSIAN);
		if (Random_Float() < NQ_MUTATE_GENE_TOGGLE_PROB)
			Genome_Mutate_Gene_Toggle(genome, 20);
		if (Random_Float() < NQ_MUTATE_GENE_ENABLE_PROB)
			Genome_Mutate_Gene_Enable(genome);
	}
}

void Genome_Mutate_Link_Weights(genome_t *genome, double power, double rate, enum mutator_e mut_type)
{	
	//The power of mutation will rise farther into the genome
	//on the theory that the older genes are more fit since
	//they have stood the test of time

	//Signifies the last part of the genome
	unsigned int endpart = genome->genes->count*0.8;

	//Go through all the Genes and perturb their link's weights
	for (unsigned int i = 0; i < genome->genes->count; i++) {
		gene_t *curgene = genome->genes->data[i];

		double gausspoint = 1.0 - rate;
		double coldgausspoint = 1.0 - rate;

		//Once in a while really shake things up
		if (Random_Float() > 0.5)
		{
			gausspoint = 0.3;
			coldgausspoint = 0.1;
		}
		else if ((genome->genes->count >= 10) && (i > endpart))
		{
			gausspoint = 0.5;  //Mutate by modification % of connections
			coldgausspoint = 0.3; //Mutate the rest by replacement % of the time
		}
		else if (Random_Float() > 0.5)
		{
			coldgausspoint -= 0.1;
		}

		double randnum = ((Random_Float() - 0.5) * 2) * power;

		if (mut_type == NQ_GAUSSIAN) 
		{
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
	}
}

// Toggle genes on or off 
void Genome_Mutate_Gene_Toggle(genome_t *genome, unsigned char times)
{
	// Mutate a number of times.
	for (unsigned char i = 0; i < times; i++) {
		//Choose a random gene
		gene_t* gene = genome->genes->data[Random_Int(0, genome->genes->count - 1)];

		//Toggle the enable on this gene
		if (gene->enabled) {

			//We need to make sure that another gene connects out of the in-node
			//Because if not a section of network will break off and become isolated
			for (unsigned int j = 0; j < genome->genes->count; j++)
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
void Genome_Mutate_Gene_Enable(genome_t *genome)
{
	for (unsigned int i = 0; i < genome->genes->count; i++)
	{
		gene_t *gene = genome->genes->data[i];
		if (!gene->enabled)
		{
			gene->enabled = true;
			break;
		}
	}
}

cbool Genome_Mutate_Add_Node(genome_t *genome, population_t *pop)
{
	cbool found = false;
	gene_t *gene = 0;

	// Attempt to find an enabled gene.
	for (int i = 0; i < NQ_NEWNODE_TRIES && !found; i++)
	{
		gene = genome->genes->data[Random_Int(0, Genome_Neuron_Total(genome) - 1)];
		if (gene->enabled && gene->inode != genome->num_in - 1)
			found = true;
	}

	// If we couldn't find anything say adios amigos
	if (!found) return false;

	// The weight of the original link
	double oldweight = gene->weight;

	// The new Genes
	gene_t *newgene1 = Gene_Init_Copy(gene);
	gene_t *newgene2 = Gene_Init_Copy(gene);

	// Disable the gene - after we copy the enabled value over.
	gene->enabled = false;

	// Create the new node
	vector_push(genome->neurons, Neuron_Init());

	newgene1->onode = Genome_Neuron_Total(genome);
	newgene1->weight = 1.0;
	newgene1->innovation_num = Population_New_Innovation(pop);

	newgene2->inode = Genome_Neuron_Total(genome);
	newgene2->innovation_num = Population_New_Innovation(pop);

	vector_push(genome->genes, newgene1);
	vector_push(genome->genes, newgene2);

	return true;
}

cbool Genome_Mutate_Add_Link(genome_t *genome, population_t *pop)
{
	// Give it as many tries as needed to mutate a link.
	for (int i = 0; i < NQ_NEWLINK_TRIES; i++)
	{
		//Random input node.
		int nodenum1 = Random_Int(0, genome->num_in - 1);

		//Random other node.
		int nodenum2 = Random_Int(genome->num_in, Genome_Neuron_Total(genome));

		//Storage for a potential existing link.
		gene_t *gene = 0;

		for (unsigned int i = 0; i < genome->genes->count; i++)
		{
			gene_t* potential_gene = genome->genes->data[i];
			if (potential_gene->inode == nodenum1 &&
				potential_gene->onode == nodenum2)
			{
				gene = genome->genes->data[i];
				break;
			}
		}

		if (gene != 0)
		{
			vector_push(genome->genes, Gene_Init((Random_Float() - 0.5) * 2, nodenum1, nodenum2, Population_New_Innovation(pop)));
			return true;
		}
	}
	return false;
}

genome_t *Genome_Mate_Multipoint(genome_t *x, genome_t *y)
{
	//The baby Genome will contain these new Traits, NNodes, and Genes
	vector *newgenes = vector_init();

	//Figure out which genome is better
	//The worse genome should not be allowed to add extra structural baggage
	//If they are the same, use the smaller one's disjoint and excess genes only
	cbool p1better = (x->fitness > y->fitness) ? true : (x->fitness == y->fitness) ? (x->genes->count < y->genes->count) : false;

	// Add all inputs and outputs
	//for (int i = 0; i < y->num_in + y->num_out; i++)
	//{
	//	neuron_t *curnode = y->neurons->data[i];
	//	vector_push(newnodes, Neuron_Init(curnode));
	//}

	//Now move through the Genes of each parent until both genomes end
	unsigned int loop_length = fmax(x->genes->count, y->genes->count);
	for (unsigned int i = 0; i < loop_length; i++)
	{
		// Should we skip the excess genes in the larger of the two genomes?
		cbool skip = false;
		cbool disable = false; //Set to true if we want to disabled a chosen gene
		gene_t *chosengene = 0;  //Gene chosen for baby to inherit

		// Pick a gene to pass on to the child.
		if (i >= x->genes->count) // We've run out of genes from out first genome. Use the second gene if it's better.
		{
			chosengene = y->genes->data[i];
			if (p1better) skip = true;  //Skip excess from the worse genome.
		}
		else if (i >= y->genes->count) // Vice versa
		{
			chosengene = x->genes->data[i];
			if (!p1better) skip = true;  //Skip excess from the worse genome.
		}
		else
		{
			gene_t* p1gene = x->genes->data[i];
			gene_t* p2gene = y->genes->data[i];

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

		//Check to see if the chosengene conflicts with an already chosen gene.
		if (!skip && Gene_Is_Within(chosengene, newgenes)) skip = true;

		//Now add the chosengene to the baby
		if (!skip)
		{
			//Next check for the nodes, add them if not in the baby Genome already.
			//if (newnodes->count <= chosengene->inode)
			//	vector_insert(newnodes, chosengene->inode, Neuron_Init());
			//
			//if (newnodes->count <= chosengene->onode)
			//	vector_insert(newnodes, chosengene->onode, Neuron_Init());

			//Add the Gene
			gene_t* newgene = Gene_Init_Copy(chosengene);
			if (disable) {
				newgene->enabled = false;
				disable = false;
			}

			vector_push(newgenes, newgene);
		}
	}

	//Return the baby Genome
	return (Genome_Init(newgenes, y->num_in, y->num_out));
}

genome_t *Genome_Mate_Multipoint_Avg(genome_t *x, genome_t *y)
{
	//The baby Genome will contain these new Traits, NNodes, and Genes
	vector *newgenes = vector_init();

	//Figure out which genome is better
	//The worse genome should not be allowed to add extra structural baggage
	//If they are the same, use the smaller one's disjoint and excess genes only
	cbool p1better = (x->fitness > y->fitness) ? true : (x->fitness == y->fitness) ? (x->genes->count < y->genes->count) : false;

	// Add all inputs and outputs
	//for (unsigned int i = 0; i < y->num_in + y->num_out; i++)
	//{
	//	neuron_t *curnode = y->neurons->data[i];
	//	vector_push(newnodes, Neuron_Init(curnode));
	//}

	gene_t *avgene = Gene_Init(0, 0, 0, 0);

	//Now move through the Genes of each parent until both genomes end
	unsigned int loop_length = fmax(x->genes->count, y->genes->count);
	for (unsigned int i = 0; i < loop_length; i++)
	{
		// Should we skip the excess genes in the larger of the two genomes?
		cbool skip = false;
		cbool disable = false; //Set to true if we want to disabled a chosen gene
		gene_t *chosengene = 0;  //Gene chosen for baby to inherit
		avgene->enabled = true;

		// Pick a gene to pass on to the child.
		if (i >= x->genes->count) // We've run out of genes from out first genome. Use the second gene if it's better.
		{
			chosengene = y->genes->data[i];
			if (p1better) skip = true;  //Skip excess from the worse genome.
		}
		else if (i >= y->genes->count) // Vice versa
		{
			chosengene = x->genes->data[i];
			if (!p1better) skip = true;  //Skip excess from the worse genome.
		}
		else
		{
			gene_t* p1gene = x->genes->data[i];
			gene_t* p2gene = y->genes->data[i];

			double p1innov = p1gene->innovation_num;
			double p2innov = p2gene->innovation_num;

			if (p2innov == p1innov)
			{
				//WEIGHTS AVERAGED HERE
				avgene->weight = (p1gene->weight + p2gene->weight) / 2.0;

				if (Random_Float() > 0.5) avgene->inode = p1gene->inode;
				else avgene->inode = p2gene->inode;

				if (Random_Float() > 0.5) avgene->onode = p1gene->onode;
				else avgene->onode = p2gene->onode;

				avgene->innovation_num = p1gene->innovation_num;
//				avgene->mutation_num = (p1gene->mutation_num + p2gene->mutation_num) / 2.0;

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

		//Check to see if the chosengene conflicts with an already chosen gene.
		if (!skip && Gene_Is_Within(chosengene, newgenes)) skip = true;

		//Now add the chosengene to the baby
		if (!skip)
		{
			//Next check for the nodes, add them if not in the baby Genome already.
			//if (newnodes->count <= chosengene->inode)
			//	vector_insert(newnodes, chosengene->inode, Neuron_Init());
			//
			//if (newnodes->count <= chosengene->onode)
			//	vector_insert(newnodes, chosengene->onode, Neuron_Init());

			//Add the Gene
			gene_t* newgene = Gene_Init_Copy(chosengene);
			if (disable) {
				newgene->enabled = false;
				disable = false;
			}

			vector_push(newgenes, newgene);
		}
	}

	Gene_Delete(avgene);

	//Return the baby Genome
	return (Genome_Init(newgenes, y->num_in, y->num_out));
}

genome_t *Genome_Mate_Singlepoint(genome_t *x, genome_t *y)
{
	//The baby Genome will contain these new Traits, NNodes, and Genes
	vector *newgenes = vector_init();
	gene_t *avgene = Gene_Init(0, 0, 0, 0);

	// Add all inputs and outputs
	//for (int i = 0; i < y->num_in + y->num_out; i++)
	//{
	//	neuron_t *curnode = y->neurons->data[i];
	//	vector_push(newnodes, Neuron_Init(curnode));
	//}

	unsigned int crosspoint = Random_Int(0, fmin(x->genes->count, y->genes->count) - 1);
	unsigned int geneCount = 0;

	//Now move through the Genes of each parent until both genomes end
	unsigned int loop_length = fmax(x->genes->count, y->genes->count);
	for (unsigned int i = 0; i < loop_length; i++)
	{
		// Should we skip the excess genes in the larger of the two genomes?
		cbool skip = false;
		cbool disable = false; //Set to true if we want to disabled a chosen gene
		gene_t *chosengene = 0;  //Gene chosen for baby to inherit
		avgene->enabled = true;

		// Pick a gene to pass on to the child.
		if (i >= x->genes->count) // We've run out of genes from out first genome. Use the second gene if it's better.
		{
			chosengene = y->genes->data[i];
		}
		else if (i >= y->genes->count) // Vice versa
		{
			chosengene = x->genes->data[i];
		}
		else
		{
			gene_t* p1gene = x->genes->data[i];
			gene_t* p2gene = y->genes->data[i];

			double p1innov = p1gene->innovation_num;
			double p2innov = p2gene->innovation_num;

			// We've reached the crosspoint
			if (i == crosspoint && p2innov == p1innov)
			{
				// Average out the weight of parent genes.
				avgene->weight = (p1gene->weight + p2gene->weight) / 2.0;

				if (Random_Float() > 0.5) avgene->inode = p1gene->inode;
				else avgene->inode = p2gene->inode;

				if (Random_Float() > 0.5) avgene->onode = p1gene->onode;
				else avgene->onode = p2gene->onode;

				avgene->innovation_num = p1gene->innovation_num;
//				avgene->mutation_num = (p1gene->mutation_num + p2gene->mutation_num) / 2.0;

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

		//Check to see if the chosengene conflicts with an already chosen gene.
		if (!skip && Gene_Is_Within(chosengene, newgenes)) skip = true;

		//Now add the chosengene to the baby
		if (!skip)
		{
			//Next check for the nodes, add them if not in the baby Genome already.
			//if (newnodes->count <= chosengene->inode)
			//	vector_insert(newnodes, chosengene->inode, Neuron_Init());
			//
			//if (newnodes->count <= chosengene->onode)
			//	vector_insert(newnodes, chosengene->onode, Neuron_Init());

			//Add the Gene
			gene_t* newgene = Gene_Init_Copy(chosengene);
			if (disable) {
				newgene->enabled = false;
				disable = false;
			}

			vector_push(newgenes, newgene);
		}
	}

	Gene_Delete(avgene);

	//Return the baby Genome
	return (Genome_Init(newgenes, y->num_in, y->num_out));
}

genome_t *Genome_Mate_Crossover(genome_t *x, genome_t *y)
{
	// Ensure y is the weaker genome, as x's genes take 
	// precedence during the crossover operation.
	if (y->fitness > x->fitness)
	{
		genome_t *temp = x;
		x = y;
		y = temp;
	}

	vector *newgenes = vector_init();
	vector *innovation_genes = vector_init();

	// Add all innovations from the weaker genome.
	for (unsigned int i = 0; i < y->genes->count; i++)
	{
		gene_t *gene = y->genes->data[i];
		vector_insert(innovation_genes, gene->innovation_num, gene);
	}

	// Loop through the stronger genomes genes in order to cross them over.
	for (unsigned int i = 0; i < x->genes->count; i++)
	{
		gene_t *gene1 = x->genes->data[i];
		gene_t *gene2 = innovation_genes->data[gene1->innovation_num];

		// If the weaker genome has a gene of the same innovation, 
		// randomly assign that gene in place of the stronger genome's gene.
		if (gene1->innovation_num < innovation_genes->count && gene2 != NULL && gene2->enabled && Random_Float() > 0.5)
			vector_push(newgenes, Gene_Init_Copy(gene2));
		else
			vector_push(newgenes, Gene_Init_Copy(gene1));
	}

	vector_free_all(innovation_genes);

	return (Genome_Init(newgenes, y->num_in, y->num_out));
}

// Returns the difference between innovations in both genes
double Genome_Disjoint(genome_t *x, genome_t *y)
{
	vector *innov1 = vector_init(), *innov2 = vector_init();

	for (unsigned int i = 0; i < x->genes->count; i++)
	{
		gene_t *gene = x->genes->data[i];
		vector_insert(innov1, gene->innovation_num, (void*)1);
	}

	for (unsigned int i = 0; i < y->genes->count; i++)
	{
		gene_t *gene = y->genes->data[i];
		vector_insert(innov2, gene->innovation_num, (void*)1);
	}

	// Check for the number of disjointed genes; genes whose innovations are unique to a single genome.
	int disjointGenes = 0;
	for (unsigned int i = 0; i < x->genes->count; i++)
	{
		gene_t *gene = x->genes->data[i];
		if (innov2->count <= gene->innovation_num || innov2->data[gene->innovation_num] != (void *)1)
			disjointGenes++;
	}

	for (unsigned int i = 0; i < y->genes->count; i++)
	{
		gene_t *gene = y->genes->data[i];
		if (innov1->count <= gene->innovation_num || innov1->data[gene->innovation_num] != (void *)1)
			disjointGenes++;
	}

	vector_free_all(innov1);
	vector_free_all(innov2);

	return (disjointGenes / fmax(x->genes->count, y->genes->count));
}

// Returns the difference in weights between both genes.
double Genome_Weights(genome_t *x, genome_t *y)
{
	vector *innov2 = vector_init();

	for (unsigned int i = 0; i < y->genes->count; i++)
	{
		gene_t *gene = y->genes->data[i];
		vector_insert(innov2, gene->innovation_num, gene);
	}

	double sum = 0.0;
	double coincident = 0.0;
	for (unsigned int i = 0; i < x->genes->count; i++)
	{
		gene_t *gene = x->genes->data[i];
		if (gene->innovation_num < innov2->count && innov2->data[gene->innovation_num] != 0)
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

unsigned int Genome_Extrons(genome_t *genome)
{
	unsigned int total = 0;
	for (unsigned int i = 0; i < genome->genes->count; i++)
		if (((gene_t*)genome->genes->data[i])->enabled)
			total++;
	return total;
}

void Genome_Add_Gene(genome_t *genome, vector *glist, gene_t *g)
{
	for (unsigned int i = 0; i < glist->count; i++)
	{
		gene_t* curgene = glist->data[i];
		if (curgene->innovation_num >= g->innovation_num)
			vector_insert(glist, i, g);
	}
}

cbool Genome_Quicksort_By_Fitness(genome_t *x, genome_t *y)
{
	return (x->fitness <= y->fitness);
}