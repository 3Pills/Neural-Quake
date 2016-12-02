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
	vector* species; // Contains: species_t*. Vector array of species in the population.

	unsigned int innovation; // The current innovation number of the population. Increments whenever an innovation is made.
	unsigned int generation; // The current generation of the population, starting at 
	unsigned int winner_generation; // The winning generation. Set when the player completes the level for the first time.
	
	double max_fitness; // The maximum fitness that has been achieved by the genomes within the population.
} population_t;

// Construct off of a single spawning Genome 
population_t *Population_Init(genome_t *g, unsigned int size);

// Construct from file data.
population_t *Population_Init_Load(FILE *f);

// Deconstructor
void Population_Delete(population_t *pop);

// Increments innovation number and returns the value.
unsigned int Population_New_Innovation(population_t *pop);

// Adds a genome to an existing compaible species, or a new species otherwise.
void Population_Speciate(population_t *pop, genome_t *genome);

// Clone population from a genome.
cbool Population_Clone(population_t *pop, genome_t *g, unsigned int size, float power);

// Compute the average fitness of every species in the population.
double Population_Average_Fitness(population_t *pop);

// Ranks the genomes within the population by their fitness.
void Population_Rank_Genomes(population_t *pop);

// Removes the less fit genomes in a species. champ_only to remove all but the best.
void Population_Cull(population_t *pop, cbool champ_only);

// Turnover the population to a new generation using fitness.
cbool Population_Epoch(population_t *pop);

// Prints population to file, by species.
cbool Population_Save(population_t* pop, FILE* f);

vector *Population_Get_Sorted_Species(population_t *pop);

#endif //!__POPULATION_H__