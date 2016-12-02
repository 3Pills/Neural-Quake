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
#include "quakedef.h"

gene_t* Gene_Init(double w, unsigned short inode, unsigned short onode, unsigned int innov)
{
	gene_t* gene = (gene_t*)malloc(sizeof(gene_t));
	if (gene == 0) return NULL;

	gene->weight = w;
	gene->inode = inode;
	gene->onode = onode;
	gene->innovation_num = innov;
	gene->enabled = true;

	return gene;
}

// Duplicate a gene from another existing gene.
gene_t* Gene_Init_Copy(gene_t* g)
{
	gene_t* gene = Gene_Init(g->weight, g->inode, g->onode, g->innovation_num);
	gene->enabled = g->enabled;

	return gene;
}

gene_t* Gene_Init_Load(char *argline)
{
	double weight;
	int inode_id;
	int onode_id;
	int innovation_num;
	int enabled;

	char wordbuf[1024];
	strcpy(wordbuf, argline);

	// Read the line into memory.
	char *curword = strtok(wordbuf, "\t\n");

	// Error handling.
	if (curword == NULL)
	{
		Con_Printf("Error loading inode from gene!\n");
		return 0;
	}

	// Convert each argument to its decimal equivalent.
	sscanf(curword, "%d", &inode_id);

	curword = strtok(NULL, "\t\n");
	if (curword == NULL)
	{
		Con_Printf("Error loading onode from gene between node #%d!\n", inode_id);
		return 0;
	}
	sscanf(curword, "%d", &onode_id);

	curword = strtok(NULL, "\t\n");
	if (curword == NULL)
	{
		Con_Printf("Error loading weight from gene between node #%d and node #%d!\n", inode_id, onode_id);
		return 0;
	}
	sscanf(curword, "%lf", &weight);

	curword = strtok(NULL, "\t\n");
	if (curword == NULL)
	{
		Con_Printf("Error loading innovation_num from gene between node #%d and node #%d!\n", inode_id, onode_id);
		return 0;
	}
	sscanf(curword, "%d", &innovation_num);

	curword = strtok(NULL, "\t\n");
	if (curword == NULL)
	{
		Con_Printf("Error loading enabled from gene between node #%d and node #%d!\n", inode_id, onode_id);
		return 0;
	}
	sscanf(curword, "%d", &enabled);

	// Apply direct parameters.
	gene_t* gene = Gene_Init(weight, inode_id, onode_id, innovation_num);
	gene->enabled = (enabled == 1);

	return gene;
}

void Gene_Delete(gene_t* gene)
{
	free(gene);
}

// Print gene data to a file. Called from Genome_Save.
void Gene_Save(gene_t* gene, FILE *f)
{
	if (f == NULL) return;

	fprintf(f, "%d\t%d\t%f\t%d\t%d\n", gene->inode, gene->onode, gene->weight, gene->innovation_num, (gene->enabled) ? 1 : 0);
}

// Check if a gene already exists within a vector,
// i.e. there is a gene with the same inode and onode.
cbool Gene_Is_Within(gene_t *gene, vector *genes)
{
	for (unsigned int i = 0; i < genes->count; i++) 
	{
		gene_t *checkedgene = genes->data[i];

		//Check if they share the same node IDs.
		if (gene->inode == checkedgene->inode && 
			gene->onode == checkedgene->onode)
			return true;
	}
	return false;
}

// Quicksort function for genes in a genome.
cbool Gene_Quicksort_By_OutputID(gene_t* x, gene_t* y)
{
	return(x->onode >= y->onode);
}