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
// neural.h -- Class definitions for neural network implementation

#ifndef __NEURAL_H__
#define __NEURAL_H__

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
void Neural_Init();



typedef struct
{
	int m_iTo; // Input neuron ID.
	int m_iFrom; // Output neuron ID.
	int m_iInnovation; // Used for finding corresponding genes during crossover.
	float m_fWeight; // Weight of genetic connection.
	cbool m_bEnabled; // Stops the genome using it.
} gene_t;
#endif

#endif // !__NEURAL_H__