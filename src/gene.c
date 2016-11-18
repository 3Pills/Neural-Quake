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
#include "neural_def.h"
#include "quakedef.h"

gene_t* Gene_Init(double w, neuron_t* in_node, neuron_t* out_node, cbool recurring, double innov, double mnum) 
{
	gene_t* gene = malloc(sizeof(gene_t));
	if (gene == 0) return NULL;

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
	if (gene == 0) return NULL;

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
	if (gene == 0) return NULL;

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
	if (gene == 0) return NULL;

	gene->link = Link_Init_Copy(g->link);
	gene->innovation_num	= g->innovation_num;
	gene->mutation_num		= g->mutation_num;
	gene->enabled			= g->enabled;
	gene->frozen			= g->frozen;

	return gene;
}

gene_t* Gene_Init_Load(char *argline, vector *nodes)
{
	char *curword;

	// Args : inode_id, onode_id, weight, recurrent, innovation_num, mutation_num, enabled.
	int args[7] = { -1, -1, -1, -1, -1, -1, -1 };

	// Read the line into memory.
	curword = strtok(argline, " ");

	// n denotes node information, and should always be present as the first word.
	if (curword != "n")
	{
		Con_Printf("Erroneus argline passed to gene [%s]!", argline);
		return 0;
	}

	for (int i = 0; i < 7; i++)
	{
		// Read the next word.
		curword = strtok(NULL, " ");

		// Error handling.
		if (curword == NULL)
		{
			Con_Printf("Error loading gene between node #%s and node #%s!", args[1], args[2]);
			return 0;
		}

		// Convert each argument to its decimal equivalent.
		sscanf(curword, "%d", &args[i]);
	}

	gene_t* gene = malloc(sizeof(gene_t));
	if (gene == 0) return 0;

	// Apply direct parameters.
	gene->innovation_num = args[4];
	gene->mutation_num = args[5];
	gene->enabled = args[6];
	gene->frozen = false;

	// Find the nodes on either side of the gene, using the ids stored in the file.
	neuron_t *inode = 0, *onode = 0;
	for (int i = 0; i < nodes->count; i++)
	{
		neuron_t *curnode = nodes->data[i];
		if (curnode->node_id == args[0]) inode = nodes->data[i];
		if (curnode->node_id == args[1]) onode = nodes->data[i];

		if (inode != 0 && onode != 0) break;
	}

	// Now we can make a link.
	gene->link = Link_Init(args[2], inode, onode, args[3]);

	return gene;
}

void Gene_Delete(gene_t* gene)
{
	Link_Delete(gene->link);
	free(gene);
}

// Print gene and its link data to a file. Called from Genome.
void Gene_FPrint(gene_t* gene, FILE *f)
{
	if (f == NULL) return;

	fprintf(f, "g %i %i %f %i %f %f %i\n", gene->link->inode->node_id, gene->link->onode->node_id, 
		gene->link->weight, gene->link->recurrent, gene->innovation_num, gene->mutation_num, gene->enabled);
}
