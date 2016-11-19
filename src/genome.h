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
#include "gene.h"
#include "innovation.h"
#include "math_vector.h"

//Describes the type of mutator to be appplied during link mutations.
enum mutator_e {
	NQ_GAUSSIAN = 0,
	NQ_COLDGAUSSIAN = 1
};

typedef struct network_s network_t;

typedef struct genome_s
{
	int id;
	int fitness; // Determined fitness of the Genome.
	vec3_t final_pos; // Final position of the genome before evaluation finished.

	//vector* traits; // Contains: trait_t*. Array of Traits within the genome.
	vector* neurons; // Contains: neuron_t*. Array of Neurons within the genome.
	vector* genes; // Contains: gene_t*. Array of Genes within the genome.

	network_t* phenotype;

} genome_t; // Network of neurons connected by genes.

//Constructor which takes full genome specs and puts them into the new one
genome_t* Genome_Init(int id, vector* nodes, vector* genes);

//Constructor which takes in links (not genes) and creates a Genome
genome_t* Genome_Init_Links(int id, vector* nodes, vector* links);

// Copy constructor
genome_t* Genome_Init_Copy(genome_t* other);

// Constructor which spawns off an input file
// Called from Population_Init_Load.
genome_t* Genome_Init_Load(int id, FILE *f);

// This special constructor creates a Genome
// with i inputs + 1 bias input, o outputs, 
// n out of nmax hidden units and random connectivity. 
// If r is true then recurrent connections will be included. 
// Linkprob is the probability of a link.
genome_t* Genome_Init_Structure(int new_id, int i, int o, int n, int nmax, cbool r, double linkprob);

//Special constructor that creates a Genome of 3 possible types:
//0 - Fully linked, no hidden nodes
//1 - Fully linked, one hidden node splitting each link
//2 - Fully connected with a hidden layer, recurrent 
//num_hidden is only used in type 2
genome_t* Genome_Init_Auto(int num_in, int num_out, int num_hidden, int type);

// Loads a new Genome from a file (doesn't require knowledge of Genome's id)
// static genome_t *Genome_New_Genome_load(char *filename);

//Destructor kills off all lists (including the trait vector)
void Genome_Delete(genome_t *genome);

//Generate a network phenotype from this Genome with specified id
network_t* Genome_Genesis(genome_t *genome, int id);

// Dump this genome to specified file
//void print_to_file(std::ostream &outFile);
//void print_to_file(std::ofstream &outFile);

// Wrapper for print_to_file above
//void print_to_filename(char *filename);

// Duplicate a Genome to create a new one with the specified id.
genome_t* Genome_Duplicate(genome_t *genome, int new_id);

// For debug: A number of tests can be run on a genome to check its integrity
// Note: Some of these tests do not indicate a bug, but rather are meant
// to be used to detect specific system states
cbool Genome_Verify(genome_t *genome);

int Genome_Get_Last_Node_ID(genome_t *genome);

double Genome_Get_Last_Gene_Innovnum(genome_t *genome); 

void Genome_Print_Genome(genome_t *genome); //Displays Genome on screen

// ******* MUTATORS *******

// Perturb params in one trait
void Genome_Mutate_Random_Trait(genome_t *genome);

// Change random link's trait. Repeats the number of times.
void Genome_Mutate_Link_Trait(genome_t *genome, int times);

// Change random node's trait. Repeats the number of times.
void Genome_Mutate_Node_Trait(genome_t *genome, int times);

// Add Gaussian noise to linkweights either GAUSSIAN or COLDGAUSSIAN (from zero)
void Genome_Mutate_Link_Weights(genome_t *genome, double power, double rate, enum mutator_e mut_type);

// toggle genes on or off 
void Genome_Mutate_Toggle_Enable(genome_t *genome, int times);

// Find first disabled gene and enable it 
void Genome_Mutate_Gene_Reenable(genome_t *genome);

// These last kinds of mutations return false if they fail
//   They can fail under certain conditions,  being unable
//   to find a suitable place to make the mutation.
//   Generally, if they fail, they can be called again if desired. 

// Mutate genome by adding a node respresentation 
cbool Genome_Mutate_Add_Node(genome_t *genome, vector *innovs, int curnode_id, double curinnov);

// Mutate the genome by adding a new link between 2 random NNodes 
cbool Genome_Mutate_Add_Link(genome_t *genome, vector *innovs, double curinnov, int tries);

void Genome_Mutate_Add_Sensor(genome_t *genome, vector *innovs, double curinnov);

// ****** MATING METHODS ***** 
// These methods are within genetic algorithm implementations.

// This method mates this Genome with another Genome g.  
//   For every point in each Genome, where each Genome shares
//   the innovation number, the Gene is chosen randomly from 
//   either parent.  If one parent has an innovation absent in 
//   the other, the baby will inherit the innovation.
//   Interspecies mating leads to all genes being inherited.
//   Otherwise, excess genes come from most fit parent.
genome_t *Genome_Mate_Multipoint(genome_t *genome, genome_t *other, int genomeid, double fitness1, double fitness2, cbool interspec_flag);

//This method mates like multipoint but instead of selecting one
//   or the other when the innovation numbers match, it averages their
//   weights 
genome_t *Genome_Mate_Multipoint_Avg(genome_t *genome, genome_t *other, int genomeid, double fitness1, double fitness2, cbool interspec_flag);

// This method is similar to a standard single point CROSSOVER
//   operator.  Traits are averaged as in the previous 2 mating
//   methods.  A point is chosen in the smaller Genome for crossing
//   with the bigger one.  
genome_t *Genome_Mate_Singlepoint(genome_t *genome, genome_t *other, int genomeid);


// ******** COMPATIBILITY CHECKING METHODS ********

// This function gives a measure of compatibility between
//   two Genomes by computing a linear combination of 3
//   characterizing variables of their compatibilty.
//   The 3 variables represent PERCENT DISJOINT GENES, 
//   PERCENT EXCESS GENES, MUTATIONAL DIFFERENCE WITHIN
//   MATCHING GENES.  So the formula for compatibility 
//   is:  disjoint_coeff*pdg+excess_coeff*peg+mutdiff_coeff*mdmg.
//   The 3 coefficients are global system parameters 
double Genome_Compatibility(genome_t *genome, genome_t *other);

// Compare the difference of params within each trait and return that difference.
// double Genome_Trait_Compare(genome_t *genome, trait_t *t1, trait_t *t2);

// Return number of non-disabled genes 
int Genome_Extrons(genome_t *genome);

// Randomize the trait pointers of all the node and connection genes 
// void Genome_Randomize_Traits(genome_t *genome);


//Inserts a NNode into a given ordered list of NNodes in order
void Genome_Node_Insert(genome_t *genome, vector *nlist, neuron_t *n);

//Adds a new gene that has been created through a mutation in the
//*correct order* into the list of genes in the genome
void Genome_Add_Gene(genome_t *genome, vector *glist, gene_t *g);

// Print genome to a file.
void Genome_FPrint(genome_t* gene, FILE *f);

#endif // !__GENOME_H__