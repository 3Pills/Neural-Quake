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
//genome.h - Contains class definition for 
//NOTE: Based heavily on Kenneth O. Stanley's C++ implementation of neural networks.

#ifndef __GENOME_H__
#define __GENOME_H__

#include "math_vector.h" //vec3_t
#include "neuron.h"

//Describes the type of mutator to be appplied during link mutations.
enum mutator_e {
	NQ_GAUSSIAN = 0,
	NQ_COLDGAUSSIAN = 1
};

// Forward declaration of network_t.
typedef struct network_s network_t;
typedef struct population_s population_t;

typedef struct genome_s
{
	unsigned int fitness; // Determined fitness of the Genome.

	unsigned short num_in; // Number of inputs within the genome.
	unsigned short num_out; // Number of outputs within the genome.

	unsigned short global_rank; // The rank of this genome compared to every other genome in the population.

	// Contains: neuron_t*. Represents the structure of nodes within the genome.
	//   Remains uninitialized until the game has begun evaluating the genome.
	vector* neurons;
	vector* genes; // Contains: gene_t*. Array of Genes within the genome.

} genome_t; // Network of neurons connected by genes.

//Constructor which takes full genome specs and puts them into the new one
genome_t* Genome_Init(vector* genes, unsigned short num_in, unsigned short num_out);

// Duplicate a Genome to create a new one with the specified id.
genome_t* Genome_Init_Copy(genome_t *genome);

// Constructor which spawns off an input file
// Called from Population_Init_Load.
genome_t* Genome_Init_Load(FILE *f, char *argline);

//Special constructor that creates a Genome of 2 possible types:
//0 - Fully linked, no hidden nodes.
//1 - Fully linked, num hidden node splitting input and output.
genome_t* Genome_Init_Auto(unsigned short num_in, unsigned short num_out, unsigned short num_hidden, unsigned char type);

//Destructor kills off all lists (including the trait vector)
void Genome_Delete(genome_t *genome);

// Print genome to a file.
void Genome_Save(genome_t* gene, FILE *f);

// Feeds values through the network and activates all outputs with values.
void Genome_Activate(genome_t* genome);

// Takes an array of sensor values and loads it into inputs ONLY
void Genome_Load_Inputs(genome_t* genome, double* sensvals);

// Generate a network within this genome's neurons vector
void Genome_Genesis(genome_t *genome);

// Clear neurons from genome.
void Genome_Clear_Nodes(genome_t *genome);

// Gets the total count of neurons within the genome from the genes.
// The neuron vector will not always represent this total count.
unsigned int Genome_Neuron_Total(genome_t *genome);

double Genome_Get_Last_Gene_Innovnum(genome_t *genome); 

// Apply various mutators randomly to a genome.
void Genome_Mutate(genome_t *genome, population_t *pop);

// Add Gaussian noise to genes.
void Genome_Mutate_Link_Weights(genome_t *genome, double power, double rate, enum mutator_e mut_type);

// Toggle genes on or off 
void Genome_Mutate_Gene_Toggle(genome_t *genome, unsigned char times);

// Find first disabled gene and enable it 
void Genome_Mutate_Gene_Enable(genome_t *genome);

// These last kinds of mutations return false if they fail
//   They can fail under certain conditions,  being unable
//   to find a suitable place to make the mutation.
//   Generally, if they fail, they can be called again if desired. 

// Mutate genome by adding a node respresentation 
cbool Genome_Mutate_Add_Node(genome_t *genome, population_t *pop);

// Mutate the genome by adding a new link between 2 random NNodes 
cbool Genome_Mutate_Add_Link(genome_t *genome, population_t *pop);

//void Genome_Mutate_Add_Sensor(genome_t *genome, vector *innovs, int curinnov);

// ****** MATING METHODS ***** 
// These methods are within genetic algorithm implementations.

// This method mates this Genome with another Genome g.  
// For every point in each Genome, where each Genome shares
// the innovation number, the Gene is chosen randomly from 
// either parent.  If one parent has an innovation absent in 
// the other, the baby will inherit the innovation.
// Interspecies mating leads to all genes being inherited.
// Otherwise, excess genes come from most fit parent.
genome_t *Genome_Mate_Multipoint(genome_t *x, genome_t *y);

// This method mates like multipoint but instead of selecting one
// or the other when the innovation numbers match, it averages their
// weights 
genome_t *Genome_Mate_Multipoint_Avg(genome_t *x, genome_t *y);

// This method is similar to a standard single point CROSSOVER
// operator. Traits are averaged as in the previous 2 mating
// methods. A point is chosen in the smaller Genome for crossing
// with the bigger one.  
genome_t *Genome_Mate_Singlepoint(genome_t *x, genome_t *y);

// This method applies a basic crossover operation between the two genomes.
genome_t *Genome_Mate_Crossover(genome_t *x, genome_t *y);

// This function gives a measure of compatibility between
// two Genomes by computing the difference in both the gene's 
// weighting and the absence of innovations within either gene.
double Genome_Compatibility(genome_t *genome, genome_t *other);

// Return number of non-disabled genes 
unsigned int Genome_Extrons(genome_t *genome);

//Inserts a NNode into a given ordered list of NNodes in order
void Genome_Node_Insert(genome_t *genome, vector *nlist, neuron_t *n);

//Adds a new gene that has been created through a mutation in the
//*correct order* into the list of genes in the genome
void Genome_Add_Gene(genome_t *genome, vector *glist, gene_t *g);

// This is used for list sorting of Organisms by fitness.. highest fitness first
cbool Genome_Quicksort_By_Fitness(genome_t *x, genome_t *y);

#endif // !__GENOME_H__