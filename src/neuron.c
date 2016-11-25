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
#include "neuron.h"

neuron_t* Neuron_Init()
{
	neuron_t* neuron = (neuron_t*)malloc(sizeof(neuron_t));
	if (neuron == 0) return NULL;

	neuron->value = 0;
	neuron->incoming_genes = vector_init();
	return neuron;
}

void Neuron_Delete(neuron_t* neuron)
{
	// Don't free any genes from the vector because they will exist within the genome still.
	vector_free_all(neuron->incoming_genes);
	free(neuron);
}