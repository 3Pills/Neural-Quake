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
#include <stdlib.h>
#include <string.h>

#include "population.h"

#include "neural.h"
#include "neural_def.h"
#include "vector.h"
#include "genome.h"
#include "species.h"

population_t *Population_Init(genome_t *g, unsigned int size)
{
	population_t* pop = (population_t*)malloc(sizeof(population_t));
	if (pop == 0) return NULL;

	pop->max_fitness = 0;
	pop->generation = 1;
	pop->winner_generation = 0;
	pop->species = vector_init();
	pop->innovation = 0;

	// Spawn a population from our existing genome.
	genome_t *new_genome = 0;

	//Create size copies of the Genome. Start with perturbed linkweights.
	for (unsigned int i = 0; i < size; i++)
	{
		new_genome = Genome_Init_Copy(g);
		Genome_Mutate_Link_Weights(new_genome, 1.0, 1.0, NQ_COLDGAUSSIAN);
		Population_Speciate(pop, new_genome);
	}

	//Keep a record of the highest innovation number we are on.
	if (new_genome != 0)
		pop->innovation = Genome_Get_Last_Gene_Innovnum(new_genome);

	return pop;
}


population_t *Population_Init_Load(FILE* f)
{
	// Create an empty population to be filled from the file.
	population_t* pop = Population_Init(NULL, 0);
	genome_t *genome = 0;

	char* curword;
	char curline[1024]; //max line size of 1024 characters

	unsigned int genome_counter = 0;

	while (fgets(curline, sizeof(curline), f))
	{
		char lineCopy[1024];
		strcpy(lineCopy, curline);

		curword = strtok(lineCopy, " \t\n");
		if (curword != NULL)
		{
			if (strcmp(curword, "gn_s") == 0)
			{
				genome_counter++;
				genome = Genome_Init_Load(f, curline);
				if (genome == 0)
				{
					Con_Printf("Error loading genome #%d!\n", genome_counter);
					return pop;
				}
				Population_Speciate(pop, genome);
			}
			else if (strcmp(curword, "/*") == 0)
			{
				Con_Printf(curline);
			}
		}
	}
	
	// Determine the last innovation from the last genome loaded.
	// This will always be the last one because innovations are added as needed.
	pop->innovation = Genome_Get_Last_Gene_Innovnum(genome);

	return pop;
}

void Population_Delete(population_t *pop)
{
	for (unsigned int i = 0; i < pop->species->count; i++)
		Species_Delete(pop->species->data[i]);

	vector_free_all(pop->species);
	free(pop);
}

cbool Population_Save(population_t* pop, FILE* f)
{
	fprintf(f, "/* Neural Quake Generation [%d] */\n", pop->generation);
	Con_Printf("/* Neural Quake Generation [%d] */\n", pop->generation);

	for (unsigned int i = 0; i < pop->species->count; i++)
		Species_Save(pop->species->data[i], i+1, f);

	return true;
}

unsigned int Population_New_Innovation(population_t* pop)
{
	return ++pop->innovation;
}

cbool Population_Clone(population_t *pop, genome_t *g, unsigned int size, float power)
{
	genome_t *new_genome = Genome_Init_Copy(g);
	Population_Speciate(pop, new_genome);

	//Create size copies of the Genome. Start with perturbed linkweights.
	for (unsigned int i = 0; i < size; i++)
	{
		new_genome = Genome_Init_Copy(g);
		if (power > 0) Genome_Mutate_Link_Weights(new_genome, power, 1.0, NQ_GAUSSIAN);
		Population_Speciate(pop, new_genome);
	}

	//Keep a record of the innovation we are on
	pop->innovation = Genome_Get_Last_Gene_Innovnum(new_genome);
	return true;
}

// Adds a genome to an existing compaible species, or a new species otherwise.
void Population_Speciate(population_t *pop, genome_t *g)
{
	// Search through existing species to find a compatible one.
	for (unsigned int i = 0; i < pop->species->count; i++)
	{
		species_t *species = pop->species->data[i];
		if (Genome_Compatibility(g, species->genomes->data[0]) < NQ_COMPAT_THRESHOLD)
		{
			vector_push(species->genomes, g);
			return;
		}
	}

	// If we couldn't find a compatible species, make a new one!
	species_t *newspecies = Species_Init(pop->species->count + 1);
	vector_push(pop->species, newspecies);
	vector_push(newspecies->genomes, g);
}

double Population_Average_Fitness(population_t *pop)
{
	double total = 0.0;
	for (unsigned int i = 0; i < pop->species->count; i++)
		total += ((species_t*)pop->species->data[i])->ave_fitness;
	return total;
}

// Ranks the genomes within the population by their fitness.
void Population_Rank_Genomes(population_t *pop)
{
	pop->max_fitness = 0;

	// Add all genomes to a global list
	vector* sorted_genomes = vector_init();
	for (unsigned int i = 0; i < pop->species->count; i++)
	{
		species_t *species = pop->species->data[i];

		for (unsigned int j = 0; j < species->genomes->count; j++)
			vector_push(sorted_genomes, species->genomes->data[j]);
	}

	// Quicksort genomes by their fitness.
	Quicksort(0, sorted_genomes->count - 1, sorted_genomes->data, Genome_Quicksort_By_Fitness);

	// Store the rank on the genome.
	for (unsigned int i = 0; i < sorted_genomes->count; i++)
		((genome_t*)sorted_genomes->data[i])->global_rank = i + 1;

	genome_t *peoples_champ = sorted_genomes->data[0];

	// A world without the peoples champ... There is one out there.
	if (peoples_champ != NULL)
		pop->max_fitness = peoples_champ->fitness;

	vector_free_all(sorted_genomes);
}

// Removes the less fit genomes in a species. champ_only to remove all but the best.
void Population_Cull(population_t *pop, cbool champ_only)
{
	for (unsigned int i = 0; i < pop->species->count; i++)
	{
		species_t *species = pop->species->data[i];
		Quicksort(0, species->genomes->count - 1, species->genomes->data, Genome_Quicksort_By_Fitness);

		genome_t *genome_low = species->genomes->data[species->genomes->count-1], *genome_high = species->genomes->data[0];

		unsigned int remaining = champ_only ? 1 : ceil(species->genomes->count / 2);
		while (species->genomes->count > remaining && species->genomes->count > 0)
		{
			Genome_Delete(species->genomes->data[species->genomes->count - 1]);
			vector_pop(species->genomes);
		}

		if (species->genomes->count <= 0)
		{
			Species_Delete(species);
			vector_delete(pop->species, i);
		}
	}
}

cbool Population_Epoch(population_t *pop)
{
	// Cull lower half of species.
	Population_Cull(pop, false);

	Population_Rank_Genomes(pop);

	// Remove all stale species.
	for (unsigned int i = 0; i < pop->species->count; i++)
	{
		species_t *species = pop->species->data[i];
		if (species->genomes->count > 0)
		{
			Quicksort(0, species->genomes->count - 1, species->genomes->data, Genome_Quicksort_By_Fitness);

			genome_t *genome = species->genomes->data[0];
			if (genome->fitness > species->max_fitness)
			{
				species->max_fitness = genome->fitness;
				species->staleness = 0;
			}
			else
			{
				species->staleness++;
			}
		}

		// Kill off the species if it is old and it is not the best.
		if (species->staleness >= NQ_DROPOFF_AGE && species->max_fitness < pop->max_fitness)
		{
			vector_delete(pop->species, i);
			Species_Delete(species);
		}
	}

	Population_Rank_Genomes(pop);

	// Get the average fitness within each species, so we can
	// compare them to the fitness of the population as a whole.
	for (unsigned int i = 0; i < pop->species->count; i++)
		Species_Compute_Average_Fitness(pop->species->data[i]);
	
	// Remove weak species. These weak species are calculated by comparing 
	// the average fitness of the species agains the populations average fitness.
	double pop_ave_fitness = Population_Average_Fitness(pop);
	for (unsigned int i = 0; i < pop->species->count; i++)
	{
		species_t *species = pop->species->data[i];

		if (floor(species->ave_fitness / pop_ave_fitness * NQ_POP_SIZE) < 1)
		{
			vector_delete(pop->species, i);
			Species_Delete(species);
		}
	}

	// Contains: genome_t*. Stores bred species for new generations.
	vector *children = vector_init();

	// Get a new average based on the remaining species.
	pop_ave_fitness = Population_Average_Fitness(pop);

	// Reproduce from each species.
	for (unsigned int i = 0; i < pop->species->count; i++)
	{
		species_t *species = pop->species->data[i];

		// Calculate genomes to breed from species.
		// Compares the average fitness of the species against the population average.
		unsigned int offspring_count = floor(species->ave_fitness / pop_ave_fitness * NQ_POP_SIZE) - 1;

		// Species_Reproduce returns false if the species is empty. Remove it if it is.
		if (!Species_Reproduce(species, pop, offspring_count, children))
		{
			vector_delete(pop->species, i);
			Species_Delete(species);
		}
	}

	// Kill off every genome but the best from each species.
	Population_Cull(pop, true);

	// Make children from our champions until we have enough for a new population.
	while (children->count + pop->species->count < NQ_POP_SIZE)
		vector_push(children, Species_Reproduce_Single(pop->species->data[Random_Int(0, pop->species->count - 1)], pop, true));

	// Add our children into the species of the new generation.
	for (unsigned int i = 0; i < children->count; i++)
		Population_Speciate(pop, children->data[i]);

	vector_free_all(children);
	pop->generation++;

	return true;
}

vector *Population_Get_Sorted_Species(population_t *pop)
{
	vector *sorted_species = vector_init();
	for (unsigned int i = 0; i < pop->species->count; i++)
		vector_push(sorted_species, pop->species->data);

	Quicksort(0, sorted_species->count - 1, sorted_species->data, Species_Quicksort_By_Fitness);
	return sorted_species;
}