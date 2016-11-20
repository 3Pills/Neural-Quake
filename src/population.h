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
#ifndef __POPULATION_H__
#define __POPULATION_H__

#include "species.h"

typedef struct population_s
{
	vector* organisms; // Contains: organism_t. Vector array of organisms in the population.
	vector* species;  // Contains: species_t. Vector array of species in the Population. Note that the species should comprise all the genomes 
	vector* innovations; // Contains: innovations_t. Vector array of generation innovations.

	// ******* Member variables used during reproduction *******
	int cur_node_id;  //Current label number available
	double cur_innov_num;

	int last_species;  //The highest species number

	// ******* Fitness Statistics *******
	double mean_fitness;
	double variance;
	double standard_deviation;

	int winnergen; //An integer that when above zero tells when the first winner appeared

	// ******* When do we need to delta code? *******
	double highest_fitness;  //Stagnation detector
	int highest_last_changed; //If too high, leads to delta coding
} population_t;

// Construct off of a single spawning Genome 
population_t *Population_Init(genome_t *g, int size);

// Construct off of a single spawning Genome without mutation
population_t *Population_Init_No_Mutation(genome_t *g, int size, float power);

// Construct off of a vector of genomes with a mutation rate of "power"
population_t *Population_Init_From_List(vector *genomeList, float power);

// Construct from file data.
population_t *Population_Init_Load(FILE *f);

// Deconstructor
void Population_Delete(population_t *pop);

// Spawn population from a single genome. 
cbool Population_Spawn(population_t *pop, genome_t *g, int size);

// Seperate organisms into species.
cbool Population_Speciate(population_t *pop);

// Run verify on all genomes in this population
cbool Population_Verify(population_t *pop);

// Clone population from a genome.
cbool Population_Clone(population_t *pop, genome_t *g, int size, float power);

// Compute the sparseness of a genome from other genomes in the population.
double Population_Compute_Sparseness(genome_t *genome);

// Turnover the population to a new generation using fitness 
// The generation argument is the next generation
cbool Population_Epoch(population_t *pop, int generation);

// Places the organisms in species in order from best to worst fitness 
cbool Population_Rank_Within_Species(population_t *pop);

// Prints population to file, by species.
cbool Population_FPrint(population_t* pop, FILE* f);

#endif //!__POPULATION_H__