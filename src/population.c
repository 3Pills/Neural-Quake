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
#include "population.h"
#include "neural_def.h"
#include <stdlib.h>
#include "neural.h"

population_t *Population_Init(genome_t *g, int size)
{
	population_t* pop = malloc(sizeof(population_t));
	if (pop == 0) return NULL;

	pop->winnergen = 0;
	pop->highest_fitness = 0.0;
	pop->highest_last_changed = 0;

	pop->species = vector_init();
	pop->organisms = vector_init();
	pop->innovations = vector_init();

	Population_Spawn(pop, g, size);

	return pop;
}

population_t *Population_Init_No_Mutation(genome_t *g, int size, float power)
{
	population_t* pop = malloc(sizeof(population_t));
	if (pop == 0) return NULL;

	pop->winnergen = 0;
	pop->highest_fitness = 0;
	pop->highest_last_changed = 0;

	pop->species = vector_init();
	pop->organisms = vector_init();
	pop->innovations = vector_init();

	Population_Clone(pop, g, size, power);

	return pop;
}

population_t *Population_Init_From_List(vector *genomeList, float power)
{
	population_t* pop = malloc(sizeof(population_t));
	if (pop == 0) return NULL;

	pop->winnergen = 0;
	pop->highest_fitness = 0.0;
	pop->highest_last_changed = 0;

	pop->species = vector_init();
	pop->organisms = vector_init();
	pop->innovations = vector_init();

	genome_t *new_genome;
	organism_t *new_organism;

	//Create size copies of the Genome
	//Start with perturbed linkweights
	for (int i = 0; i < genomeList->count; i++)
	{
		new_genome = genomeList->data[i];
		if (power > 0)
			Genome_Mutate_Link_Weights(new_genome, power, 1.0, NQ_GAUSSIAN);
		//Genome_Randomize_Traits(new_genome);
		new_organism = Organism_Init(0.0, new_genome, 1, NULL);
		vector_add(pop->organisms, new_organism);
	}

	//Keep a record of the innovation and node number we are on
	pop->cur_node_id = Genome_Get_Last_Node_ID(new_genome);
	pop->cur_innov_num = Genome_Get_Last_Gene_Innovnum(new_genome);

	//Separate the new Population into species
	Population_Speciate(pop);

	return pop;
}

population_t *Population_Init_Load(FILE* f)
{
	population_t* pop = malloc(sizeof(population_t));
	if (pop == 0) return NULL;

	// Initialize basic values.
	pop->winnergen = 0;
	pop->highest_fitness = 0.0;
	pop->highest_last_changed = 0;

	pop->cur_node_id = 0;
	pop->cur_innov_num = 0.0;

	pop->species = vector_init();
	pop->organisms = vector_init();
	pop->innovations = vector_init();

	char* curword;
	char curline[1024]; //max line size of 1024 characters

	cbool md = false;
	char metadata[128];

	while (fgets(curline, sizeof(curline), f))
	{
		char lineCopy[1024];
		strcpy(lineCopy, curline);

		curword = strtok(lineCopy, " \n");
		if (curword != NULL)
		{
			if (strcmp(curword, "gnome_s") == 0)
			{
				int idcheck;
				sscanf(strtok(NULL, " "), "%d", &idcheck);

				// Clear metadata if we haven't gathered any.
				if (md == false) strcpy(metadata, "");
				md = false;
				genome_t *new_genome = Genome_Init_Load(idcheck, f);
				if (new_genome == 0)
				{
					Con_Printf("Error creating genome #%d!\n", idcheck);
					return pop;
				}
				vector_add(pop->organisms, Organism_Init(0.0, new_genome, 1, metadata));
			}
			else if (strcmp(curword, "/*") == 0)
			{
				strcpy(metadata, "");
				curword = strtok(NULL, " ");
				while (curword != NULL && strcmp(curword, "*/") != 0)
				{
					if (md) strncat(metadata, " ", 128 - strlen(metadata));
					strncat(metadata, curword, 128 - strlen(metadata));
					md = true;
					curword = strtok(NULL, " ");
				}
			}
		}
	}
	return pop;
}

void Population_Delete(population_t *pop)
{
	if (pop->species->count > 0)
		for (int i = 0; i < pop->species->count; i++)
			Species_Delete(pop->species->data[i]);
	else
		for (int i = 0; i < pop->organisms->count; i++)
			Organism_Delete(pop->organisms->data[i]);

	for (int i = 0; i < pop->innovations->count; i++)
		free(pop->innovations->data[i]);

	vector_free_all(pop->species);
	vector_free_all(pop->organisms);
	vector_free_all(pop->innovations);
}

cbool Population_Verify(population_t *pop)
{
	cbool verification;

	for (int i = 0; i < pop->organisms->count; i++)
		verification = Genome_Verify(((organism_t*)pop->organisms->data[i])->gnome);

	return verification;
}

cbool Population_Clone(population_t *pop, genome_t *g, int size, float power)
{
	genome_t *new_genome = Genome_Duplicate(g, 1);
	organism_t *new_organism = Organism_Init(0.0, new_genome, 1, NULL);
	vector_add(pop->organisms, new_organism);

	//Create size copies of the Genome. Start with perturbed linkweights.
	for (int i = 1; i <= size; i++)
	{
		new_genome = Genome_Duplicate(g, i);

		if (power > 0) Genome_Mutate_Link_Weights(new_genome, power, 1.0, NQ_GAUSSIAN);
		//Genome_Randomize_Traits(new_genome);

		new_organism = Organism_Init(0.0, new_genome, 1, NULL);
		vector_add(pop->organisms, new_organism);
	}

	//Keep a record of the innovation and node number we are on
	pop->cur_node_id = Genome_Get_Last_Node_ID(new_genome);
	pop->cur_innov_num = Genome_Get_Last_Gene_Innovnum(new_genome);

	Population_Speciate(pop);

	return true;
}

cbool Population_Spawn(population_t *pop, genome_t *g, int size)
{
	genome_t *new_genome;

	//Create size copies of the Genome. Start with perturbed linkweights.
	for (int i = 1; i <= size; i++)
	{
		new_genome = Genome_Duplicate(g, i);

		Genome_Mutate_Link_Weights(new_genome, 1.0, 1.0, NQ_COLDGAUSSIAN);
		//Genome_Randomize_Traits(new_genome);

		vector_add(pop->organisms, Organism_Init(0.0, new_genome, 1, NULL));
	}

	//Keep a record of the innovation and node number we are on
	pop->cur_node_id = Genome_Get_Last_Node_ID(new_genome);
	pop->cur_innov_num = Genome_Get_Last_Gene_Innovnum(new_genome);

	Population_Speciate(pop);

	return true;
}

cbool Population_Speciate(population_t *pop)
{
	int counter = 0;
	for (int i = 0; i < pop->species->count; i++)
		Species_Delete(pop->species->data[i]);

	vector_free(pop->species);

	// Seperate organisms into species based on their compatibility.
	for (int i = 0; i < pop->organisms->count; i++)
	{
		organism_t* curorg = pop->organisms->data[i];

		cbool added = false;

		// Search if the organism is compatible with existing species.
		for (int j = 0; j < pop->species->count; j++)
		{
			species_t *curspecies = pop->species->data[j];
				
			// Check for compatibility between the organisms.
			if (Genome_Compatibility(curorg->gnome, Species_First(curspecies)->gnome) < NQ_COMPAT_THRESHOLD)
			{
				Species_Add_Organism(curspecies, curorg);
				curorg->species = curspecies;
				added = true;

				break;
			}
		}

		//If we didn't find a match, create a new species.
		if (!added)
		{
			//Create the first species
			species_t *newspecies = Species_Init(pop->species->count + 1);
			vector_add(pop->species, newspecies);
			Species_Add_Organism(newspecies, curorg);
			curorg->species = newspecies; //Point organism to its species
		}
	}
	pop->last_species = pop->species->count;

	return true;
}

cbool Population_Epoch(population_t *pop, int generation)
{
	int half_pop;

	int best_species_num;  //Used in debugging to see why (if) best species dies

	//We can try to keep the number of species constant at this number
	int num_species_target = 4;
	int num_species = pop->species->count;
	double compat_mod = 0.3;  //Modify compat thresh to control speciation


	/*
	Keeping species diverse
	This commented out code forces the system to aim for 
	num_species species at all times, enforcing diversity
	This tinkers with the compatibility threshold, which
	normally would be held constant

	if (generation>1) {
	if (num_species<num_species_target)
	NEAT::compat_threshold-=compat_mod;
	else if (num_species>num_species_target)
	NEAT::compat_threshold+=compat_mod;

	if (NEAT::compat_threshold<0.3) NEAT::compat_threshold=0.3;

	}
	*/

	vector *sorted_species = vector_init();  //Species sorted by max fit org in Species

	//Stick the Species pointers into a new Species list for sorting
	for (int i = 0; i < pop->species->count; i++)
		vector_add(sorted_species, pop->species->data[i]);

	//Sort the Species by max fitness (Use an extra list to do this)
	//These need to use ORIGINAL fitness
	Quicksort(0, sorted_species->count - 1, sorted_species->data, Species_Order_By_Fitness_Orig);

	//Flag the lowest performing species over age 20 every 30 generations 
	//NOTE: THIS IS FOR COMPETITIVE COEVOLUTION STAGNATION DETECTION
	species_t *curspecies = sorted_species->data[sorted_species->count - 1];

	for (int i = sorted_species->count - 1; i >= 0 && curspecies->age < 20; i--)
		curspecies = sorted_species->data[i];

	if ((generation % 30) == 0)
		curspecies->obliterate = true;

	printf("Number of Species: %i\n", num_species);
	//printf("compat_thresh: %i\n", compat_threshold);

	//Use Species' ages to modify the objective fitness of organisms
	//in other words, make it more fair for younger species
	//so they have a chance to take hold
	//Also penalize stagnant species
	//Then adjust the fitness using the species size to "share" fitness
	//within a species.
	//Then, within each Species, mark for death 
	//those below survival_thresh*average
	for (int i = 0; i < pop->species->count; i++)
		Species_Adjust_Fitness(pop->species->data[i]);

	double total = 0.0; //Used to compute average fitness over all Organisms
	int total_organisms = pop->organisms->count;

	//Go through the organisms and add up their fitnesses to compute the overall average
	for (int i = 0; i < pop->organisms->count; i++)
		total += ((organism_t*)pop->organisms->data[i])->fitness;

	//The average modified fitness among ALL organisms
	double overall_average = total / total_organisms;
	//printf("Generation %i: overall_average = %i\n", generation, overall_average);

	//Now compute expected number of offspring for each individual organism
	for (int i = 0; i < pop->organisms->count; i++)
		((organism_t*)pop->organisms->data[i])->expected_offspring = ((organism_t*)pop->organisms->data[i])->fitness / overall_average;

	//The fractional parts of expected offspring that can be used only when 
	//they accumulate above 1 for the purposes of counting Offspring
	double skim = 0.0;
	int total_expected = 0;


	//Now add those offspring up within each Species to get the number of
	//offspring per Species
	for (int i = 0; i < pop->species->count; i++)
	{
		skim = Species_Count_Offspring((species_t*)pop->species->data[i], skim);
		total_expected += ((species_t*)pop->species->data[i])->expected_offspring;
	}

	//Need to make up for lost foating point precision in offspring assignment
	//If we lost precision, give an extra baby to the best Species
	if (total_expected<total_organisms) {
		//Find the Species expecting the most
		int max_expected = 0;
		int final_expected = 0;
		species_t *best_species;

		for (int i = 0; i < pop->species->count; i++)
		{
			curspecies = pop->species->data[i];
			if (curspecies->expected_offspring >= max_expected)
			{
				max_expected = curspecies->expected_offspring;
				best_species = curspecies;
			}
			final_expected += curspecies->expected_offspring;
		}
		//Give the extra offspring to the best species 
		++(best_species->expected_offspring);
		final_expected++;

		//If we still arent at total, there is a problem
		//Note that this can happen if a stagnant Species
		//dominates the population and then gets killed off by its age
		//Then the whole population plummets in fitness
		//If the average fitness is allowed to hit 0, then we no longer have 
		//an average we can use to assign offspring.
		if (final_expected < total_organisms)
		{
			for (int i = 0; i < pop->species->count; i++)
				((species_t*)pop->species->data[i])->expected_offspring = 0;

			best_species->expected_offspring = total_organisms;
		}
	}

	//Sort the Species by max fitness (Use an extra list to do this)
	//These need to use ORIGINAL fitness
	Quicksort(0, sorted_species->count - 1, sorted_species->data, Species_Order_By_Fitness_Orig);

	best_species_num = ((species_t*)sorted_species->data[0])->id;

	for (int i = 0; i < sorted_species->count; i++)
	{
		curspecies = sorted_species->data[i];
		//Print out for Debugging/viewing what's going on 
		printf("orig fitness of Species %i (Size %i): %f last improved %i\n", curspecies->id, curspecies->organisms->count, ((organism_t*)curspecies->organisms->data[0])->orig_fitness, (curspecies->age - curspecies->age_of_last_improvement));
	}

	//Check for Population-level stagnation
	curspecies = sorted_species->data[0];
	((organism_t*)curspecies->organisms->data[0])->pop_champ = true; //DEBUG marker of the best of pop

	if (((organism_t*)curspecies->organisms->data[0])->orig_fitness > pop->highest_fitness) 
	{
		pop->highest_fitness = ((organism_t*)curspecies->organisms->data[0])->orig_fitness;
		pop->highest_last_changed = 0;
		printf("NEW POPULATION RECORD FITNESS: %i\n", pop->highest_fitness);
	}
	else 
	{
		++pop->highest_last_changed;
		printf("%i generations since last population fitness record: %i\n", pop->highest_last_changed, pop->highest_fitness);
	}


	//Check for stagnation- if there is stagnation, perform delta-coding
	if (pop->highest_last_changed >= NQ_DROPOFF_AGE + 5) {

		//    cout<<"PERFORMING DELTA CODING"<<endl;

		pop->highest_last_changed = 0;

		half_pop = NQ_POP_SIZE/2;

		//    cout<<"half_pop"<<half_pop<<" pop_size-halfpop: "<<pop_size-half_pop<<endl;

		curspecies = sorted_species->data[0];

		((organism_t*)curspecies->organisms->data[0])->super_champ_offspring = half_pop;
		curspecies->expected_offspring = half_pop;
		curspecies->age_of_last_improvement = curspecies->age;
		
		if (sorted_species->count > 1) 
		{
			((organism_t*)curspecies->organisms->data[0])->super_champ_offspring = NQ_POP_SIZE - half_pop;
			curspecies->expected_offspring = NQ_POP_SIZE - half_pop;
			curspecies->age_of_last_improvement = curspecies->age;

			//Get rid of all species under the first 2
			for (int i = 1; i < sorted_species->count; i++)
				((species_t*)sorted_species->data[i])->expected_offspring = 0;
		}
		else 
		{
			((organism_t*)curspecies->organisms->data[0])->super_champ_offspring += NQ_POP_SIZE - half_pop;
			curspecies->expected_offspring = NQ_POP_SIZE - half_pop;
		}
	}

	//STOLEN BABIES:  The system can take expected offspring away from
	//  worse species and give them to superior species depending on
	//  the system parameter babies_stolen (when babies_stolen > 0)
	else if (NQ_BABIES_STOLEN > 0) {
		//Take away a constant number of expected offspring from the worst few species

		//Babies taken from the bad species and given to the champs
		int stolen_babies = 0;

		curspecies = 0;

		//Determine the exact number that will be given to the top three
		//They get, in order, 1/5 1/5 and 1/10 of the stolen babies

		int one_fifth_stolen = NQ_BABIES_STOLEN / 5;
		int one_tenth_stolen = NQ_BABIES_STOLEN / 10;

		int NUM_STOLEN = NQ_BABIES_STOLEN; //Number of babies to steal

		for (int i = sorted_species->count - 1; i >= 0 && stolen_babies < NUM_STOLEN; i--)
		{
			curspecies = sorted_species->data[i];

			if (curspecies->age > 5 && curspecies->expected_offspring > 2) 
			{
				//This species has enough to finish off the stolen pool
				if ((curspecies->expected_offspring - 1) >= (NUM_STOLEN - stolen_babies)) 
				{
					curspecies->expected_offspring -= (NUM_STOLEN - stolen_babies);
					stolen_babies = NUM_STOLEN;
				}
				//Not enough here to complete the pool of stolen
				else 
				{
					stolen_babies += curspecies->expected_offspring - 1;
					curspecies->expected_offspring = 1;
				}
			}
		}

		//Mark the best champions of the top species to be the super champs
		//who will take on the extra offspring for cloning or mutant cloning
		int species_index = 0;
		curspecies = sorted_species->data[species_index];

		int curr_stolen = one_fifth_stolen;

		for (int i = 0; i < 3; i++)
		{
			if (i == 2) curr_stolen = one_tenth_stolen;

			while (species_index < sorted_species->count && curspecies->age_of_last_improvement > NQ_DROPOFF_AGE)
				curspecies = sorted_species->data[++species_index];

			if (species_index < sorted_species->count && stolen_babies >= curr_stolen)
			{
				((organism_t*)(curspecies->organisms->data[0]))->super_champ_offspring = curr_stolen;
				curspecies->expected_offspring += curr_stolen;
				stolen_babies -= curr_stolen;
				curspecies = sorted_species->data[++species_index];
			}
		}

		while (species_index < sorted_species->count && curspecies->age_of_last_improvement > NQ_DROPOFF_AGE)
			curspecies = sorted_species->data[++species_index];

		while (species_index < sorted_species->count && stolen_babies > 0)
		{
			//Randomize a little which species get boosted by a super champ
			if (Random_Float() > 0.1)
			{
				// Don't take into account more than 3 stolen babies.
				int stolen_babies_clamped = fmin(stolen_babies, 3);

				((organism_t*)(curspecies->organisms->data[0]))->super_champ_offspring = stolen_babies_clamped;
				curspecies->expected_offspring += stolen_babies_clamped;
				stolen_babies -= stolen_babies_clamped;
			}

			curspecies = sorted_species->data[++species_index];

			while (species_index < sorted_species->count && curspecies->age_of_last_improvement > NQ_DROPOFF_AGE)
				curspecies = sorted_species->data[++species_index];

		}

		//If any stolen babies aren't taken, give them to species #1's champ
		if (stolen_babies > 0)
		{
			curspecies = sorted_species->data[0];
			((organism_t*)(curspecies->organisms->data[0]))->super_champ_offspring = stolen_babies;
			curspecies->expected_offspring += stolen_babies;
			stolen_babies -= stolen_babies;
		}
	}

	//Kill off all Organisms marked for death.  The remainder
	//will be allowed to reproduce.
	for (int i = 0; i < pop->organisms->count; i++)
	{
		organism_t *curorg = pop->organisms->data[i];
		if (curorg->eliminate)
		{
			Species_Remove_Organism(curorg->species, curorg);
			vector_delete(pop->organisms, i);
			Organism_Delete(curorg);
			i--;
		}
	}


	// Reproduce Species
	curspecies = pop->species->data[0];
	for (int i = 0; i < pop->species->count; i++)
	{
		curspecies = pop->species->data[i];
		int last_id = curspecies->id;
		Species_Reproduce(curspecies, generation, pop, sorted_species);

		//Set the current species to the id of the last species checked
		//The iterator must be reset because there were possibly vector insertions during reproduce
		for (int j = 0; j < pop->species->count; j++)
			if (((species_t*)pop->species->data[j])->id == last_id)
				i = j;

	}

	//Destroy and remove the old generation from the organisms and species  
	while (pop->organisms->count > 0)
	{
		organism_t *curorg = pop->organisms->data[0];
		Species_Remove_Organism(curorg->species, curorg);
		vector_delete(pop->organisms, 0);
		Organism_Delete(curorg);
	}

	//Remove all empty Species and age ones that survive
	//As this happens, create master organism list for the new generation
	int orgcount = 0;
	for (int i = 0; i < pop->species->count; i++)
	{
		curspecies = pop->species->data[i];
		if (curspecies->organisms->size == 0)
		{
			vector_delete(pop->species, i);
			Species_Delete(curspecies);
			i--;
		}
		//Age surviving Species and rebuild master Organism list: NUMBER THEM as they are added to the list
		else
		{
			//Age any Species that is not newly created in this generation
			if (curspecies->novel)
				curspecies->novel = false;
			else
				curspecies->age++;

			//Go through the organisms of the curspecies and add them to the master list
			for (int j = 0; j < curspecies->organisms->count; j++)
			{
				((organism_t*)curspecies->organisms->data[j])->gnome->id = orgcount++;
				vector_add(pop->organisms, curspecies->organisms->data[j]);
			}
		}
	}

	//Remove the innovations of the current generation
	vector_free(pop->innovations);

	return true;
}

cbool Population_Rank_Within_Species(population_t *pop)
{
	for (int i = 0; i < pop->species->count; i++)
		Species_Rank(pop->species->data[i]);

	return true;
}

cbool Population_FPrint(population_t* pop, FILE* f)
{
	for (int i = 0; i < pop->species->count; i++)
		Species_FPrint(pop->species->data[i], f);

	return true;
}