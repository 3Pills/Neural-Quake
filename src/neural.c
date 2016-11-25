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
#include "vector.h"
#include "neural_def.h"
#include "population.h"

// Determines whether the network should be executing or not. 
// Set to true during NQ_Start.
static cbool network_on = false;

// Neural network population. Accessor to every neural network. 
static population_t *population = 0;

// Contains: trace_t*. A vector of input data gathered during NQ_GetInputs.
static vector *inputs = 0;

// Array of output values from the neural network.
static char *outputs = 0;

// Timer to restart the level when the AI idles or dies.
static double timeout = 0.0;

// Timer to restart the level when the AI is stuck in a small area.
static double timeout_moving = 0.0;

// Timer to space out neural processing, rather than doing it every frame.
static double timestep = 0.0;

// Trace length is about a small corridor's length.
// We keep it this short to keep the AI interested in 
// traces that end in empty space, i.e. a place to go.
static float trace_length = 325.0f;

// Index of the current species within the population.
static unsigned int curr_species = 0;

// Index of the current organism within the species.
static unsigned int curr_genome = 0;

// Position of the player spawn point.
static vec3_t start_pos = { 0, 0, 0 };

// Tracks the position change of the player. Used to determine idle timeout.
static vec3_t prev_short_pos = { 0, 0, 0 };

// Tracks the position change of the player. Used to determine moving timeout.
static vec3_t prev_long_pos = { 0, 0, 0 };

// Distance to travel before prev_pos is updated.
// Roughly the distance of a jump.
static double distance_to_timeout = 56.0;
static double distance_to_timeout_moving = 215.0;

// Contains: vec3_t*. Stores all the final pos results of every genome ever.
static vector *distance_storage = 0;

// Spawn flag to determine when to reload
static cbool spawn_set = false;

// Name of the currently loaded level.
// Tracks whether the level has changed or not.
static char level_name[64];

// Output commands without +/- prefix. 
// Prefix will be added depending on the output of the network.
static char* output_cmds[NQ_OUTPUT_COUNT] = {
	"forward", "back", "moveleft", "moveright",
	"left", "right", "lookup", "lookdown", "attack", "jump"
};

// Contains: uinode_t. Stores nodes for display in the neural graph.
static vector *uinodes = 0;

// Contains: uilink_t. Stores links for display in the neural graph.
static vector *uilinks = 0;

void NQ_Init()
{
	Cmd_AddCommands(NQ_Init);
}

void NQ_Reload()
{
	if (strcmp(level_name, cl.worldname) != 0)
	{
		spawn_set = false;
		strcpy(level_name, cl.worldname);
	}
}

void CL_NeuralThink(double frametime)
{
	if (!network_on || population == NULL || cl.paused || key_dest != key_game) return;

	float timescale = (host_timescale.value == 0) ? 1 : host_timescale.value;

	if (!spawn_set)
	{
		VectorCopy(cl_entities[cl.viewentity].origin, start_pos);
		spawn_set = true;

		// Add the spawn position of the player to check the sparseness of our results from.
		for (unsigned int i = 0; i < distance_storage->count; i++)
			free(distance_storage->data[i]);

		vector_free(distance_storage);

		vec3_t *first_storage = (vec3_t*)malloc(sizeof(vec3_t));

		*(*first_storage + 0) = cl_entities[cl.viewentity].origin[0];
		*(*first_storage + 1) = cl_entities[cl.viewentity].origin[1];
		*(*first_storage + 2) = cl_entities[cl.viewentity].origin[2];

		vector_push(distance_storage, first_storage);
	}

	// Only gather input when the player is alive.
	if (cl.stats[STAT_HEALTH] > 0)
	{
		timestep += frametime * timescale;

		// Interact with network at 12FPS.
		if (timestep > (double)1 / 12)
		{
			// Evaluate the input, passing it into the current organism within the current species.
			NQ_Evaluate(((species_t*)population->species->data[curr_species])->genomes->data[curr_genome]);
		}

		if (DistanceBetween2Points(prev_short_pos, cl_entities[cl.viewentity].origin) > distance_to_timeout)
		{
			timeout = 0;
			VectorCopy(cl_entities[cl.viewentity].origin, prev_short_pos);
		}

		if (DistanceBetween2Points(prev_long_pos, cl_entities[cl.viewentity].origin) > distance_to_timeout_moving)
		{
			timeout_moving = 0;
			VectorCopy(cl_entities[cl.viewentity].origin, prev_long_pos);
		}
	}
	// Timeout after two seconds once we die.
	else if (timeout + 2 < NQ_TIMEOUT)
	{
		timeout = NQ_TIMEOUT - 2;
	}

	if (sv_player != NULL && (timeout >= NQ_TIMEOUT || timeout_moving >= NQ_TIMEOUT_MOVING)) NQ_Timeout();

	// Add to timeout after the frame has processed. 
	// Allows for high timescales without immediately exceeding the timeout value.
	timeout += frametime * timescale;
	timeout_moving += frametime * timescale;
}

void CL_NeuralMove(usercmd_t *cmd)
{
	// If the client doesn't have entities, it will 
	// not have the player to trace from. Return.
	if (!network_on || cl.maxclients == 0) return;

	// Timestep for input gathering and engine output.
	if (timestep < (double)1 / 12) return;

	// We need to disable all inputs if we're paused or otherwise not inputting to the game.
	if (cl.paused || key_dest != key_game) return;

	timestep = 0;

	species_t *species = population->species->data[curr_species];
	genome_t *genome = species->genomes->data[curr_genome];

	// Execute movement commands based on the output results of the network.
	for (int i = 0; i < genome->num_out; i++)
	{
		char output_cmd[80];
		strcpy(output_cmd, (outputs[i] == 1) ? "+" : "-");
		strcat(output_cmd, output_cmds[i]);
		Cmd_ExecuteString(output_cmd, src_client);
	}
}

void R_DrawNeuralData()
{
	if (!network_on || inputs == NULL) return;

	R_DrawPoint(prev_short_pos, 12, 7);
	R_DrawPoint(prev_long_pos, 12, 12);

	// Draw the impact point of the traces we gathered in NQ_GetInputs.
	for (unsigned int i = 0; i < inputs->count; i++)
	{
		trace_t* trace = inputs->data[i];
		// We use the otherwise unused plane.dist for debug color storage.
		R_DrawPoint(trace->endpos, fmax(2 * trace->fraction, 0.25), trace->plane.dist);
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
	// If the network is on we don't need to initialize it!
	if (network_on) return;

	// Attempt to load filename if its passed in as argument.
	if (line != 0 && line->count == 2)
	{
		NQ_Load(line);
		// Population will be NULL if the load fails.
		if (population == 0) return;
	}

	Con_Printf("\nNeural population initialization\n");
	
	genome_t *genome = 0;
	if (population == 0)
	{
		Con_Printf("  Spawning Population\n");
		genome_t *base_genome = Genome_Init_Auto(NQ_INPUT_COUNT, NQ_OUTPUT_COUNT, 0, 0);
		population = Population_Init(base_genome, NQ_POP_SIZE);
		Genome_Delete(base_genome);

		Con_Printf("  Generating First Genome with Population\n");
		species_t *species = population->species->data[curr_species];
		genome = species->genomes->data[curr_genome];
		Genome_Genesis(genome);

		Con_Printf("  Activating Genome's Neural Network\n");
		Genome_Activate(genome);
	}
	else
	{
		species_t *species = population->species->data[curr_species];
		genome = species->genomes->data[curr_genome];
	}

	if (inputs == 0 || outputs == 0)
		Con_Printf("  Building Inputs and Outputs\n");

	if (inputs == 0)
	{
		inputs = vector_init();

		// Initialize a bunch of trace objects to store the data from input gathering in.
		for (int i = 0; i < genome->num_in - 1; i++)
		{
			trace_t* trace = (trace_t*)malloc(sizeof(trace_t));
			trace->allsolid = false;
			trace->startsolid = false;
			trace->ent = 0;
			trace->fraction = 1.0;
			trace->inopen = false;
			trace->inwater = false;
			trace->plane.dist = 10;

			memset(trace->endpos, 0, 3 * sizeof(float));
			memset(trace->plane.normal, 0, 3 * sizeof(float));

			vector_push(inputs, trace);
		}
	}

	spawn_set = false;
	if (distance_storage == 0)
		distance_storage = vector_init();
	else
		vector_clear(distance_storage);

	if (outputs == 0)
	{ 
		outputs = (char*)malloc(sizeof(*outputs) * genome->num_out);
		memset(outputs, 0, genome->num_out * sizeof(*outputs));
	}

	Con_Printf("  Building UI Graph Data\n");

	if (uinodes == 0)
	{
		uinodes = vector_init();
		uinode_t *uinode = 0;

		// Create input UI Nodes for the graph.
		for (int i = 0; i < genome->num_in - 1; i++)
		{
			int x = i % NQ_INPUT_COLS, y = (NQ_INPUT_ROWS - 1) - (i / NQ_INPUT_COLS);

			uinode = (uinode_t*)malloc(sizeof(uinode_t));

			uinode->x = NQ_GRAPH_POSX + 1 + x * NQ_GRAPH_INBOX_WIDTH;
			uinode->y = NQ_GRAPH_POSY + y * NQ_GRAPH_INBOX_HEIGHT;
			uinode->sizex = NQ_GRAPH_INBOX_WIDTH - NQ_GRAPH_BOX_PADDING;
			uinode->sizey = NQ_GRAPH_INBOX_HEIGHT - NQ_GRAPH_BOX_PADDING;
			uinode->color = 63;

			vector_push(uinodes, uinode);
		}
		// Create the bias node
		uinode = (uinode_t*)malloc(sizeof(uinode_t));

		uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH / 2 - NQ_GRAPH_INBOX_WIDTH / 2;
		uinode->y = NQ_GRAPH_POSY + (NQ_INPUT_ROWS+0.25) * NQ_GRAPH_INBOX_HEIGHT;
		uinode->sizex = NQ_GRAPH_INBOX_WIDTH - NQ_GRAPH_BOX_PADDING;
		uinode->sizey = NQ_GRAPH_INBOX_HEIGHT - NQ_GRAPH_BOX_PADDING;
		uinode->color = 63;

		vector_push(uinodes, uinode);

#pragma region output_uinodes

		// Create all the output nodes now.
		for (int i = genome->num_in; i < genome->num_in + genome->num_out; i++)
		{
			uinode = (uinode_t*)malloc(sizeof(uinode_t));

			uinode->x = 0;
			uinode->y = 0;
			uinode->sizex = NQ_GRAPH_OUTBOX_SIZE;
			uinode->sizey = NQ_GRAPH_OUTBOX_SIZE;

			vector_push(uinodes, uinode);
		}

		double y_anchor = NQ_GRAPH_POSY + NQ_GRAPH_OUTPUT_Y;
		// forward
		uinode = uinodes->data[genome->num_in];
		uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.025 + NQ_GRAPH_OUTBOX_SIZE;
		uinode->y = y_anchor - NQ_GRAPH_OUTBOX_SIZE * 2;

		// back
		uinode = uinodes->data[genome->num_in + 1];
		uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.025 + NQ_GRAPH_OUTBOX_SIZE;
		uinode->y = y_anchor;

		// moveleft
		uinode = uinodes->data[genome->num_in + 2];
		uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.025;
		uinode->y = y_anchor - NQ_GRAPH_OUTBOX_SIZE;

		// moveright
		uinode = uinodes->data[genome->num_in + 3];
		uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.025 + NQ_GRAPH_OUTBOX_SIZE * 2;
		uinode->y = y_anchor - NQ_GRAPH_OUTBOX_SIZE;

		// left
		uinode = uinodes->data[genome->num_in + 4];
		uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.975 - NQ_GRAPH_OUTBOX_SIZE * 3;
		uinode->y = y_anchor - NQ_GRAPH_OUTBOX_SIZE;

		// right
		uinode = uinodes->data[genome->num_in + 5];
		uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.975 - NQ_GRAPH_OUTBOX_SIZE;
		uinode->y = y_anchor - NQ_GRAPH_OUTBOX_SIZE;

		// lookup
		uinode = uinodes->data[genome->num_in + 6];
		uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.975 - NQ_GRAPH_OUTBOX_SIZE * 2;
		uinode->y = y_anchor - NQ_GRAPH_OUTBOX_SIZE * 2;

		// lookdown
		uinode = uinodes->data[genome->num_in + 7];
		uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.975 - NQ_GRAPH_OUTBOX_SIZE * 2;
		uinode->y = y_anchor;

		// attack
		uinode = uinodes->data[genome->num_in + 8];
		uinode->x = NQ_GRAPH_POSX + (NQ_GRAPH_WIDTH * 0.5 - NQ_GRAPH_OUTBOX_SIZE / 2) - NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING;
		uinode->y = y_anchor - (NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING) / 2;

		// jump
		uinode = uinodes->data[genome->num_in + 9];
		uinode->x = NQ_GRAPH_POSX + (NQ_GRAPH_WIDTH * 0.5 - NQ_GRAPH_OUTBOX_SIZE / 2) + NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING;
		uinode->y = y_anchor - (NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING) / 2;

#pragma endregion

		// Add all extra hidden nodes.
		for (unsigned int i = genome->num_in + genome->num_out; i < genome->neurons->count; i++)
		{
			uinode = (uinode_t*)malloc(sizeof(uinode_t));
			vector_push(uinodes, uinode);
		}
	}
	
	if (uilinks == 0)
	{
		uilinks = vector_init();
	}

	// Initialize hidden / bias node positions and represent their neural links. 
	UI_RefreshGraph(genome);

	// Network_Flush(network);

	Con_Printf("Neural population initialized\n");

	network_on = true;

	// Reload the level on start.
	char map_cmd[128] = "map ";

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

void NQ_End(lparse_t *line)
{
	// If the network is already off then don't do anything.
	if (!network_on) return;

	if (line != 0 && line->count == 2)
		NQ_Save(line);

	// Disable all key inputs on end.
	for (int i = 0; i < NQ_OUTPUT_COUNT; i++)
	{
		char output_cmd[80];
		strcpy(output_cmd, "-");
		strcat(output_cmd, output_cmds[i]);
		Cmd_ExecuteString(output_cmd, src_client);
	}

	// Memory cleanup crew, gettin our hands dirty...!
	Population_Delete(population);
	population = NULL;

	for (unsigned int i = 0; i < inputs->count; i++)
		free(inputs->data[i]);
	vector_free_all(inputs);
	inputs = NULL;

	free(outputs);
	outputs = NULL;

	for (unsigned int i = 0; i < uinodes->count; i++)
		free(uinodes->data[i]);
	vector_free_all(uinodes);
	uinodes = NULL;

	for (unsigned int i = 0; i < uilinks->count; i++)
		free(uilinks->data[i]);
	vector_free_all(uilinks);
	uilinks = NULL;

	for (unsigned int i = 0; i < distance_storage->count; i++)
		free(distance_storage->data[i]);
	vector_free_all(distance_storage);
	distance_storage = NULL;

	curr_genome = 0;
	curr_species = 0;
	network_on = false;
}

void NQ_Save(lparse_t *line)
{
	if (line == 0) return;

	if (line->count != 2)
	{
		Con_Printf("nq_save [filename] : saves a file containing neural data.");
		return;
	}

	char filename[256];

	// Neural save data will be stored in neural_backups subfolder.
	strcpy(filename, "./neural_backups/");
	strcat(filename, line->args[1]);
	strcat(filename, ".nq");

	FILE *f = fopen(filename, "w");
	if (f == NULL)
	{
		Con_Printf("Unable to open %s", filename);
		return;
	}

	Population_Save(population, f);
	fclose(f);
}

void NQ_Load(lparse_t *line)
{
	if (line == 0) return;

	if (population != NULL)
	{
		Con_Printf("Neural AI Busy! Run nq_end before nq_load!");
		return;
	}

	if (line->count != 2)
	{
		Con_Printf("nq_load [filename] : loads a file containing neural data.");
		return;
	}

	Con_Printf("Loading population from file \"%s\"...\n", line->args[1]);

	char filename[256];
	// Neural save data is stored in neural_backups subfolder.
	strcpy(filename, "./neural_backups/");
	strcat(filename, line->args[1]);
	strcat(filename, ".nq");

	FILE *f = fopen(filename, "r");
	if (f == NULL)
	{
		Con_Printf("No neural data found in %s\n", filename);
		return;
	}
	
	// Read in the global header data.
	char curline[1024];
	if (fgets(curline, sizeof(curline), f) != NULL)
		population = Population_Init_Load(f);

	if (!feof(f))
	{
		Con_Printf("Error loading data from %s!\n", filename);
		Population_Delete(population);
		population = NULL;
		fclose(f);
		return;
	}
	fclose(f);


	Con_Printf("Generating First Genome with Population\n");
	species_t *species = population->species->data[0];
	genome_t *genome = species->genomes->data[0];
	Genome_Genesis(genome);

	Con_Printf("Activating Genome's Neural Network\n");
	Genome_Activate(genome);


	Con_Printf("Population sucessfully loaded from %s!\n", filename);
}

void NQ_ForceTimeout()
{
	if (!network_on) return;
	NQ_Timeout();
}

void NQ_GetInputs(double *values)
{
	// If the client doesn't have entities, it will 
	// not have the player to trace from. Return.
	if (cl.maxclients == 0) return;

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
			trace.plane.dist = 40;

			// Initialize a default node_value of -1.0. This denotes empty space.
			double node_value = -1.0;

			if (trace.fraction != 1.0) // fraction is 1.0 if nothing was hit.
			{
				if (trace.ent->v.solid == SOLID_BSP) // We traced a world clip.
				{
					vec3_t up = { 0, 0, 1 };

					// We set floors and ceilings to a range between -1 and 0.
					// We want them to illicate the opposite response to empty space
					// (Avoid and look away from floors and ceilings).
					node_value = (-fabs(DotProduct(up, trace.plane.normal)) + 0.5) * 1.5;

					// We also want to take into account the distance from these
					// walls, so the further away the wall, the lower the input.
					node_value *= (1.0 - trace.fraction);

					trace.plane.dist = (int)(239 + node_value * 3);
				}
				else // It's an entity of some kind!
				{
					node_value = -1.0;
					trace.plane.dist = 79;
				}
				/*
				// If we're at 5 specific nodes, look for entities, rather than boundaries.
				if ((midX && (i == (int)(NQ_INPUT_ROWS * 0.5) || i == (int)(NQ_INPUT_ROWS * 0.4) || i == (int)(NQ_INPUT_ROWS * 0.6))) ||
					(midY && (j == (int)(NQ_INPUT_COLS * 0.5) || j == (int)(NQ_INPUT_COLS * 0.4) || j == (int)(NQ_INPUT_COLS * 0.6))))
				{
					node_value = 1.0;
					trace.plane.dist = 79;
				}

				else
				{

				}
				*/
			}

			values[j + NQ_INPUT_COLS*i] = node_value;

			// Copy trace data to the input data for the frame, so we can draw it later.
			TraceCopy(&trace, inputs->data[j + NQ_INPUT_COLS*i]);
		}
	}
}

void NQ_Evaluate(genome_t* genome)
{
	// Error Handling for null organism.
	if (genome == NULL)
	{
		Con_Printf("NQ ERROR: ATTEMPTED TO EVALUATE ERRONEOUS GENOME.");
		return;
	}
	/***** INPUT RETRIEVING *****/

	// New values of each node to be passed into the network.
	double *input_values = malloc(sizeof(*input_values) * genome->num_in);

	// Retrieve information on what the AI can see,
	// passing it as a double into input_values.
	NQ_GetInputs(input_values);

	/***** NETWORK PROCESSING *****/

	// Add the input values to the network.
	Genome_Load_Inputs(genome, input_values);
	free(input_values);

	Genome_Activate(genome);

	// Take the value of each output in the network into the global output vector for use with inputs.
	for (unsigned int i = genome->num_in; i < (unsigned int)(genome->num_in + genome->num_out); i++)
		outputs[i - genome->num_in] = round(((neuron_t*)genome->neurons->data[i])->value);

	// Process output value into the graph HUD element.
	for (unsigned int i = 0; i < uilinks->count; i++)
	{
		uilink_t *uilink = uilinks->data[i];
		gene_t *gene = genome->genes->data[uilink->gene];
		uilink->opacity = !gene->enabled ? 0 : fmin(fmax(fabs(((neuron_t*)genome->neurons->data[gene->inode])->value) * 150, 24), 150);
	}

	/***** FITNESS EVALUATION *****/

	// Create a distance array to store the distances for our novelty search algorithm.
	unsigned int distCount = distance_storage->count;

	// Stored as a pointer array to allow compatibility with Quicksort function.
	double **distList = malloc(distCount * sizeof(*distList));

	// First get the distance of the point to the rest of the population.
	for (unsigned int i = 0; i < distCount; i++)
	{
		double* distance = malloc(sizeof(*distance));
		vec3_t* other = distance_storage->data[i];
		*distance = (double)DistanceBetween2Points(cl_entities[cl.viewentity].origin, *other);
		distList[i] = distance;
	}

	// Sort the list to get the closest distances.
	Quicksort(0, distCount - 1, distList, Quicksort_Ascending);

	double fitness = 0.0;

	// Compute the sparseness of the point. The average distance away from the genome's point.
	double sparseness = 0.0;
	for (unsigned int i = 0; i < (unsigned int)fmin(NQ_NOVELTY_COEFF, distCount); i++)
	{
		sparseness += *distList[i];
		free(distList[i]);
	}

	free(distList);

	sparseness /= NQ_NOVELTY_COEFF;

	// Assign the sparseness as the base fitness value.
	fitness = sparseness;

	// Increment the fitness value based on a number of stats.
	fitness += cl.stats[STAT_MONSTERS] * 100; // Monster kills. Single player.
	fitness += cl.stats[STAT_SECRETS] * 250; // Secrets found. Single player.
	fitness += cl.stats[STAT_FRAGS] * 500; // Player kills. Multi player.

	// Lower the final fitness value by a maximum of 80% based on the player's health.
	fitness = sparseness * (1 - ((1 - cl.stats[STAT_HEALTH] / 100) * 0.8));

	genome->fitness = fitness;
}

void NQ_NextOrganism()
{
	species_t *species = population->species->data[curr_species];

	Genome_Clear_Nodes(species->genomes->data[curr_genome]);

	curr_genome++;
	// We've finished evaluating a species' genomes.
	if (curr_genome >= species->genomes->count)
	{
		// Move on to the next species.
		curr_species++;
		curr_genome = 0;

		// Reset the spawn flag, so the position list will be reset on respawn.
		spawn_set = false;

		// If there is no next species, the generation shall EVOLVE!
		if (curr_species >= population->species->count)
		{
			curr_species = 0;

			Population_Epoch(population);

			// Save the newly created generation. 
			// We only need the structures to reload a generation, not their results.
			char buffer[255];
			char final_cmd[255];
			sprintf(buffer, "nq_save \"%%Y-%%m-%%d %%H-%%M-%%S %f\"", population->max_fitness);

			const time_t cur_time = time(NULL);
			strftime(final_cmd, 255, buffer, localtime(&cur_time));
			lparse_t *line = Line_Parse_Alloc(final_cmd, false);
			NQ_Save(line);
			Line_Parse_Free(line);
		}
	}

	species = population->species->data[curr_species];

	// Generate neurons for next genome to be evaluated.
	Genome_Genesis(species->genomes->data[curr_genome]);

	// Refresh the graph with the new genome.
	UI_RefreshGraph(species->genomes->data[curr_genome]);

	char map_cmd[128] = "map ";

	// The reload the level in single player, otherwise kill the bot.
	if (cl.maxclients == 1)
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
	species_t *species = population->species->data[curr_species];
	genome_t *genome = species->genomes->data[curr_genome];

	// Add the in-game clock timer to the organism when done.
	// genome->time_alive = cl.time;

	// Store the player's current position as the organisms final position.
	VectorCopy(cl_entities[cl.viewentity].origin, genome->final_pos);

	// Add the final position of the organism to the global list.
	vec3_t *final_pos = malloc(sizeof(*final_pos));

	*(*final_pos + 0) = cl_entities[cl.viewentity].origin[0];
	*(*final_pos + 1) = cl_entities[cl.viewentity].origin[1];
	*(*final_pos + 2) = cl_entities[cl.viewentity].origin[2];

	vector_push(distance_storage, final_pos);

	timeout = 0;
	timeout_moving = 0;
	timestep = 0;

	// Disable all inputs on timeout.
	for (unsigned char i = 0; i < NQ_OUTPUT_COUNT; i++)
	{
		char output_cmd[80];
		strcpy(output_cmd, "-");
		strcat(output_cmd, output_cmds[i]);
		Cmd_ExecuteString(output_cmd, src_client);
	}

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
		c_snprintf2(str, "Input Row/Col |  %4i %4i", NQ_INPUT_ROWS, NQ_INPUT_COLS);
		Draw_String(x, (y++) * 8 - x, str);

		c_strlcpy(str, "NEURAL STATS  |  Curr Total ");
		Draw_String(x, (y++) * 8 - x, str);

		c_strlcpy(str, "--------------+-------------");
		Draw_String(x, (y++) * 8 - x, str);

		species_t *species = population->species->data[curr_species];
		genome_t *genome = species->genomes->data[curr_genome];
		if (genome != 0)
		{
			// Get stats from the population to enter into the graph.
			int best_species_id = 0; // The best species in the population.
			int best_genome_id = 0; // The champion organism in the population.

			species_t* best_species = 0;
			for (unsigned int i = 0; i < population->species->count; i++)
			{
				species_t *species = population->species->data[i];

				if (best_species == 0 || best_species->peak_fitness < species->peak_fitness)
				{
					best_species = species;
					best_species_id = i;
				}
			}
			//for (int i = 0; i < population->genomes->count; i++)
			//{
			//	genome_t *cur_genome = population->genomes->data[i];
			//	//if (cur_genome->champion) best_genome_id = i;
			//}

			//c_snprintf2(str, "Time         |  %4i %4i", (int)organism->time_alive, (int)bestOrgTime);
			//Draw_String(x, (y++) * 8 - x, str);

			c_snprintf(str,  "Fitness       |  %4i", genome->fitness);
			Draw_String(x, (y++) * 8 - x, str);

			c_snprintf2(str, "Genome        |  %4i %4i", curr_genome + 1, species->genomes->count);
			Draw_String(x, (y++) * 8 - x, str);

			c_snprintf2(str, "Species       |  %4i %4i", curr_species + 1, population->species->count);
			Draw_String(x, (y++) * 8 - x, str);

			c_snprintf2(str, "Generation    |  %4i %4i", population->generation + 1, ((population->winner_generation != 0) ? population->winner_generation + 1 : 0));
			Draw_String(x, (y++) * 8 - x, str);

			//c_snprintf2(str,	"Percentage   |  %4i %4i", 1, 1);
			//Draw_String(x, (y++) * 8 - x, str);
		}
		else
		{
			c_strlcpy(str, "Genome       |   -  -");
			Draw_String(x, (y++) * 8 - x, str);

			c_strlcpy(str, "Species      |   -  -");
			Draw_String(x, (y++) * 8 - x, str);

			c_strlcpy(str, "Fitness      |   -  -");
			Draw_String(x, (y++) * 8 - x, str);

			c_strlcpy(str, "Generation   |   -  -");
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
		Draw_Fill(NQ_GRAPH_POSX, NQ_GRAPH_POSY, NQ_GRAPH_WIDTH, NQ_GRAPH_HEIGHT, 0, 0.5);

		species_t *species = population->species->data[curr_species];
		genome_t *genome = species->genomes->data[curr_genome];

		// Draw all nodes in the network here.
		if (uinodes != 0 && uinodes->count > 0)
		{
			if (inputs != 0 && inputs->count > 0)
			{
				for (int i = 0; i < genome->num_in; i++)
				{
					uinode_t *uinode = uinodes->data[i];
					trace_t *trace = inputs->data[i];

					Draw_Square(uinode->x, uinode->y, uinode->sizex, uinode->sizey, 1, (i != genome->num_in-1) ? trace->plane.dist : uinode->color, 1.0f);
				}
			}

			// Draw all nodes between the input and the output.
			for (unsigned int i = genome->num_in + genome->num_out; i < uinodes->count; i++)
			{
				uinode_t *uinode = uinodes->data[i];
				if (uinode != 0) Draw_Square(uinode->x, uinode->y, uinode->sizex, uinode->sizey, 1, uinode->color, 1.0f);
			}

			// Draw each output in spots resembling a controller
			if (outputs != 0)
			{
				// Draw output nodes.
				for (unsigned int i = genome->num_in; i < (unsigned int)(genome->num_in + genome->num_out); i++)
				{
					uinode_t *uinode = uinodes->data[i];
					Draw_Square(uinode->x, uinode->y, uinode->sizex, uinode->sizey, 1, outputs[i - genome->num_in] > 0.5 ? 251 : 248, 1.0f);
				}

				// Draw output labels.
				Draw_String(NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.025 + NQ_GRAPH_OUTBOX_SIZE * 0.85, NQ_GRAPH_POSY + NQ_GRAPH_OUTPUT_Y + NQ_GRAPH_OUTBOX_SIZE * 1.15, "Move");
				Draw_String(NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.5 - (NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING) * 2.25, NQ_GRAPH_POSY + NQ_GRAPH_OUTPUT_Y + NQ_GRAPH_OUTBOX_SIZE * 0.65, "Attack");
				Draw_String(NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.5 + (NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING) * 0.35, NQ_GRAPH_POSY + NQ_GRAPH_OUTPUT_Y + NQ_GRAPH_OUTBOX_SIZE * 0.65, "Jump");
				Draw_String(NQ_GRAPH_POSX + NQ_GRAPH_WIDTH * 0.975 - NQ_GRAPH_OUTBOX_SIZE * 2.25, NQ_GRAPH_POSY + NQ_GRAPH_OUTPUT_Y + NQ_GRAPH_OUTBOX_SIZE * 1.15, "Look");
			}
		}

		// Draw all existing links between hidden nodes in the network. 
		if (uilinks != 0 && uilinks->count > 0)
		{
			for (unsigned int i = 0; i < uilinks->count; i++)
			{
				uilink_t *uilink = uilinks->data[i];
				uinode_t *inode = uinodes->data[uilink->inode];
				uinode_t *onode = uinodes->data[uilink->onode];
				Draw_Line(inode->x + (NQ_GRAPH_INBOX_WIDTH - NQ_GRAPH_BOX_PADDING) / 2, inode->y + (NQ_GRAPH_INBOX_HEIGHT - NQ_GRAPH_BOX_PADDING) / 2,
						  onode->x + (NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING) / 2, onode->y + (NQ_GRAPH_OUTBOX_SIZE - NQ_GRAPH_BOX_PADDING) / 2, 1, uilink->color, (float)uilink->opacity/255.0f);
			}
		}
	}
}

void UI_RefreshGraph(genome_t *genome)
{
	// Remove all hidden UI nodes from the vector. 
	// We will re-add them /IF/ they still exist.
	for (int i = uinodes->count - 1; i > genome->num_in + genome->num_out; i--)
	{
		free(uinodes->data[i]);
		uinodes->data[i] = NULL;
		uinodes->count--;
	}

	// Add all existing hidden layer nodes to uinodes.
	for (unsigned int i = genome->num_in + genome->num_out; i < genome->neurons->count; i++)
	{
		uinode_t *uinode = (uinode_t*)malloc(sizeof(uinode_t));
		vector_push(uinodes, uinode);

		// Calculate the nodes position within the grid.
		int x = ((i - (genome->num_in + genome->num_out)) % NQ_INPUT_COLS),
			y = (i - (genome->num_in + genome->num_out)) / NQ_INPUT_COLS;

		// Update our hidden layer nodes variables.
		uinode->x = NQ_GRAPH_POSX + NQ_GRAPH_BOX_PADDING + x * NQ_GRAPH_INBOX_WIDTH;
		uinode->y = NQ_GRAPH_POSY + NQ_GRAPH_HIDDEN_Y + NQ_GRAPH_BOX_PADDING + y * NQ_GRAPH_INBOX_HEIGHT;
		uinode->sizex = NQ_GRAPH_INBOX_WIDTH - NQ_GRAPH_BOX_PADDING;
		uinode->sizey = NQ_GRAPH_INBOX_HEIGHT - NQ_GRAPH_BOX_PADDING;

		// Hidden nodes will be a soft green / yellow.
		uinode->color = 63;
	}

	// Clear out genes from previous genome.
	for (unsigned int i = 0; i < uilinks->count; i++)
		free(uilinks->data[i]);

	vector_free(uilinks);

	// Add the genes from the current genome in.
	for (unsigned int i = 0; i < genome->genes->count; i++)
	{
		gene_t *gene = genome->genes->data[i];
		uilink_t *uilink = (uilink_t*)malloc(sizeof(uilink_t));

		uilink->inode = gene->inode;
		uilink->onode = gene->onode;
		uilink->color = gene->weight > 0 ? 63 : gene->weight < 0 ? 79 : 7;
		uilink->opacity = 24;
		uilink->gene = i;

		vector_push(uilinks, uilink);
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
	return x == y ? x : rand() % (y - x + 1) + x;
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