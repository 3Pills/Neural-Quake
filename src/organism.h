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
//organism.h -- Contains a genome and neural network representation. 

#ifndef __ORGANISM_H__
#define __ORGANISM_H__

#include "environment.h"
#include "genome.h"
#include "species.h"
#include "math_vector.h"

typedef struct species_s species_t;
typedef struct network_s network_t;

typedef struct organism_s
{
	double fitness; // A measure of fitness for the Organism
	double orig_fitness; // A fitness measure that won't change during adjustments
	double error; // Used just for reporting purposes
	cbool winner; // Win marker (if needed for a particular task)
	network_t* net; // The Organism's phenotype
	genome_t* gnome; // The Organism's genotype 
	species_t* species; // The Organism's Species 
	double expected_offspring; // Number of children this Organism may have
	int generation; // Tells which generation this Organism is from
	cbool eliminate; // Marker for destruction of inferior Organisms
	cbool champion; // Marks the species champ
	int super_champ_offspring;  // Number of reserved offspring for a population leader
	cbool pop_champ; // Marks the best in population
	cbool pop_champ_child; // Marks the duplicate child of a champion (for tracking purposes)
	double high_fit; // DEBUG variable- high fitness of champ
	double time_alive; // When playing in real-time allows knowing the maturity of an individual

	// Track its origin- for debugging or analysis- we can tell how the organism was born
	cbool mut_struct_baby;
	cbool mate_baby;

	// MetaData for the object
	char metadata[128];
	cbool modified;

	vec3_t final_pos; // Final position of the organism before evaluation finished
} organism_t; // Container of network. 

organism_t* Organism_Init(double fit, genome_t* genome, int gen, const char* md);

// Copy constructor
organism_t* Organism_Init_Copy(organism_t* o);

void Organism_Delete(organism_t *org);

void Organism_Update_Phenotype(organism_t *org);

// This is used for list sorting of Organisms by fitness..highest fitness first
cbool Organism_Order_Orgs(organism_t *x, organism_t *y);

cbool Organism_Order_Orgs_By_Adjusted_Fit(organism_t *x, organism_t *y);

cbool Organism_FPrint(organism_t* organism, FILE* f);

#endif // !__ORGANISM_H__