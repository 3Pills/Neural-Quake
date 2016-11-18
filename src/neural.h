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

#include "quakedef.h"

// Defining base struct, which will be fleshed out via include later on.
typedef struct organism_s organism_t;

// ****** ENGINE HOOK FUNCTIONS ******
// These functions are called from within the engine, and
// allow a means of interfacing between the game and the network.

// Neural network commands are created within the engine.
//   Called as part of the engine's startup functions. Anything that 
//   should run as soon as the game boots should be run within this function.
void NQ_Init();

// Neural network is reloaded, resetting all variables.
//   Ran whenever the client has finished transitioning to a new level, 
//   or reloaded the current one. Anything that should reinitialize at
//   the start of a new genome run should do it here.
//   Called in the CL_ParseServerInfo function within cl_parse.c, 
//   after the client has received information on a server level transition.
void NQ_Reload();

// Neural network input gathering and layer processing stage.
//   Called in the CL_UpdateClient function within cl_main.c
//   after the client has received all updates from the server
//   and these updates have been applied within the client's game.
void CL_NeuralThink(double frametime);

// Neural network output stage. Game is issued inputs here.
//   Any input sent from the client to the engine should 
//   be either be added into the cmd variable or called 
//   here through use of the Cmd_ExecuteString command.
//   Called in the CL_SendCmd function within cl_main.c, right
//   before client commands are processed.
void CL_NeuralMove(usercmd_t *cmd);

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

// ***** COMMAND FUNCTIONS ******
// These functions are defined within cmd_list_sheet.h as functions for
// use within the game's console. They allow various functionality within
// the network to be called at will by the player.

// Neural network data is initialized and the network begins execution.
// Usage is nq_start <filename if loading>.
void NQ_Start(lparse_t *line);

// Ends execution of the network.
// Usage is nq_start <filename if saving>.
void NQ_End(lparse_t *line);

// Saves the current data of the network to a file for later use.
// Usage is nq_save <filename> [0 - continue executing | 1 - end execution].
void NQ_Save(lparse_t *line);

// Loads neural data from a file and begins executing it. 
// Usage is nq_load <filename>.
void NQ_Load(lparse_t *line);

// Forces the network to move on to the next organism.
// To be called if an organism gets stuck in a loop during execution.
void NQ_ForceTimeout();

// ***** NETWORK FUNCTIONS ******
// These helper functions are called during runtime, and contain 
// various functions called to process data within the network.
// Should not be called outside of the neural network space.

// Retrives information from the world for use within the neural network.
void NQ_GetInputs();

// Evaluates an organism within the neural network against the current input.
void NQ_Evaluate(organism_t* organism);

// Iterates to the next organism within the network
void NQ_NextOrganism();

// Resets the state of the neural network to prepare for the next evaluation.
void NQ_Timeout();

// ***** MATHS / UTILITY FUNCTIONS ******

// Draw various stats related to the current genome within a
// graph in the bottom left of the screen.
void Draw_NeuralStats();

// Draw the layout of the neural network in a graph 
// at the top right of the screen.
void Draw_NeuralGraph();

// Refreshes the hidden node data and node link data within the 
// neural graph to match that of the organism argument.
void UI_RefreshGraph(organism_t* organism);

// Getter function to determine whether the neural network is active.
cbool NQ_IsEnabled();

// This is a sigmoidal activation function, which is an S-shaped squashing function
// It smoothly limits the amplitude of the output of a neuron to between 0 and 1
// It is a helper to the neural-activation function get_active_out
double Sigmoid(double x); 

// Hebbian Adaptation Function
// Based on equations in Floreano & Urzelai 2000
// Takes the current weight, the maximum weight in the containing network,
// the activation coming in and out of the synapse, and three learning rates 
// for hebbian, presynaptic, and postsynaptic modification
// Returns the new modified weight
double Hebbian(double weight, double maxweight, double active_in, 
			   double active_out, double hebb_rate, double pre_rate, 
			   double post_rate);

// Returns a normally distributed deviate with 0 mean and unit variance
// Algorithm is from Numerical Recipes in C, Second Edition
double Random_Gauss();

// Returns a random integer between x and y.
int Random_Int(int x, int y);

// Returns a random decimal float between 0 and 1.
float Random_Float();

// Quicksort helper function for arrays. 
// args: first - First index to sort from.
//		 last - Last index to sort from.
//		 array - Array to sort.
//		 sort_func - A function returning a cbool which compares two pointer objects.
void Quicksort(int first, int last, void* array, cbool(*sort_func)(void*, void*));

// Sorting function to be passed into Quicksort function.
//   Sorts an array of values in ascending order.
cbool Quicksort_Ascending(double *x, double *y);

// Sorting function to be passed into Quicksort function.
//   Sorts an array of values in descending order.
cbool Quicksort_Descending(double *x, double *y);

// Helper function that copies the values from trace a to trace b.
void TraceCopy(trace_t *a, trace_t *b);

#endif // !__NEURAL_H__