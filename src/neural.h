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
// neural.h -- Bridge between neural AI and the rest of the application.
#ifndef __NEURAL_H__
#define __NEURAL_H__

#include <stdlib.h>
#include "vector.h"
#include "quakedef.h"
#include "network.h"
#include "population.h"
#include "organism.h"
#include "genome.h"
#include "species.h"

// ****** ENGINE HOOK FUNCTIONS ******
// These functions are called from within the engine, and
// allow a means of interfacing between the game and the network.

// Neural network data is initialized.
//   Called as part of the engine's startup functions. Anything that 
//   should run as soon as the game boots should be run within this function.
void Neural_Init();

// Neural network is reloaded.
//   Called whenever the client has finished transitioning to a new level, 
//   or reloaded the current one. Anything that should reinitialize at
//   the start of a new genome run should do it here.
void Neural_Reload();

// Neural network input gathering and layer processing stage.
//   Called in the CL_UpdateClient function within cl_main.c
//   after the client has received all updates from the server
//   and these updates have been applied within the client's game.
void CL_NeuralThink(double frametime);

// Neural network output stage. Game is issued inputs here.
//   Any input sent from the client to the engine should be,
//   called here through use of the Cmd_ExecuteString command.
//   Called in the CL_SendCmd function within cl_main.c, right
//   before client commands are processed.
void CL_NeuralMove();

// Neural network 3D rendering stage. 
//   All debug world rendering should be done here, to visually 
//   represent the function of the neural network. 
//   Called in the R_RenderScene function in gl_rmain.c.
void R_DrawNeuralData();

// Neural network 2D rendering stage. 
//   All debug HUD rendering should be done here, to visually 
//   represent the function of the neural network. 
//   Called in the SCR_UpdateScreen function in screen.c.
//   Objects drawn in this function will be drawn over the game, 
//   all HUD elements and dev statistics, but will not be drawn over
//   the console, the FPS counter, any download bars or any menus.
void SCR_DrawNeuralData();

// ***** EVOLUTION FUNCTIONS ******
// These functions are called during evolution, and should not
// be called outside of the neural network space.

void NQ_Test();
void NQ_Evaluate(organism_t *org);
int NQ_Epoch(population_t *pop, int generation, int *winnernum, int *winnergenes, int *winnernodes);

void NQ_InitializePool();
void NQ_InitializeRun();
void NQ_NextOrganism();
void NQ_Timeout();
void NQ_GetInputs();

// ***** MATHS / UTILITY FUNCTIONS ******

// This is a signmoidal activation function, which is an S-shaped squashing function
// It smoothly limits the amplitude of the output of a neuron to between 0 and 1
// It is a helper to the neural-activation function get_active_out
// It is made inline so it can execute quickly since it is at every non-sensor 
// node in a network.
double Sigmoid(double x); 

// Hebbian Adaptation Function
// Based on equations in Floreano & Urzelai 2000
// Takes the current weight, the maximum weight in the containing network,
// the activation coming in and out of the synapse,
// and three learning rates for hebbian, presynaptic, and postsynaptic
// modification
// Returns the new modified weight
// NOTE: For an inhibatory connection, it makes sense to
//      emphasize decorrelation on hebbian learning!
double Hebbian(double weight, double maxweight, double active_in, 
			   double active_out, double hebb_rate, double pre_rate, 
			   double post_rate);

// Returns a normally distributed deviate with 0 mean and unit variance
// Algorithm is from Numerical Recipes in C, Second Edition
double Random_Gauss();

// Returns a random integer between x and y.
int Random_Int(int x, int y);

// Returns a random decimal float between 0 and 1.
int Random_Float();

// Returns either 1 or -1 randomly.
int Random_Sign();

// Quicksort helper function for arrays. 
// args: first - First index to sort from.
//		 last - Last index to sort from.
//		 array - Array to sort.
//		 sort_func - A function returning a cbool which compares two pointer objects.
void Quicksort(int first, int last, void* array, cbool(*sort_func)(void*, void*));


// Sorting function to be passed into Quicksort function.
//   Sorts an array of values in ascending order.
cbool Quicksort_Ascending(double x, double y);

// Sorting function to be passed into Quicksort function.
//   Sorts an array of values in descending order.
cbool Quicksort_Descending(double x, double y);

void TraceCopy(trace_t *a, trace_t *b);

//#endif

#endif // !__NEURAL_H__