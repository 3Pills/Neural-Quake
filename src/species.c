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

species_t Species_Init(int i)
{
	species_t species;

	return species;
}

species_t Species_Init_Frozen(int i, cbool n)
{
	species_t species;

	return species;
}

void Species_Delete(void)
{

}

organism_t* Species_First(species_t* species)
{
	return (species->organisms.data[0]);
}
organism_t* Species_Champion(species_t* species)
{
	return (species->organisms.data[0]);
}

cbool Species_Rank_Organisms(species_t *species)
{
	return false;
}

cbool Species_Remove_Organism(species_t *species, organism_t *organism)
{
	return false;
}

void Species_Adjust_Fitness(species_t *species)
{

}

double Species_Compute_Max_Fitness(species_t *species)
{
	return 0.00;
}
double Species_Compute_Average_Fitness(species_t *species)
{
	return 0.00;
}

double Species_Count_Offspring(species_t *species)
{
	return 0.00;
}