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
#ifndef __NEURON_H__
#define __NEURON_H__

#include "gene.h"

typedef struct neuron_s
{
	double value; // The Value output by the Neuron.
	vector* incoming_genes; // Contains: gene_t*. Vector array of IDs of incoming genes.
} neuron_t; // Genetic nodes which make up a neural network

// Base Constructor
neuron_t* Neuron_Init();

// Base Deconstructor
void Neuron_Delete(neuron_t* node);

#endif // !__NEURON_H__