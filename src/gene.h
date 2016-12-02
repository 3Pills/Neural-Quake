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
#include "environment.h"
#include "vector.h"
#include "neural_def.h"

//Type definitions for our neural network.
typedef struct gene_s
{
	unsigned short inode; // Input neuron index within genome.
	unsigned short onode; // Output neuron index within genome.

	double weight; // The weight between node values within the network.

	cbool enabled; // Flag to disable weight processing.

	// The creation number of the gene within the population.
	// Used for finding corresponding genes during crossover.
	unsigned int innovation_num; 
	//double mutation_num; // Used to see how much mutation has changed the link.

} gene_t; // Defines the connection between neurons.

// Construct a gene without a trait.
gene_t* Gene_Init(double w, unsigned short inode, unsigned short onode, unsigned int innov);

// Construct a duplicate gene from an existing gene.
gene_t* Gene_Init_Copy(gene_t* g);

// Construct a gene from a line of data. This line will be loaded froma file.
gene_t* Gene_Init_Load(char *argline);

// Gene Deconstructor. Also frees memory
void Gene_Delete(gene_t* gene);

// Print gene data to a file. Called from Genome_Save.
void Gene_Save(gene_t* gene, FILE *f);

// Check if a gene already exists within a vector,
// i.e. there is a gene with the same inode and onode.
cbool Gene_Is_Within(gene_t *gene, vector *genes);

// Quicksort function for genes in a genome.
cbool Gene_Quicksort_By_OutputID(gene_t* x, gene_t* y);

#endif // !__GENE_H__