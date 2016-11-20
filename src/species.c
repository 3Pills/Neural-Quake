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
#include "species.h"

#include "neural.h"
#include "neural_def.h"
#include "vector.h"
#include "organism.h"
#include "population.h"
#include "network.h"

#include <math.h>
#include <stdlib.h>

species_t* Species_Init(int i)
{
	species_t* species = malloc(sizeof(species_t));

	species->id = i;
	species->age = 1;
	species->ave_fitness = 0.0;
	species->expected_offspring = 0;
	species->novel = false;
	species->age_of_last_improvement = 0;
	species->max_fitness = 0;
	species->peak_fitness = 0;
	species->obliterate = false;
	species->average_est = 0;
	species->organisms = vector_init();

	return species;
}

species_t* Species_Init_Novel(int i, cbool n)
{
	species_t* species = malloc(sizeof(species_t));

	species->id = i;
	species->age = 1;
	species->ave_fitness = 0.0;
	species->expected_offspring = 0;
	species->novel = n;
	species->age_of_last_improvement = 0;
	species->max_fitness = 0;
	species->peak_fitness = 0;
	species->obliterate = false;
	species->average_est = 0;
	species->organisms = vector_init();

	return species;
}

void Species_Delete(species_t* species)
{
	for (int i = 0; i < species->organisms->count; ++i)
		Organism_Delete(species->organisms->data[i]);

	vector_free_all(species->organisms);
	free(species);
}

cbool Species_Rank(species_t *species)
{
	if (species->organisms->count <= 1) return true;

	Quicksort(0, species->organisms->count - 1, species->organisms->data, Species_Order_By_Fitness);

	return true;
}

cbool Species_Add_Organism(species_t* species, organism_t *organism)
{
	vector_add(species->organisms, organism);
	return true;
}

organism_t* Species_First(species_t* species)
{
	return (species->organisms->data[0]);
}

organism_t* Species_Champion(species_t* species)
{
	double champ_fitness = -1.0;
	organism_t* peoplesChamp; // What's cookin?

	for (int i = 0; i < species->organisms->count; ++i)
	{
		organism_t* curOrg = (organism_t*)species->organisms->data[i];
		
		//TODO: Remove DEBUG code
		//cout<<"searching for champ...looking at org "<<(*curorg)->gnome->genome_id<<" fitness: "<<(*curorg)->fitness<<endl;
		if (curOrg->fitness > champ_fitness) 
		{
			peoplesChamp = curOrg;
			champ_fitness = peoplesChamp->fitness;
		}
	}

	//cout<<"returning champ #"<<thechamp->gnome->genome_id<<endl;

	return peoplesChamp;
}

cbool Species_Remove_Organism(species_t *species, organism_t *organism)
{
	for (int i = 0; i < species->organisms->count; i++)
		if (species->organisms->data[i] == organism) 
			vector_delete(species->organisms, i);

	return false;
}

void Species_Adjust_Fitness(species_t *species)
{
	int age_debt = (species->age - species->age_of_last_improvement + 1) - NQ_DROPOFF_AGE;

	//std::cout<<"Species "<<id<<" last improved "<<(age-age_of_last_improvement)<<" steps ago when it moved up to "<<max_fitness_ever<<std::endl;

	if (age_debt == 0) age_debt = 1;

	for (int i = 0; i < species->organisms->count; i++)
	{
		organism_t* organism = species->organisms->data[i];
		//Remember the original fitness before it gets modified
		organism->orig_fitness = organism->fitness;

		//Make fitness decrease after a stagnation point dropoff_age
		//Added an if to keep species pristine until the dropoff point
		//obliterate is used in competitive coevolution to mark stagnation
		//by obliterating the worst species over a certain age
		if ((age_debt >= 1) || species->obliterate) {

			//Possible graded dropoff
			//organism->fitness = organism->fitness *(-atan(age_debt));

			//Extreme penalty for a long period of stagnation (divide fitness by 100)
			organism->fitness = organism->fitness * 0.01;
		}

		//Give a fitness boost up to some young age (niching)
		//The age_significance parameter is a system parameter
		//  if it is 1, then young species get no fitness boost
		if (species->age <= 10) (organism->fitness) = (organism->fitness) * NQ_AGE_SIGNIFICANCE;

		//Do not allow negative fitness
		if ((organism->fitness) < 0.0) organism->fitness = 0.0001;

		//Share fitness with the species
		organism->fitness = (organism->fitness) / (species->organisms->count);

	}

	//Sort the population and mark for death those after survival_threshold * pop_size
	Species_Rank(species);

	organism_t* first = species->organisms->data[0];
	first->champion = true; // The first will be the best, mark it has such.

	// Update age_of_last_improvement here
	if (first->orig_fitness > species->peak_fitness) {
		species->age_of_last_improvement = species->age;
		species->peak_fitness = first->orig_fitness;
	}

	//Decide how many get to reproduce based on survival_thresh * pop_size
	//Adding 1.0 ensures that at least one will survive
	int num_parents = (int)floor((NQ_SURVIVAL_THRESHOLD*((double)species->organisms->count)) + 1.0);

	int threshold_index = 0; // Index of survival threshold; organisms after will not survive.

	// Find the threshold index at which to start killing off organisms.
	for (int i = 1; i <= num_parents; i++) 
		if (i < species->organisms->count)
			threshold_index = i;

	// Mark for death those who are ranked too low to be parents.
	for (int i = threshold_index; i < species->organisms->count; i++)
		((organism_t*)species->organisms->data[i])->eliminate = true;
}

double Species_Compute_Max_Fitness(species_t *species)
{
	double max = 0.00;
	for (int i = 0; i < species->organisms->count; i++)
		if (max < ((organism_t*)species->organisms->data[i])->fitness) 
			max = ((organism_t*)species->organisms->data[i])->fitness;

	return max;
}

double Species_Compute_Average_Fitness(species_t *species)
{
	double total = 0.00;


	for (int i = 0; i < species->organisms->count; i++)
	{
		total += ((organism_t*)species->organisms->data[i])->fitness;
	}

	species->ave_fitness = total / species->organisms->count;

	return species->ave_fitness;
}

double Species_Count_Offspring(species_t *species, double skim)
{
	int e_o_intpart;  //The floor of an organism's expected offspring
	double e_o_fracpart; //Expected offspring fractional part
	double skim_intpart;  //The whole offspring in the skim

	species->expected_offspring = 0;
	for (int i = 0; i < species->organisms->count; i++)
	{
		organism_t *curorg = species->organisms->data[i];
		e_o_intpart = floor(curorg->expected_offspring);
		e_o_fracpart = fmod(curorg->expected_offspring, 1.0);

		species->expected_offspring += e_o_intpart;

		skim += e_o_fracpart;

		if (skim > 1.0)
		{
			skim_intpart = floor(skim);
			species->expected_offspring += (int)skim_intpart;
			skim -= skim_intpart;
		}
	}

	return skim;
}

genome_t *Species_Reproduce_Simple()
{

}

cbool Species_Reproduce(species_t *species, int generation, population_t* pop, vector *sorted_species)
{
	organism_t *mom; //Parent Organisms
	organism_t *dad;
	organism_t *baby;  //The new Organism

	genome_t *new_genome;  //For holding baby's genes

	species_t *randspecies;  //For mating outside the Species
	double randmult;
	int randspeciesnum;

	network_t *net_analogue;  //For adding link to test for recurrency

	cbool outside;
	cbool found;  //When a Species is found
	cbool champ_done = false; //Flag the preservation of the champion  

	cbool mut_struct_baby;
	cbool mate_baby;

	//The weight mutation power is species specific depending on its age
	double mut_power = NQ_WEIGHT_MUT_POWER;

	//Roulette wheel variables
	double total_fitness = 0.0;

	//Compute total fitness of species for a roulette wheel
	//Note: You don't get much advantage from a roulette here
	// because the size of a species is relatively small.
	// But you can use it by using the roulette code here
	//for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
	//  total_fitness+=(*curorg)->fitness;
	//}

	if (species->expected_offspring > 0 && species->organisms->count == 0) 
		return false;

	//The number of Organisms in the old generation
	int poolsize = species->organisms->count - 1;

	organism_t *peoplesChamp = species->organisms->data[0];

	//Create the designated number of offspring for the Species
	//one at a time
	for (int i = 0; i < species->expected_offspring; i++) {

		mut_struct_baby = false;
		mate_baby = false;

		outside = false;

		//If we have a super_champ (Population champion), finish off some special clones
		if (peoplesChamp->super_champ_offspring > 0) 
		{
			mom = peoplesChamp;
			new_genome = Genome_Duplicate(mom->gnome, i);

			//Most superchamp offspring will have their connection weights mutated only
			//The last offspring will be an exact duplicate of this super_champ
			//Note: Superchamp offspring only occur with stolen babies!
			//      Settings used for published experiments did not use this
			if (peoplesChamp->super_champ_offspring > 1) {
				if (Random_Float()<0.8 || NQ_MUTATE_ADD_LINK_PROB == 0.0) //Make sure no links get added when the system has link adding disabled
					Genome_Mutate_Link_Weights(new_genome, mut_power, 1.0, NQ_GAUSSIAN);
				else 
				{
					//Sometimes we add a link to a superchamp
					net_analogue = Genome_Genesis(new_genome, generation);
					Genome_Mutate_Add_Link(new_genome, pop->innovations, pop->cur_innov_num, NQ_NEWLINK_TRIES);
					Network_Delete(net_analogue);
					mut_struct_baby = true;
				}
			}

			baby = Organism_Init(0.0, new_genome, generation, NULL);

			if (peoplesChamp->super_champ_offspring == 1) 
			{
				if (peoplesChamp->pop_champ) 
				{
					baby->pop_champ_child = true;
					baby->high_fit = mom->orig_fitness;
				}
			}

			peoplesChamp->super_champ_offspring--;
		}
		//If we have a Species champion, just clone it 
		else if (!champ_done && species->expected_offspring > 5)
		{
			mom = peoplesChamp; //Mom is the peoples champ
			new_genome = Genome_Duplicate(mom->gnome, i);
			baby = Organism_Init(0.0, new_genome, generation, NULL);  //Baby is just like mommy
			champ_done = true;
		}
		//First, decide whether to mate or mutate
		//If there is only one organism in the pool, then always mutate
		else if ((Random_Float() < NQ_MUTATE_ONLY_PROB) || poolsize == 0) 
		{
			//Pick a random parent.
			mom = species->organisms->data[Random_Int(0, poolsize)];

			new_genome = Genome_Duplicate(mom->gnome, i);

			// Do the mutation depending on probabilities of various mutations
			if (Random_Float() < NQ_MUTATE_ADD_NODE_PROB)
			{
				Genome_Mutate_Add_Node(new_genome,pop->innovations, pop->cur_node_id, pop->cur_innov_num);
				mut_struct_baby = true;
			}
			else if (Random_Float() < NQ_MUTATE_ADD_LINK_PROB)
			{
				net_analogue = Genome_Genesis(new_genome, generation);
				Genome_Mutate_Add_Link(new_genome, pop->innovations, pop->cur_innov_num, NQ_NEWLINK_TRIES);
				Network_Delete(net_analogue);
				mut_struct_baby = true;
			}
			//NOTE:  A link CANNOT be added directly after a node was added because the phenotype
			//       will not be appropriately altered to reflect the change
			else {
				//If we didn't do a structural mutation, we do the other kinds

				//if (Random_Float() < NQ_MUTATE_RAND_TRAIT_PROB)
				//	Genome_Mutate_Random_Trait(new_genome);
				//if (Random_Float() < NQ_MUTATE_LINK_TRAIT_PROB)
				//	Genome_Mutate_Link_Trait(new_genome, 1);
				//if (Random_Float() < NQ_MUTATE_NODE_TRAIT_PROB)
				//	Genome_Mutate_Node_Trait(new_genome, 1);
				if (Random_Float() < NQ_MUTATE_LINK_WEIGHTS_PROB)
					Genome_Mutate_Link_Weights(new_genome, mut_power, 1.0, NQ_GAUSSIAN);
				if (Random_Float() < NQ_MUTATE_TOGGLE_ENABLE_PROB)
					Genome_Mutate_Toggle_Enable(new_genome, 20);
				if (Random_Float() < NQ_MUTATE_GENE_REENABLE_PROB)
					Genome_Mutate_Gene_Reenable(new_genome);
			}

			baby = Organism_Init(0.0, new_genome, generation, NULL);

		}

		//Otherwise we should mate 
		else {

			//Choose the random mom
			mom = species->organisms->data[Random_Int(0, poolsize)];

			//Choose random dad

			if (Random_Float() > NQ_INTERSPECIES_MATE_RATE) {
				//Mate within Species
				dad = species->organisms->data[Random_Int(0, poolsize)];
			}
			else {

				//Mate outside Species  
				randspecies = species;

				//Select a random species, giving up after 5 attempts.
				for (int j = 0; j < 5 && randspecies == species; j++)
				{
					//This old way just chose any old species
					//randspeciesnum=randint(0,(pop->species).size()-1);

					//Choose a random species tending towards better species
					randmult = Random_Gauss() / 4;
					if (randmult>1.0) randmult = 1.0;
					//This tends to select better species
					randspeciesnum = (int)floor((randmult*(sorted_species->count - 1.0)) + 0.5);
					randspecies = sorted_species->data[randspeciesnum - 1];
				}

				//OLD WAY: Choose a random dad from the random species
				//Select a random dad from the random Species
				//NOTE:  It is possible that a mating could take place
				//       here between the mom and a baby from the NEW
				//       generation in some other Species
				//orgnum=randint(0,(randspecies->organisms).size()-1);
				//curorg=(randspecies->organisms).begin();
				//for(orgcount=0;orgcount<orgnum;orgcount++)
				//  ++curorg;
				//dad=(*curorg);            

				//New way: Make dad be a champ from the random species
				dad = randspecies->organisms->data[0];

				outside = true;
			}
			
			//Perform mating based on probabilities of differrent mating types
			//if (Random_Float() < NQ_MATE_MULTIPOINT_PROB)
			//	new_genome = Genome_Mate_Multipoint(mom->gnome, dad->gnome, i, mom->orig_fitness, dad->orig_fitness, outside);
			//else if (Random_Float() < NQ_MATE_MULTIPOINT_AVG_PROB / (NQ_MATE_MULTIPOINT_AVG_PROB + NQ_MATE_SINGLEPOINT_PROB))
			//	new_genome = Genome_Mate_Multipoint_Avg(mom->gnome, dad->gnome, i, mom->orig_fitness, dad->orig_fitness, outside);
			//else
				new_genome = Genome_Mate_Singlepoint(mom->gnome, dad->gnome, i);

			mate_baby = true;

			//Determine whether to mutate the baby's Genome
			//This is done randomly or if the mom and dad are the same organism
			if (Random_Float() > NQ_MATE_ONLY_PROB || dad->gnome->id == mom->gnome->id || Genome_Compatibility(dad->gnome, mom->gnome) == 0.0)
			{

				//Do the mutation depending on probabilities of 
				//various mutations
				if (Random_Float() < NQ_MUTATE_ADD_NODE_PROB)
				{
					Genome_Mutate_Add_Node(new_genome, pop->innovations, pop->cur_node_id, pop->cur_innov_num);
					//  std::cout<<"mutate_add_node: "<<new_genome<<std::endl;
					mut_struct_baby = true;
				}
				else if (Random_Float() < NQ_MUTATE_ADD_LINK_PROB)
				{
					net_analogue = Genome_Genesis(new_genome, generation);
					Genome_Mutate_Add_Link(new_genome, pop->innovations, pop->cur_innov_num, NQ_NEWLINK_TRIES);
					Network_Delete(net_analogue);

					mut_struct_baby = true;
				}
				else {
					//Only do other mutations when not doing sturctural mutations
					//if (Random_Float() < NQ_MUTATE_RAND_TRAIT_PROB)
					//	Genome_Mutate_Random_Trait(new_genome);
					//if (Random_Float() < NQ_MUTATE_LINK_TRAIT_PROB)
					//	Genome_Mutate_Link_Trait(new_genome, 1);
					//if (Random_Float() < NQ_MUTATE_NODE_TRAIT_PROB)
					//	Genome_Mutate_Node_Trait(new_genome, 1);
					if (Random_Float() < NQ_MUTATE_LINK_WEIGHTS_PROB)
						Genome_Mutate_Link_Weights(new_genome, mut_power, 1.0, NQ_GAUSSIAN);
					if (Random_Float() < NQ_MUTATE_TOGGLE_ENABLE_PROB)
						Genome_Mutate_Toggle_Enable(new_genome, 1);
					if (Random_Float() < NQ_MUTATE_GENE_REENABLE_PROB)
						Genome_Mutate_Gene_Reenable(new_genome);
				}

				//Create the baby
				baby = Organism_Init(0.0, new_genome, generation, NULL);

			}
			else {
				//Create the baby without mutating first
				baby = Organism_Init(0.0, new_genome, generation, NULL);
			}

		}

		//Add the baby to its proper Species
		//If it doesn't fit a Species, create a new one

		baby->mut_struct_baby = mut_struct_baby;
		baby->mate_baby = mate_baby;
		if (pop->species->count == 0){
			//Create the first species			
			species_t *newspecies = Species_Init_Novel(++(pop->last_species), true);
			vector_add(pop->species, newspecies);
			Species_Add_Organism(newspecies, baby);  //Add the baby
			baby->species = newspecies;  //Point the baby to its species
		}
		else {
			found = false;
			for (int j = 0; j < pop->species->count; j++)
			{
				organism_t *comporg = Species_First(pop->species->data[j]);
				if (Genome_Compatibility(baby->gnome, comporg->gnome) < NQ_COMPAT_THRESHOLD)
				{
					Species_Add_Organism(pop->species->data[j], baby);
					baby->species = pop->species->data[j];
					found = true;
					break;
				}
			}

			//If we didn't find a match, create a new species
			if (found == false) {
				species_t *newspecies = Species_Init_Novel(++(pop->last_species), true);
				vector_add(pop->species, newspecies);
				Species_Add_Organism(newspecies, baby);  //Add the baby
				baby->species = newspecies;  //Point baby to its species
			}


		} //end else 

	}
	return true;
}

cbool Species_Order_By_Fitness(organism_t *x, organism_t *y) 
{
	return (x->fitness >= y->fitness);
}

cbool Species_Order_By_Fitness_Orig(species_t *x, species_t *y) 
{
	return (((organism_t*)x->organisms->data[0])->orig_fitness >= ((organism_t*)y->organisms->data[0])->orig_fitness);
}

cbool Species_Order_By_Fitness_Max(species_t *x, species_t *y) 
{
	return (x->max_fitness >= y->max_fitness);
}

cbool Species_FPrint(species_t* species, FILE* f)
{

	//Print a comment on the Species info
	fprintf(f, "/* Species #%d : (Size %d) (AF %f) (Age %d)  */\n\n", species->id, species->organisms->count, species->average_est, species->age);

	//Show user what's going on
	Con_Printf("/* Species #%d : (Size %d) (AF %f) (Age %d)  */\n\n", species->id, species->organisms->count, species->average_est, species->age);

	//Print all the Organisms' Genomes to the outFile
	for (int i = 0; i < species->organisms->count; i++) 
	{
		//Put the fitness for each organism in a comment
		Organism_FPrint(species->organisms->data[i], f);
	}
	fprintf(f, "\n\n");

	return true;
}