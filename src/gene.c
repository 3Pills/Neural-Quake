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
#include "gene.h"
#include "neuron.h"
#include "link.h"
#include "stdlib.h"
#include "neural_def.h"

gene_t* Gene_Init(double w, neuron_t* in_node, neuron_t* out_node, cbool recurring, double innov, double mnum) 
{
	gene_t* gene = malloc(sizeof(gene_t));

	gene->link = Link_Init(w, in_node, out_node, recurring);
	gene->innovation_num = innov;
	gene->mutation_num = mnum;
	gene->enabled = true;
	gene->frozen = false;

	return gene;
}

// Construct a gene with a trait.
//gene_t* Gene_Init_Trait(trait_t* trait, double w, neuron_t* in_node, neuron_t* out_node, cbool recurring, double innov, double mnum)
gene_t* Gene_Init_Trait(double w, neuron_t* in_node, neuron_t* out_node, cbool recurring, double innov, double mnum)
{
	gene_t* gene = malloc(sizeof(gene_t));

	gene->link = Link_Init_Trait(w, in_node, out_node, recurring);
	gene->innovation_num = innov;
	gene->mutation_num = mnum;
	gene->enabled = true;
	gene->frozen = false;

	return gene;
}

gene_t* Gene_Init_Dupe(gene_t *g, neuron_t *inode, neuron_t *onode)
{
	gene_t* gene = malloc(sizeof(gene_t));

	gene->link = Link_Init_Trait(g->link->weight, inode, onode, g->link->recurrent);
	gene->innovation_num = g->innovation_num;
	gene->mutation_num = g->mutation_num;
	gene->enabled = g->enabled;

	gene->frozen = g->frozen;

	return gene;
}

// Duplicate a gene from another existing gene.
gene_t* Gene_Init_Copy(gene_t* g)
{
	gene_t* gene = malloc(sizeof(gene_t));

	gene->link = Link_Init_Copy(g->link);
	gene->innovation_num	= g->innovation_num;
	gene->mutation_num		= g->mutation_num;
	gene->enabled			= g->enabled;
	gene->frozen			= g->frozen;

	return gene;
}


gene_t* Gene_Init_File(const char *argline, vector *traits, vector *nodes)
{
	gene_t* gene = malloc(sizeof(gene_t));
	return gene;
}

/*
gene_t Gene_Init(const char *argline, vector traits, vector nodes)
{
	gene_t gene;

	//Gene parameter holders
	int traitnum;
	int inodenum;
	int onodenum;
	neuron_t *inode;
	neuron_t *onode;
	double weight;
	int recur;
	Trait *traitptr;

	std::vector<Trait*>::iterator curtrait;
	std::vector<NNode*>::iterator curnode;

	//Get the gene parameters

	std::stringstream ss(argline);

	ss >> traitnum >> inodenum >> onodenum >> weight >> recur >> innovation_num >> mutation_num >> enable;
	//std::cout << traitnum << " " << inodenum << " " << onodenum << " ";
	//std::cout << weight << " " << recur << " " << innovation_num << " ";
	//std::cout << mutation_num << " " << enable << std::endl;

	frozen = false; //TODO: MAYBE CHANGE

	//Get a pointer to the trait
	if (traitnum == 0) traitptr = 0;
	else {
		curtrait = traits.begin();
		while (((*curtrait)->trait_id) != traitnum)
			++curtrait;
		traitptr = (*curtrait);
	}

	//Get a pointer to the input node
	curnode = nodes.begin();
	while (((*curnode)->node_id) != inodenum)
		++curnode;
	inode = (*curnode);

	//Get a pointer to the output node
	curnode = nodes.begin();
	while (((*curnode)->node_id) != onodenum)
		++curnode;
	onode = (*curnode);

	lnk = new Link(traitptr, weight, inode, onode, recur);

	return gene;
}
*/

void Gene_Delete(gene_t* gene)
{
	Link_Delete(gene->link);
	free(gene);
}

// Print gene to a file. Called from Genome.
void Gene_Print(FILE *filePointer)
{

}
