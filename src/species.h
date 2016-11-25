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
#ifndef __SPECIES_H__
#define __SPECIES_H__

#include "genome.h"

// Forward declaration of population_t.
typedef struct population_s population_t;

typedef struct species_s
{
	unsigned int age; // How old is the species? 

	double ave_fitness; // Average fitness of the species.
	double max_fitness; // Max fitness of the species in its current evolution.
	double peak_fitness; // Max fitness of the species since its creation.

	vector* genomes; // Contains: genome_t*. Vector of genome organisms within the species.

	unsigned char staleness;  //If this is too long ago, the Species will goes extinct
} species_t; // Collection of Genomes that hold a specific trait.

// Base Constructor
species_t* Species_Init();

// Class Deconstructor, also frees memory.
void Species_Delete(species_t* species);

// Saves some species data to the file as a comment.
cbool Species_Save(species_t* species, unsigned int id, FILE* f);

// Perform mating and mutation within the entire species to form the next generation.
cbool Species_Reproduce(species_t *species, population_t* pop, unsigned int offspring_count, vector *children);

// Perform mating and mutation on a single genome and return it.
genome_t* Species_Reproduce_Single(species_t* species, population_t *pop);

// Averages out the fitnesses of the genomes within a species.
double Species_Compute_Average_Fitness(species_t *species);

//Quicksort function for species by max fitness.
cbool Species_Quicksort_By_Fitness(species_t *x, species_t *y);

#endif // !__SPECIES_H__