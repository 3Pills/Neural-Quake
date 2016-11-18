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
#include "organism.h"
#include "neural_def.h"
#include "genome.h"
#include <string.h>
#include <stdlib.h>

organism_t* Organism_Init(double fit, genome_t* g, int gen, const char* md)
{
	organism_t* organism = malloc(sizeof(organism_t));

	organism->fitness = fit;
	organism->orig_fitness = fit;
	organism->gnome = g;
	organism->net = Genome_Genesis(organism->gnome, organism->gnome->ID);
	organism->species = 0;  //Start it in no Species
	organism->expected_offspring = 0;
	organism->generation = gen;
	organism->eliminate = false;
	organism->error = 0;
	organism->winner = false;
	organism->champion = false;
	organism->super_champ_offspring = 0;

	strncpy(organism->metadata, (md == 0) ? "" : md, 128);

	organism->time_alive = 0;

	organism->pop_champ = false;
	organism->pop_champ_child = false;
	organism->high_fit = 0;
	organism->mut_struct_baby = 0;
	organism->mate_baby = 0;

	organism->modified = true;

	return organism;
}

// Copy constructor
organism_t* Organism_Init_Copy(organism_t* o)
{
	organism_t* organism = malloc(sizeof(organism_t));

	organism->fitness = o->fitness;
	organism->orig_fitness = o->orig_fitness;
	organism->gnome = Genome_Init_Copy(o->gnome); // Associative relationship
	organism->net = Network_Init_Copy(o->net); // Associative relationship
	organism->species = o->species;	// Delegation relationship
	organism->expected_offspring = o->expected_offspring;
	organism->generation = o->generation;
	organism->eliminate = o->eliminate;
	organism->error = o->error;
	organism->winner = o->winner;
	organism->champion = o->champion;
	organism->super_champ_offspring = o->super_champ_offspring;

	strcpy(organism->metadata, o->metadata);

	organism->time_alive = o->time_alive;

	organism->pop_champ = o->pop_champ;
	organism->pop_champ_child = o->pop_champ_child;
	organism->high_fit = o->high_fit;
	organism->mut_struct_baby = o->mut_struct_baby;
	organism->mate_baby = o->mate_baby;

	organism->modified = false;
	
	return organism;
}

void Organism_Delete(organism_t* org)
{
	Genome_Delete(org->gnome);
	free(org);
}


void Organism_Update_Phenotype(organism_t* org)
{
	Network_Delete(org->net);
	org->net = Genome_Genesis(org->gnome, org->gnome->ID);
	org->modified = true;
}

// This is used for list sorting of Organisms by fitness..highest fitness first
cbool Organism_Order_Orgs(organism_t *x, organism_t *y)
{
	return (x->fitness > y->fitness);
}

cbool Organism_Order_Orgs_By_Adjusted_Fit(organism_t *x, organism_t *y)
{
	return ((x->fitness / x->species->organisms->count) > (y->fitness / y->species->organisms->count));
}

cbool Organism_FPrint(organism_t* organism, FILE* f)
{
	if (organism->modified)
		fprintf(f, "/* Organism #%d Fitness: %f Time: %d */\n", (organism->gnome)->ID, organism->fitness, organism->time_alive);
	else
		fprintf(f, "/* %s */\n", organism->metadata);

	//If it is a winner, mark it in a comment
	if (organism->winner) fprintf(f, "/* ##------$ WINNER %d SPECIES #%d $------## */\n", organism->gnome->ID, organism->species->id);

	Genome_FPrint(organism->gnome, f);

	return true;
}