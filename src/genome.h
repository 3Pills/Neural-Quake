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
#ifndef __GENOME_H__
#define __GENOME_H__
#include "neural_def.h"
#include "gene.h"
#include "innovation.h"

typedef struct genome_s
{
	int ID;
	int fitness; // Determined fitness of the Genome.

	vector genes; // Array of Genes.
	vector neurons; // Array of Neurons.
	vector traits; // 

} genome_t; // Network of neurons connected by genes.

#endif // !__GENOME_H__