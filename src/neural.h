/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2009-2010 Ozkan Sezer
Copyright (C) 2009-2014 Baker and others
Copyright (C) 2016		Stephen Koren

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
// neural.h -- Bridge between neural AI and the rest of the application.

#ifndef __NEURAL_H__
#define __NEURAL_H__

/*
#ifdef __cplusplus
#include <vector>

// Defines the connection between neurons
class Gene
{
	int m_iTo; // Input neuron ID.
	int m_iFrom; // Output neuron ID.
	int m_iInnovation; // Used for finding corresponding genes during crossover.
	float m_fWeight; // Weight of genetic connection.
	bool m_bEnabled; // Stops the genome using it.
};

// Genetic nodes which make up a neural network
class Neuron
{
	//std::vector<Gene> m_vInputGenes;
	int m_iValue;
};

// Network of neurons connected by genes.
class Genome
{
	//std::vector<Gene> m_vGenes;
	//std::vector<Neuron> m_vNeurons;
	int m_iFitness;
};

// Collection of Genomes that serve towards a specific trait.
class Species
{
public:
	//std::vector<Genome> m_vGenomes;
	int m_iHighestFitness;
	int m_iAverageFitness;
	int m_iStaleness;
private:

};

// The entire pool of species.
class Pool
{
public:
	//std::vector<Species> m_vSpecies;
	int m_iGeneration;
	int m_iCurrSpecies;
	int m_iCurrGenome;
	int m_iMaxFitness;
private:

};

class NeuralNetwork 
{
public:
	NeuralNetwork(const std::vector<unsigned int> &toplogy);

	void feedForward(const std::vector<double> &inputVals);
	void backProp(const std::vector<double> &targetVals);
	void getResults(std::vector<double> &resultVals) const;
private:

};
#else
*/

#include "vector.h"
#include "math_general.h"
#include "neural_def.h"

/*
typedef struct vector;
typedef struct neuron_t;

typedef struct 
{
	vector outputGenes; // vector array of outgoing Genes.
	vector inputGenes; // vector array of incoming Genes.
	int outputValue; // The Value output by the Neuron.
} neuron_t; // Genetic nodes which make up a neural network

typedef struct
{
	neuron_t* in_node; // Input neuron.
	neuron_t* out_node; // Output neuron.

	cbool recurrent;
	cbool time_delay;

	int trait_id; // identify the trait derived by this link.

	int added_weight;
	int params[NQ_NUM_TRAIT_PARAMS];

} link_t; // Link between two neural nodes.

typedef struct
{
	int ID;
	int fitness; // Determined fitness of the Genome.

	vector genes; // Pointer to array of Genes.
	vector neurons; // Pointer to array of Neurons.

} genome_t; // Network of neurons connected by genes.

typedef struct
{
	vector genomes;
} species_t; // Collection of Genomes that hold a specific trait.

typedef struct
{
	vector species;
} network_t; // The entire pool of species.
*/

void Neural_Init();

double Sigmoid(double x); 

int Random_Int(int x, int y);

//#endif

#endif // !__NEURAL_H__