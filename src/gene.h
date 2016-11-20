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
//gene.h - Contains class definition for genetic data stored on neural links.
//NOTE: Based heavily on Kenneth O. Stanley's C++ implementation of neural networks.

#ifndef __GENE_H__
#define __GENE_H__

#include <stdio.h>
#include "innovation.h"
#include "vector.h"

//Type definitions for our neural network.
typedef struct gene_s
{
	int inode; // Input neuron index within genome.
	int onode; // Output neuron index within genome.

	double weight;

	cbool enabled; // Disables the gene.

	int innovation_num; // Used for finding corresponding genes during crossover.
	double mutation_num; // Used to see how much mutation has changed the link.

} gene_t; // Defines the connection between neurons.

// Construct a gene without a trait.
gene_t* Gene_Init(double w, int inode, int onode, double innov, double mnum);

//Construct a gene off of another gene as a duplicate
gene_t* Gene_Init_Dupe(gene_t *g, int inode, int onode);

// Duplicate a gene from another existing gene.
gene_t* Gene_Init_Copy(gene_t* g);

// Construct a gene from a file spec given traits and nodes.
gene_t* Gene_Init_Load(char *argline, vector *nodes);

// Delete a gene
void Gene_Delete(gene_t* gene);

// Print gene data to a file. Called from Genome_FPrint.
void Gene_FPrint(gene_t* gene, FILE *f);

#endif // !__GENE_H__