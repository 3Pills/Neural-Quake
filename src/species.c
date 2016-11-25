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
#include <math.h>
#include <stdlib.h>

#include "species.h"

#include "neural.h"
#include "neural_def.h"
#include "vector.h"
#include "population.h"

species_t* Species_Init()
{
	species_t* species = (species_t*)malloc(sizeof(species_t));

	species->age = 1;
	species->staleness = 0;

	species->ave_fitness = 0.0;
	species->max_fitness = 0.0;
	species->peak_fitness = 0.0;

	species->genomes = vector_init();

	return species;
}

void Species_Delete(species_t* species)
{
	for (unsigned int i = 0; i < species->genomes->count; ++i)
		Genome_Delete(species->genomes->data[i]);

	vector_free_all(species->genomes);
	free(species);
}

cbool Species_Save(species_t* species, unsigned int id, FILE* f)
{
	//Print a comment on the Species info
	fprintf(f, "/* Species %d : (Size %d) (Age %d) */\n\n", id, species->genomes->count, species->age);
	Con_Printf("/* Species #%d : (Size %d) (Age %d) */\n", id, species->genomes->count, species->age);

	//Print all the species' Genomes to the file
	for (unsigned int i = 0; i < species->genomes->count; i++)
		Genome_Save(species->genomes->data[i], f);

	return true;
}

cbool Species_Reproduce(species_t *species, population_t* pop, unsigned int offspring_count, vector *children)
{
	cbool champ_done = false; //Flag to preserve the one who fights for the people.

	if (species->genomes->count <= 0) return false;

	unsigned int poolsize = species->genomes->count - 1; // The number of genomes in the old generation

	// Create the designated number of offspring for the species one at a time
	for (unsigned int i = 0; i < offspring_count; i++)
	{
		genome_t *child = 0;

		//Clone our species champion to preserve the genome in a new generation.
		if (!champ_done && offspring_count > 5)
		{
			child = Genome_Init_Copy(species->genomes->data[0]);
			champ_done = true;
		}
		else
		{
			child = Species_Reproduce_Single(species, pop);
		}

		vector_push(children, child);
	}
	return true;
}

genome_t* Species_Reproduce_Single(species_t* species, population_t *pop)
{
	genome_t *child = 0;

	// Either crossover between two genomes, or just mutate a single genome.
	if (Random_Float() < NQ_MATE_CROSSOVER_PROB)
	{
		genome_t *g1 = species->genomes->data[Random_Int(0, species->genomes->count - 1)];
		genome_t *g2 = species->genomes->data[Random_Int(0, species->genomes->count - 1)];
		child = Genome_Mate_Crossover(g1, g2);
	}
	else
	{
		genome_t *g1 = species->genomes->data[Random_Int(0, species->genomes->count - 1)];
		child = Genome_Init_Copy(g1);
	}

	Genome_Mutate(child, pop);
	return child;

	/*
	//First, decide whether to mate or mutate
	//If there is only one organism in the pool, then always mutate
	if ((Random_Float() < NQ_MUTATE_ONLY_PROB) || species->genomes->count - 1 == 0)
	{
	//Pick a random parent.
	genome_t *g1 = species->genomes->data[Random_Int(0, species->genomes->count - 1)];
	child = Genome_Init_Copy(g1);

	//Mutate the child.
	Genome_Mutate(child, pop);

	return child;
	}

	// Choose one random parent
	genome_t *g1 = species->genomes->data[Random_Int(0, Random_Int(0, species->genomes->count - 1))];
	genome_t *g2 = 0;

	//Choose the other from the same species if we're mating in it.
	if (Random_Float() > NQ_INTERSPECIES_MATE_RATE)
	{
	g2 = species->genomes->data[Random_Int(0, Random_Int(0, species->genomes->count - 1))];
	}
	else
	{
	vector *sorted_species = Population_Get_Sorted_Species(pop);
	//Mate outside Species
	species_t *randspecies = species;

	//Try to select a random species, tending towards better species, giving up after 5 attempts.
	for (int i = 0; i < 5 && randspecies == species; i++)
	{
	randspecies = sorted_species->data[(int)floor((fmin(Random_Gauss() / 4, 1.0) * (sorted_species->count - 1)) + 0.5)];
	if (randspecies->genomes == NULL) randspecies = species;
	}

	//New way: Make dad be a champ from the random species
	g2 = randspecies->genomes->data[0];

	vector_free_all(sorted_species);
	}

	//Perform mating based on probabilities of differrent mating types
	if (Random_Float() < NQ_MATE_CROSSOVER_PROB)
	child = Genome_Mate_Crossover(g1, g2);
	if (Random_Float() < NQ_MATE_MULTIPOINT_PROB)
	child = Genome_Mate_Multipoint(g1, g2);
	else if (Random_Float() < NQ_MATE_MULTIPOINT_AVG_PROB / (NQ_MATE_MULTIPOINT_AVG_PROB + NQ_MATE_SINGLEPOINT_PROB))
	child = Genome_Mate_Multipoint_Avg(g1, g2);
	else
	child = Genome_Mate_Singlepoint(g1, g2);

	//if the probability is in favor or the mom and dad are the same genome then mutate.
	if (Random_Float() > NQ_MATE_ONLY_PROB || g1 == g2 || Genome_Compatibility(g1, g2) == 0.0)
	Genome_Mutate(child, pop);

	return child;
	*/
}

double Species_Compute_Average_Fitness(species_t *species)
{
	double total = 0.0;
	for (unsigned int i = 0; i < species->genomes->count; i++)
		total += ((genome_t*)species->genomes->data[i])->fitness;

	species->ave_fitness = total / species->genomes->count;
	return species->ave_fitness;
}

cbool Species_Quicksort_By_Fitness(species_t *x, species_t *y) 
{
	return (x->max_fitness <= y->max_fitness);
}