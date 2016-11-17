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
// neural.cpp -- Function definitions for neural network implementations

#include "quakedef.h"
#include "r_neural.h"
#include "neural_def.h"

// Determines whether the network should be executing or not. 
// Set to true during NQ_Start.
static cbool network_on = false;

// Neural network population. Accessor to every neural network. 
static population_t *population = 0;

// Contains: trace_t*. A vector of input data gathered during NQ_GetInputs.
static vector *inputs = 0;

// Array of output values from the neural network.
static double *outputs = 0;

// Timer to restart the level when the AI idles or dies.
static double timeout = 0.0;

// Timer to space out neural processing, rather than doing it every frame.
static double timestep = 0.0;

static int trace_length = 1000.0f;

// Index of the current species within the population.
static int currSpecies = 0;

// Index of the current organism within the species.
static int currOrganism = 0;

// Current generation of the population.
static int generation = 0;

// Position of the player spawn point.
static vec3_t start_pos = { 0, 0, 0 };

// Tracks the position change of the player. Used to determine timeout.
static vec3_t prev_pos = { 0, 0, 0 };

// Distance to travel before prev_pos is updated.
// Roughly the distance of a jump.
static double distance_to_timeout = 64.0;

// Contains: vec3_t*. Stores all the final pos results of every genome ever.
static vector *distStorage = 0;

// Spawn flag to determine when to reload
static cbool spawnSet = false;

// Name of the currently loaded level.
// Tracks whether the level has changed or not.
static char levelName[64];

// Output commands without +/- prefix. 
// Prefix will be added depending on the output of the network.
static char* outputCmds[NQ_OUTPUT_COUNT] = {
	"forward", "back", "moveleft", "moveright",
	"left", "right", "lookup", "lookdown", "attack", "jump"
};


// Neural Graph variable definitions.
#define NQ_GRAPH_POSX 0
#define NQ_GRAPH_POSY 80

#define NQ_GRAPH_WIDTH 360
#define NQ_GRAPH_HEIGHT 220

// Values for boxes in graph.
#define NQ_GRAPH_BOX_PADDING 2
#define NQ_GRAPH_INBOX_WIDTH NQ_GRAPH_WIDTH / NQ_INPUT_COLS
#define NQ_GRAPH_INBOX_HEIGHT NQ_GRAPH_HEIGHT / NQ_INPUT_ROWS

#define NQ_GRAPH_HIDDEN_HEIGHT 1.1

#define NQ_GRAPH_OUTBOX_SIZE NQ_GRAPH_WIDTH / 15
#define NQ_GRAPH_OUTPUT_HEIGHT 1.8

// Contains: uinode_t. Stores nodes for display in the neural graph.
static vector *uinodes;

// Contains: uilink_t. Stores links for display in the neural graph.
static vector *uilinks;

void NQ_Init()
{
	Cmd_AddCommands(NQ_Init);
}

void NQ_Reload()
{
	if (strcmp(levelName, cl.worldname) != 0)
	{
		spawnSet = false;
		strcpy(levelName, cl.worldname);
	}
}

void CL_NeuralThink(double frametime)
{
	if (!network_on || population == NULL || sv.paused || key_dest != key_game) return;

	float timescale = (host_timescale.value == 0) ? 1 : host_timescale.value;

	if (!spawnSet)
	{
		VectorCopy(cl_entities[cl.viewentity].origin, start_pos);
		spawnSet = true;

		// Add the spawn position of the player to check the sparseness of our results from.
		vector_clear(distStorage);
		vec3_t *firstStorage = malloc(sizeof(vec3_t));

		*(*firstStorage + 0) = cl_entities[cl.viewentity].origin[0];
		*(*firstStorage + 1) = cl_entities[cl.viewentity].origin[1];
		*(*firstStorage + 2) = cl_entities[cl.viewentity].origin[2];

		vector_add(distStorage, firstStorage);
	}

	timeout += frametime * timescale;

	// Only gather input when the player is alive.
	if (cl.stats[STAT_HEALTH] > 0)
	{
		timestep += frametime * timescale;

		if (DistanceBetween2Points(prev_pos, cl_entities[cl.viewentity].origin) > distance_to_timeout)
		{
			timeout = 0;
			VectorCopy(cl_entities[cl.viewentity].origin, prev_pos);
		}

		// Interact with network at 12FPS.
		if (timestep > (double)1 / 12)
		{
			// Retrieve information on what the AI can see.
			NQ_GetInputs();

			// Evaluate the input, passing it into the current organism within the current species.
			NQ_Evaluate(((species_t*)population->species->data[currSpecies])->organisms->data[currOrganism]);
		}
	}
	// Timeout after two seconds once we die. Just to 
	else if (timeout + 2 < NQ_TIMEOUT)
	{
		timeout = NQ_TIMEOUT - 2;
	}

	if (sv_player != NULL && timeout >= NQ_TIMEOUT) NQ_Timeout();
}

void CL_NeuralMove(usercmd_t *cmd)
{
	// If the client doesn't have entities, it will 
	// not have the player to trace from. Return.
	if (!network_on || sv.max_edicts == 0) return;

	// Timestep for input gathering and engine output.
	if (timestep < (double)1 / 12) return;

	// We need to disable all inputs if we're paused or otherwise not inputting to the game.
	if (sv.paused || key_dest != key_game)
	{
		for (int i = 0; i < NQ_OUTPUT_COUNT; i++)
		{
			char out_cmd[80];
			strcpy(out_cmd, "-");
			strcat(out_cmd, outputCmds[i]);
			Cmd_ExecuteString(out_cmd, src_client);
		}
		return;
	}

	timestep = 0;

	// Execute movement commands based on the output results of the network.
	for (int i = 0; i < NQ_OUTPUT_COUNT; i++)
	{
		char out_cmd[80];
		double output = outputs[i];
		strcpy(out_cmd, (output > 0.5) ? "+" : "-");
		strcat(out_cmd, outputCmds[i]);
		Cmd_ExecuteString(out_cmd, src_client);
	}
}

void R_DrawNeuralData()
{
	if (!network_on || inputs == NULL) return;

	R_DrawPoint(prev_pos, 12, (DistanceBetween2Points(prev_pos, cl_entities[cl.viewentity].origin) > distance_to_timeout) ? 15 : 7);

	// Draw the impact point of the traces we gathered in NQ_GetInputs.
	for (int i = 0; i < inputs->count; i++)
	{
		trace_t* trace = inputs->data[i];
		// We use the otherwise unused plane.dist for debug color storage.
		R_DrawPoint(trace->endpos, fmax(8 * trace->fraction, 1), trace->plane.dist);
	}
}

void SCR_DrawNeuralData()
{
	if (!network_on) return;
	if (neuralstats.value) Draw_NeuralStats();
	if (neuralgraph.value) Draw_NeuralGraph();
}

void NQ_Start(lparse_t *line)
{
	// Attempt to load filename if its passed in as argument.
	if (line->count == 2) NQ_Load(line);

	Con_Printf("\nNeural population initialization\n");

	Con_Printf("  Spawning Population\n");
	genome_t *start_genome = Genome_Init_Auto(NQ_INPUT_COUNT, NQ_OUTPUT_COUNT, 0, 0);
	population = Population_Init(start_genome, NQ_POP_SIZE);

	Con_Printf("  Verifying Spawned Population\n");
	Population_Verify(population);

	network_t *network = Genome_Genesis(start_genome, population->organisms->count);
	Con_Printf("  Activating Network\n");
	Network_Activate(network);

	Con_Printf("  Building Inputs and Outputs\n");

	inputs = vector_init();

	// Initialize a bunch of trace objects to store the data from input gathering in.
	for (int i = 0; i < NQ_INPUT_COUNT; i++)
	{
		trace_t* trace = malloc(sizeof(trace_t));
		trace->allsolid = false;
		trace->startsolid = false;
		trace->ent = 0;
		trace->fraction = 1.0;
		trace->inopen = false;
		trace->inwater = false;
		trace->plane.dist = 10;

		memset(trace->endpos, 0, 3 * sizeof(float));
		memset(trace->plane.normal, 0, 3 * sizeof(float));

		vector_add(inputs, trace);
	}

	distStorage = vector_init();

	outputs = malloc(sizeof(outputs)*NQ_OUTPUT_COUNT);
	memset(outputs, 0, NQ_OUTPUT_COUNT * sizeof(double*));

	Con_Printf("   Building UI Graph Data\n");

	uinodes = vector_init();
	uinode_t *uinode = 0;

	// Create input UI Nodes for the graph.
	for (int i = 0; i < NQ_INPUT_COUNT; i++)
	{
		int x = i % NQ_INPUT_COLS, y = (NQ_INPUT_ROWS - 1) - (i / NQ_INPUT_COLS);

		uinode = malloc(sizeof(uinode_t));

		uinode->x = NQ_GRAPH_POSX + 1 + x * NQ_GRAPH_INBOX_WIDTH;
		uinode->y = NQ_GRAPH_POSY + y * NQ_GRAPH_INBOX_HEIGHT;
		uinode->sizex = NQ_GRAPH_INBOX_WIDTH - NQ_GRAPH_BOX_PADDING;
		uinode->sizey = NQ_GRAPH_INBOX_HEIGHT - NQ_GRAPH_BOX_PADDING;

		vector_add(uinodes, uinode);
	}

#pragma region output_uinodes
	// Manually Create output UI nodes for the graph.

	// forward
	uinode = malloc(sizeof(uinode_t));
	uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.025 + NQ_GRAPH_OUTBOX_SIZE;
	uinode->y = NQ_GRAPH_POSY + (NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT) - NQ_GRAPH_OUTBOX_SIZE * 3;
	uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;

	vector_add(uinodes, uinode);

	// back
	uinode = malloc(sizeof(uinode_t));
	uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.025 + NQ_GRAPH_OUTBOX_SIZE;
	uinode->y = NQ_GRAPH_POSY + (NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT) - NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;

	vector_add(uinodes, uinode);

	// moveleft
	uinode = malloc(sizeof(uinode_t));
	uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.025;
	uinode->y = NQ_GRAPH_POSY + NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT - NQ_GRAPH_OUTBOX_SIZE * 2;
	uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;

	vector_add(uinodes, uinode);

	// moveright
	uinode = malloc(sizeof(uinode_t));
	uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.025 + NQ_GRAPH_OUTBOX_SIZE * 2;
	uinode->y = NQ_GRAPH_POSY + NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT - NQ_GRAPH_OUTBOX_SIZE * 2;
	uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;

	vector_add(uinodes, uinode);

	// left
	uinode = malloc(sizeof(uinode_t));
	uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.975 - NQ_GRAPH_OUTBOX_SIZE * 3;
	uinode->y = NQ_GRAPH_POSY + NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT - NQ_GRAPH_OUTBOX_SIZE * 2;
	uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;

	vector_add(uinodes, uinode);

	// right
	uinode = malloc(sizeof(uinode_t));
	uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.975 - NQ_GRAPH_OUTBOX_SIZE;
	uinode->y = NQ_GRAPH_POSY + NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT - NQ_GRAPH_OUTBOX_SIZE * 2;
	uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;

	vector_add(uinodes, uinode);

	// lookup
	uinode = malloc(sizeof(uinode_t));
	uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.975 - NQ_GRAPH_OUTBOX_SIZE * 2;
	uinode->y = NQ_GRAPH_POSY + (NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT) - NQ_GRAPH_OUTBOX_SIZE * 3;
	uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;

	vector_add(uinodes, uinode);

	// lookdown
	uinode = malloc(sizeof(uinode_t));
	uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.975 - NQ_GRAPH_OUTBOX_SIZE * 2;
	uinode->y = NQ_GRAPH_POSY + NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT - NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;

	vector_add(uinodes, uinode);

	// attack
	uinode = malloc(sizeof(uinode_t));
	uinode->x = NQ_GRAPH_POSX + (NQ_GRAPH_WIDTH * 0.5 - NQ_GRAPH_OUTBOX_SIZE / 2) - NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING;
	uinode->y = NQ_GRAPH_POSY + (NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT) - NQ_GRAPH_OUTBOX_SIZE - (NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING) / 2;
	uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;

	vector_add(uinodes, uinode);

	// jump
	uinode = malloc(sizeof(uinode_t));
	uinode->x = NQ_GRAPH_POSX + (NQ_GRAPH_WIDTH * 0.5 - NQ_GRAPH_OUTBOX_SIZE / 2) + NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING;
	uinode->y = NQ_GRAPH_POSY + (NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT) - NQ_GRAPH_OUTBOX_SIZE - (NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING) / 2;
	uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
	uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;
	vector_add(uinodes, uinode);

#pragma endregion

	uilinks = vector_init();

	// Add all extra hidden / bias nodes.
	for (int i = NQ_INPUT_COUNT + NQ_OUTPUT_COUNT; i < network->all_nodes->count; i++)
	{
		uinode = malloc(sizeof(uinode_t));
		vector_add(uinodes, uinode);
	}

	// Initialize hidden / bias node positions and represent their neural links. 
	UI_RefreshGraph(population->organisms->data[0]);

	Network_Flush(network);

	Con_Printf("Neural population initialized\n");

	network_on = true;
}

void NQ_End(lparse_t *line)
{
	// Disable all inputs on end.
	for (int i = 0; i < NQ_OUTPUT_COUNT; i++)
	{
		char out_cmd[80];
		strcpy(out_cmd, "-");
		strcat(out_cmd, outputCmds[i]);
		Cmd_ExecuteString(out_cmd, src_client);
	}

	network_on = false;
}

void NQ_Save(lparse_t *line)
{
	if (line->count != 2)
	{
		Con_Printf("nq_save <filename> : save network data\n");
		return;
	}
}

void NQ_Load(lparse_t *line)
{
	if (line->count != 2)
	{
		Con_Printf("nq_load <filename> [0|1] : load network data, set to 1 to start immediately on load.\n");
		return;
	}
}

void NQ_ForceTimeout()
{
	NQ_Timeout();
}


void NQ_GetInputs()
{
	// If the client doesn't have entities, it will 
	// not have the player to trace from. Return.
	if (sv.max_edicts == 0) return;

	// Rotation delta per cell.
	double deltaX = r_fovx / NQ_INPUT_COLS, deltaY = r_fovy / NQ_INPUT_ROWS;

	// Starting angle offset from centre. 
	// These angles combine to be the top left corner of the player's view.
	double angX = (-r_fovx + deltaX) / 2, angY = (-r_fovy + deltaY) / 2;

	// Define directional vectors.
	vec3_t view_pos;
	VectorCopy(r_refdef.vieworg, view_pos);

	// Player's directional view vectors.
	vec3_t view_f, view_r, view_u;

	// Translate the player's view angles into directional vectors.
	AngleVectors(cl.viewangles, view_f, view_r, view_u);

	// Intermediate operational vector values.
	vec3_t final_pitch, final_r, final_dir, final_pos;

	for (int i = 0; i < NQ_INPUT_ROWS; i++)
	{
		// Bool to stop rotations along pitch if centred on the player's view.
		// This exists solely for code optimisation.
		cbool midY = (i == (int)(NQ_INPUT_ROWS / 2));

		if (midY) // Pitch is the same as the view.
		{
			VectorCopy(view_f, final_pitch);
		}
		else
		{
			//Rotate final_dir to the final pitch.
			TurnVector(final_pitch, view_f, view_u, angY + deltaY * i);
		}

		for (int j = 0; j < NQ_INPUT_COLS; j++)
		{
			// Bool to stop rotations along yaw if centred on the player's view.
			cbool midX = (j == (int)(NQ_INPUT_COLS / 2));

			if (midX) // Yaw is the same as the view
			{
				if (midY)
				{
					// The direction is the view forward.
					VectorCopy(view_f, final_dir);
				}
				else
				{
					// No need to calculate yaw. Use pitch as the final direction.
					VectorCopy(final_pitch, final_dir);
				}
			}
			else
			{
				if (midY)
				{
					VectorCopy(view_r, final_r); // Right hasn't changed.
				}
				else
				{
					// Calculate the new right vector using the new forward.
					CrossProduct(final_pitch, view_u, final_r);
				}

				// Rotate forward to the final yaw and subsequently final direction.
				TurnVector(final_dir, final_pitch, final_r, angX + deltaX * j);
			}

			// Scale the end direction forward by the falloff distance amount.
			VectorScale(final_dir, trace_length, final_dir);

			// Move the direction to world space and to the final pos.
			VectorAdd(final_dir, view_pos, final_pos);

			// Complete a trace, ignoring EDICT_NUM(1).
			// This will always be the client's player entity.
			trace_t trace = SV_Move(view_pos, vec3_origin, vec3_origin,
				final_pos, false, EDICT_NUM(1));

			// We use the plane distance value as a color value, because we don't 
			// need this information, and we need extra storage for debug colors.
			trace.plane.dist = 15;

			if (trace.fraction != 1.0) // fraction is 1.0 if nothing was hit.
			{
				if (trace.ent->v.solid == SOLID_BSP) // We traced a world clip.
				{
					vec3_t up = { 0, 0, 1 };
					trace.plane.dist = 240 + (int)(4.0 - DotProduct(up, trace.plane.normal) * 4.0);
				}
				else // It's an entity of some kind!
				{
					trace.plane.dist = 79;
				}
			}
			else // Hit nothing
			{
				trace.plane.dist = 40;
			}

			// Copy trace data to the input data for the frame.
			TraceCopy(&trace, inputs->data[j + NQ_INPUT_COLS*i]);
		}
	}
}

void NQ_Evaluate(organism_t* organism)
{
	// Error Handling for null organism.
	if (organism == NULL)
	{
		Con_Printf("NQ ERROR: ATTEMPTED TO EVALUATE ERRONEOUS GENOME.");
		return;
	}

	/***** INPUT RETRIEVING *****/

	// New values of each node to be passed into the network.
	double nodeVals[NQ_INPUT_COUNT];

	network_t *network = organism->net;

	// Convert input traces into numerical values for network nodes.
	for (int i = 0; i < inputs->count; i++)
	{
		trace_t *trace = inputs->data[i];

		// Default value is 0.0, which will denote a flat ground to walk to. 
		// Useful because the only time if check will not modify the input is if its empty space.
		double nodeValue = 0.0;

		if (trace->fraction != 1.0) // fraction is 1.0 if nothing was hit.
		{
			if (trace->ent->v.solid == SOLID_BSP) // We traced a world clip.
			{
				vec3_t up = { 0, 0, 1 };

				// We want normals parallel to the up axis to be 0.
				// We also want the range to be between 0.0 and 1.0, so down vectors
				// will not drive the value up to 2.0 (1.0 - -1.0).
				nodeValue = (1.0 - DotProduct(up, trace->plane.normal)) / 2;
			}
			else // It's an entity of some kind!
			{
				// -1.0 is a unique value for entities only.
				nodeValue = -1.0;
			}
		}
		nodeVals[i] = nodeValue;
	}

	/***** NETWORK PROCESSING *****/

	// Add the node vals to the network.
	Network_Load_Sensors(network, nodeVals);

	cbool success = false;

	// Process the values through the layers and relax network, using depth.
	for (int i = 0; i <= Network_Max_Depth(network) + 1; i++)
		success = Network_Activate(network);

	// Take the activation value of the network into the global output vector for use with inputs.
	for (int i = 0; i < network->outputs->count; i++)
		outputs[i] = ((neuron_t*)network->outputs->data[i])->activation;

	for (int i = 0; i < uilinks->count; i++)
	{
		uilink_t *uilink = uilinks->data[i];
		gene_t *gene = uilink->gene;
		uilink->opacity = !gene->enabled ? 0 : gene->link->inode->value == 0 ? 0.1 : 0.6;
	}

	Network_Flush(network);

	/***** FITNESS EVALUATION *****/

	// This will hold the distances from our new behavior

	int distCount = distStorage->count;

	// Stored as a pointer array to allow compatibility with Quicksort function.
	double **distList = malloc(distCount * sizeof(*distList));

	// First get the distance of the point to the rest of the population.
	for (int i = 0; i < distCount; i++)
	{
		double* distance = malloc(sizeof(distance));
		vec3_t* other = distStorage->data[i];
		*distance = (double)DistanceBetween2Points(cl_entities[cl.viewentity].origin, *other);
		distList[i] = distance;
	}

	// Sort the list to get the closest distances.
	Quicksort(0, distCount - 1, distList, Quicksort_Ascending);

	double fitness = 0.0;

	// Compute the sparseness of the point. The average distance away from the genome's point.
	double sparseness = 0.0;
	for (unsigned int i = 0; i < (unsigned int)fmin(NQ_NOVELTY_COEFF, distCount); i++)
		sparseness += *distList[i];

	sparseness /= NQ_NOVELTY_COEFF;

	// Assign the sparseness as the base fitness value.
	fitness = sparseness;

	// Increate the fitness value based on a number of stats.
	fitness += cl.stats[STAT_MONSTERS] * 100; // Monster kills. Single player.
	fitness += cl.stats[STAT_SECRETS] * 250; // Secrets found. Single player.
	fitness += cl.stats[STAT_FRAGS] * 500; // Player kills. Multi player.

	// Lower the final fitness value by a maximum of 80% based on the player's health.
	fitness = sparseness * (1 - ((1 - cl.stats[STAT_HEALTH] / 100) * 0.8));

	organism->fitness = success ? sparseness : 0.001;

	free(distList);
}

void NQ_NextOrganism()
{
	currOrganism++;

	species_t *species = population->species->data[currSpecies];

	// We've finished evaluating the population's organisms.
	if (currOrganism >= species->organisms->count)
	{
		for (int i = 0; i < population->species->count; i++)
		{
			Species_Compute_Average_Fitness(population->species->data[i]);
			Species_Compute_Max_Fitness(population->species->data[i]);
		}

		Population_Epoch(population, ++generation);

		for (int i = 0; i < population->organisms->count; i++)
			Organism_Update_Phenotype(population->organisms->data[i]);

		currOrganism = 0;
	}

	UI_RefreshGraph(species->organisms->data[currOrganism]);

	char map_cmd[128] = "map ";

	// The reload the level in single player, otherwise kill the bot.
	if (svs.maxclients == 1)
	{
		strcat(map_cmd, cl.worldname);
		Cmd_ExecuteString(map_cmd, src_command);
	}
	else
	{
		Cmd_ExecuteString("kill", src_client);
	}
}

void NQ_Timeout()
{
	species_t *species = population->species->data[currSpecies];
	organism_t *organism = species->organisms->data[currOrganism];

	// Add the in-game clock timer to the organism when done.
	organism->time_alive = cl.time;

	// Store the player's current position as the organisms final position.
	VectorCopy(cl_entities[cl.viewentity].origin, organism->final_pos);

	// Add the final position of the organism to the global list.
	vec3_t *final_pos = malloc(sizeof(vec3_t));

	*(*final_pos + 0) = cl_entities[cl.viewentity].origin[0];
	*(*final_pos + 1) = cl_entities[cl.viewentity].origin[1];
	*(*final_pos + 2) = cl_entities[cl.viewentity].origin[2];

	vector_add(distStorage, final_pos);

	timeout = 0;
	timestep = 0;

	NQ_NextOrganism();
}

void Draw_NeuralStats()
{
	// Drawing Neural statistics here.
	Draw_SetCanvas(CANVAS_BOTTOMLEFT);

	int x = 4; //margin
	int y = 25 - 7.5;
	char str[40];

	Draw_Fill(0, y * 7.5, 25 * 9, 9 * 8, 0, 0.5); //dark rectangle

	if (population != 0)
	{
		c_snprintf2(str, "Input Row/Col|  %4i %4i", NQ_INPUT_ROWS, NQ_INPUT_COLS);
		Draw_String(x, (y++) * 8 - x, str);

		c_strlcpy(str, "NEURAL STATS |  Curr Best  ");
		Draw_String(x, (y++) * 8 - x, str);

		c_strlcpy(str, "-------------+-------------");
		Draw_String(x, (y++) * 8 - x, str);

		species_t *species = population->species->data[currSpecies];
		organism_t *organism = species->organisms->data[currOrganism];
		if (organism != 0)
		{

			int bestSpeciesID, bestOrgID = 0;
			species_t* bestSpecies = 0;
			for (int i = 0; i < population->species->count; i++)
			{
				species_t *species = population->species->data[i];

				if (bestSpecies == 0 || bestSpecies->peak_fitness < species->peak_fitness)
				{
					bestSpecies = species;
					bestSpeciesID = species->id;
				}
			}

			for (int i = 0; i < population->organisms->count; i++)
				if (((organism_t*)population->organisms->data[i])->champion)
					bestOrgID = i;

			c_snprintf2(str, "Fitness      |  %4i %4i", (int)organism->fitness, (int)organism->species->peak_fitness);
			Draw_String(x, (y++) * 8 - x, str);

			c_snprintf2(str, "Genome       |  %4i %4i", currOrganism, bestOrgID);
			Draw_String(x, (y++) * 8 - x, str);

			c_snprintf2(str, "Species      |  %4i %4i", organism->species->id, bestSpeciesID);
			Draw_String(x, (y++) * 8 - x, str);

			c_snprintf2(str, "Generation   |  %4i %4i", organism->generation, population->winnergen);
			Draw_String(x, (y++) * 8 - x, str);

			//c_snprintf2(str,	"Percentage   |  %4i %4i", 1, 1);
			//Draw_String(x, (y++) * 8 - x, str);
		}
		else
		{
			c_strlcpy(str, "Genome       |  -- --");
			Draw_String(x, (y++) * 8 - x, str);

			c_strlcpy(str, "Species      |  -- --");
			Draw_String(x, (y++) * 8 - x, str);

			c_strlcpy(str, "Fitness      |  -- --");
			Draw_String(x, (y++) * 8 - x, str);

			c_strlcpy(str, "Generation   |  -- --");
			Draw_String(x, (y++) * 8 - x, str);
		}
	}
}

void Draw_NeuralGraph()
{
	// Set to draw from the top left.
	Draw_SetCanvas(CANVAS_DEFAULT);

	if (population != 0)
	{
		Draw_Fill(NQ_GRAPH_POSX, NQ_GRAPH_POSY, NQ_GRAPH_WIDTH, NQ_GRAPH_HEIGHT * 1.9, 0, 0.5);

		species_t *species = population->species->data[currSpecies];
		organism_t *organism = species->organisms->data[currOrganism];

		// Draw all nodes in the network here.
		if (uinodes != 0)
		{
			if (inputs != 0 && inputs->count > 0)
			{
				for (int i = 0; i < NQ_INPUT_COUNT; i++)
				{
					uinode_t *uinode = uinodes->data[i];
					trace_t *trace = inputs->data[i];

					Draw_Square(uinode->x, uinode->y, uinode->sizex, uinode->sizey, 1, trace->plane.dist, 1);
				}
			}

			// Draw all nodes between the input and the output in a new row.
			// This begins with the input bias node, which always exists.
			for (int i = NQ_INPUT_COUNT + NQ_OUTPUT_COUNT; i < uinodes->count; i++)
			{
				uinode_t *uinode = uinodes->data[i];
				if (uinode != 0) Draw_Square(uinode->x, uinode->y, uinode->sizex, uinode->sizey, 1, uinode->color, 1);
			}

			// Manually draw each output in spots resembelling a controller
			if (outputs != 0)
			{
				// Draw output nodes.
				for (int i = NQ_INPUT_COUNT; i < NQ_INPUT_COUNT + NQ_OUTPUT_COUNT; i++)
				{
					uinode_t *uinode = uinodes->data[i];
					Draw_Square(uinode->x, uinode->y, uinode->sizex, uinode->sizey, 1, outputs[i - NQ_INPUT_COUNT] > 0.5 ? 251 : 248, 1);
				}

				// Draw output labels.
				Draw_String(NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.025 + NQ_GRAPH_OUTBOX_SIZE * 0.85, NQ_GRAPH_POSY + (NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT) + NQ_GRAPH_OUTBOX_SIZE * 0.15, "Move");
				Draw_String(NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.5 - (NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING) * 2.25, NQ_GRAPH_POSY + (NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT) - NQ_GRAPH_OUTBOX_SIZE * 0.35, "Attack");
				Draw_String(NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.5 + (NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING) * 0.35, NQ_GRAPH_POSY + (NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT) - NQ_GRAPH_OUTBOX_SIZE * 0.35, "Jump");
				Draw_String(NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.975 - NQ_GRAPH_OUTBOX_SIZE * 2.25, NQ_GRAPH_POSY + (NQ_GRAPH_HEIGHT * NQ_GRAPH_OUTPUT_HEIGHT) + NQ_GRAPH_OUTBOX_SIZE * 0.15, "Look");
			}
		}

		// Draw all existing links between hidden nodes in the network. 
		if (uilinks != 0 && uilinks->count > 0)
		{
			for (int i = 0; i < uilinks->count; i++)
			{
				uilink_t *uilink = uilinks->data[i];
				Draw_Line(uilink->start->x + NQ_GRAPH_INBOX_WIDTH / 2, uilink->start->y + NQ_GRAPH_INBOX_HEIGHT, 
					uilink->end->x + NQ_GRAPH_INBOX_WIDTH / 2, uilink->end->y, 1, uilink->color, uilink->opacity);
			}
		}
	}
}

void UI_RefreshGraph(organism_t *organism)
{
	// Remove all hidden UI nodes from the vector. Except for the input bias.
	for (int i = uinodes->count - 1; i > NQ_INPUT_COUNT + NQ_OUTPUT_COUNT + 1; i--)
	{
		free(uinodes->data[i]);
		uinodes->data[i] = NULL;
		uinodes->count--;
	}

	vector_free(uilinks);


	for (int i = 0; i < NQ_INPUT_COUNT; i++)
	{
		neuron_t *node = organism->net->all_nodes->data[i];
		uinode_t *uinode = uinodes->data[i];
		uinode->node = node;
	}

	/*
	// Build links for bias, outputs and hidden layers.
	for (int i = 0; i < node->ilinks->count; i++)
	{
		nlink_t *link = node->ilinks->data[i];
		// Just be sure the output node is the node we're after.
		if (link->onode == node)
		{
			uilink_t *uilink = malloc(sizeof(uilink_t));

			uilink->start = uinodes->data[link->inode->node_id];
			uilink->end = uinode;
			uilink->color = link->weight > 0 ? 63 : link->weight < 0 ? 79 : 7;
			uilink->opacity = link->inode->value == 0 ? 0.1 : 0.6;
			uilink->link = link;

			vector_add(uilinks, uilink);
		}
	}

	for (int i = 0; i < node->olinks->count; i++)
	{
		nlink_t *link = node->olinks->data[i];
		// Same check with input node of the output link.
		if (link->inode == node)
		{
			uilink_t *uilink = malloc(sizeof(uilink_t));

			uilink->start = uinode;
			uilink->end = uinodes->data[link->onode->node_id];
			uilink->color = roundf(247 + link->weight * 4);
			uilink->opacity = link->inode->value == 0 ? 0.1 : 0.6;
			uilink->link = link;

			vector_add(uilinks, uilink);
		}
	}
	*/

	// Refresh all hidden layer nodes, also linking them between their inputs and outputs.
	for (int i = NQ_INPUT_COUNT; i < organism->net->all_nodes->count; i++)
	{
		neuron_t *node = organism->net->all_nodes->data[i];
		uinode_t *uinode = 0;

		if (uinodes->count <= i || uinodes->data[i] == NULL)
		{
			uinode = malloc(sizeof(uinode_t));
			vector_add(uinodes, uinode);
		}
		else
		{
			uinode = uinodes->data[i];
		}

		uinode->node = node;

		if (i >= NQ_INPUT_COUNT + NQ_OUTPUT_COUNT)
		{

			int x = ((i - (NQ_INPUT_COUNT + NQ_OUTPUT_COUNT)) % NQ_INPUT_COLS),
				y = (i - (NQ_INPUT_COUNT + NQ_OUTPUT_COUNT + 1)) / NQ_INPUT_COLS;

			uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_BOX_PADDING + x * NQ_GRAPH_INBOX_WIDTH;
			uinode->y = NQ_GRAPH_POSY + NQ_GRAPH_HEIGHT * NQ_GRAPH_HIDDEN_HEIGHT + NQ_GRAPH_BOX_PADDING + y * NQ_GRAPH_INBOX_HEIGHT;
			uinode->sizex = NQ_GRAPH_INBOX_WIDTH - NQ_GRAPH_BOX_PADDING;
			uinode->sizey = NQ_GRAPH_INBOX_HEIGHT - NQ_GRAPH_BOX_PADDING;

			// We don't link the bias between nodes, because it is implied to link to every node.
			// If we did it would just obfuscate the diagram.
			if (node->node_label == NQ_BIAS)
			{
				uinode->color = 15;
			}
			else if (node->node_label == NQ_HIDDEN)
			{
				uinode->color = 63;
			}
		}
	}

	for (int i = 0; i < organism->gnome->genes->count; i++)
	{
		gene_t *gene = organism->gnome->genes->data[i];
		uilink_t *uilink = malloc(sizeof(uilink_t));

		uilink->start = uinodes->data[gene->link->inode->node_id];
		uilink->end = uinodes->data[gene->link->onode->node_id];
		uilink->color = gene->link->weight > 0 ? 63 : gene->link->weight < 0 ? 79 : 7;
		uilink->opacity = !gene->enabled ? 0.0 : 0.1;
		uilink->gene = gene;

		vector_add(uilinks, uilink);
	}
}

cbool NQ_IsEnabled()
{
	return network_on;
}

double Sigmoid(double x) 
{
	return (1.0 / (1.0 + exp(-(4.924273*x))));
}

double Hebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate)
{
	cbool neg = false;
	double delta;

	double topweight;

	if (maxweight<5.0) maxweight = 5.0;

	if (weight>maxweight) weight = maxweight;

	if (weight<-maxweight) weight = -maxweight;

	if (weight<0) {
		neg = true;
		weight = -weight;
	}

	topweight = weight + 2.0;
	if (topweight>maxweight) topweight = maxweight;

	if (!neg) {
		delta = hebb_rate*(maxweight - weight)*active_in*active_out + pre_rate*(topweight)*active_in*(active_out - 1.0);
		return weight + delta;
	}

	delta = pre_rate*(maxweight - weight)*active_in*(1.0 - active_out) + -hebb_rate*(topweight + 2.0)*active_in*active_out;
	return -(weight + delta);
}

double Random_Gauss() {
	static int iset = 0;
	static double gset;
	double fac, rsq, v1, v2;

	if (iset == 0) {
		do {
			v1 = 2.0*(Random_Float()) - 1.0;
			v2 = 2.0*(Random_Float()) - 1.0;
			rsq = v1*v1 + v2*v2;
		} while (rsq >= 1.0 || rsq == 0.0);
		fac = sqrt(-2.0*log(rsq) / rsq);
		gset = v1*fac;
		iset = 1;
		return v2*fac;
	}
	else {
		iset = 0;
		return gset;
	}
}

int Random_Int(int x, int y)
{ 
	return rand() % (y - x + 1) + x;
}

float Random_Float()
{
	return (float)rand() / (float)RAND_MAX;
}

void Quicksort(int first, int last, void** array, cbool(*sort_func)(void*, void*))
{
	if (first < last)
	{
		int i = first, pivot = i, j = last;

		while (i < j)
		{
			while (i < last && sort_func(array[pivot], array[i]))
				i++;
			while (j > first && sort_func(array[j], array[pivot]))
				j--;

			if (i < j)
			{
				void *temp = array[i];
				array[i] = array[j];
				array[j] = temp;
			}
		}

		void *temp = array[pivot];
		array[pivot] = array[j];
		array[j] = temp;

		Quicksort(first, j - 1, array, sort_func);
		Quicksort(j + 1, last, array, sort_func);
	}
}

cbool Quicksort_Ascending(double* x, double* y)
{
	return (*x >= *y);
}

cbool Quicksort_Descending(double* x, double* y)
{
	return (*x <= *y);
}

void TraceCopy(trace_t *a, trace_t *b)
{
	b->allsolid = a->allsolid;
	b->ent = a->ent;
	b->fraction = a->fraction;
	b->inopen = a->inopen;
	b->inwater = a->inwater;
	b->startsolid = a->startsolid;
	b->plane.dist = a->plane.dist;

	for (int i = 0; i < 3; i++)
	{
		b->endpos[i] = a->endpos[i];
		b->plane.normal[i] = a->plane.normal[i];
	}
}