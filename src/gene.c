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

gene_t* Gene_Init(double w, neuron_t* inode, neuron_t* onode, double innov, double mnum)
{
	gene_t* gene = malloc(sizeof(gene_t));
	if (gene == 0) return NULL;

	gene->inode = inode;
	gene->onode = onode;
	gene->innovation_num = innov;
	gene->mutation_num = mnum;
	gene->enabled = true;

	return gene;
}

gene_t* Gene_Init_Dupe(gene_t *g, neuron_t *inode, neuron_t *onode)
{
	gene_t* gene = malloc(sizeof(gene_t));
	if (gene == 0) return NULL;

	gene->inode = inode;
	gene->onode = onode;
	gene->weight = g->weight;
	gene->innovation_num = g->innovation_num;
	gene->mutation_num = g->mutation_num;
	gene->enabled = g->enabled;

	return gene;
}

// Duplicate a gene from another existing gene.
gene_t* Gene_Init_Copy(gene_t* g)
{
	gene_t* gene = malloc(sizeof(gene_t));
	if (gene == 0) return NULL;

	gene->inode				= g->inode;
	gene->onode				= g->onode;
	gene->innovation_num	= g->innovation_num;
	gene->mutation_num		= g->mutation_num;
	gene->enabled			= g->enabled;

	return gene;
}

gene_t* Gene_Init_Load(char *argline, vector *nodes)
{
	char *curword;

	int inode_id;
	int onode_id;
	double weight;
	int innovation_num;
	double mutation_num;
	cbool enabled;

	char wordbuf[1024];
	strcpy(wordbuf, argline);

	// Read the line into memory.
	curword = strtok(wordbuf, " \n");

	// n denotes node information, and should always be present as the first word.
	if (strcmp(curword, "g") != 0)
	{
		Con_Printf("Erroneus argline passed to gene [%s]!", argline);
		return 0;
	}

	// Read the next word.
	curword = strtok(NULL, " \n");

	// Error handling.
	if (curword == NULL)
	{
		Con_Printf("Error loading value from gene!\n");
		return 0;
	}

	// Convert each argument to its decimal equivalent.
	sscanf(curword, "%d", &inode_id);

	curword = strtok(NULL, " \n");
	if (curword == NULL)
	{
		Con_Printf("Error loading value from gene between node #%d!\n", inode_id);
		return 0;
	}
	sscanf(curword, "%d", &onode_id);

	curword = strtok(NULL, " \n");
	if (curword == NULL)
	{
		Con_Printf("Error loading value from gene between node #%d and node #%d!\n", inode_id, onode_id);
		return 0;
	}
	sscanf(curword, "%lf", &weight);

	curword = strtok(NULL, " \n");
	if (curword == NULL)
	{
		Con_Printf("Error loading value from gene between node #%d and node #%d!\n", inode_id, onode_id);
		return 0;
	}
	sscanf(curword, "%d", &innovation_num);

	curword = strtok(NULL, " \n");
	if (curword == NULL)
	{
		Con_Printf("Error loading value from gene between node #%d and node #%d!\n", inode_id, onode_id);
		return 0;
	}
	sscanf(curword, "%lf", &mutation_num);

	curword = strtok(NULL, " \n");
	if (curword == NULL)
	{
		Con_Printf("Error loading value from gene between node #%d and node #%d!\n", inode_id, onode_id);
		return 0;
	}
	sscanf(curword, "%d", &enabled);

	gene_t* gene = malloc(sizeof(gene_t));
	if (gene == 0) return 0;

	// Apply direct parameters.
	gene->inode = 0;
	gene->onode = 0;
	gene->innovation_num = innovation_num;
	gene->mutation_num = mutation_num;
	gene->enabled = enabled;
	gene->weight = weight;

	// Find the nodes on either side of the gene, using the ids stored in the file.
	for (int i = 0; i < nodes->count; i++)
	{
		neuron_t *curnode = nodes->data[i];
		if (curnode->id == inode_id) gene->inode = nodes->data[i];
		if (curnode->id == onode_id) gene->onode = nodes->data[i];

		if (gene->inode != 0 && gene->onode != 0) break;
	}

	return gene;
}

void Gene_Delete(gene_t* gene)
{
	free(gene);
}

// Print gene and its link data to a file. Called from Genome.
void Gene_FPrint(gene_t* gene, FILE *f)
{
	if (f == NULL) return;

	fprintf(f, "g %i %i %f %i %d %f %i\n", gene->inode->id, gene->onode->id, 
		gene->weight, gene->innovation_num, gene->mutation_num, gene->enabled);
}
