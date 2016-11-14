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

population_t *population;
vector *inputs; // Contains: trace_t*. A vector of input data gathered during the frame prior to 
double *outputs;

double timeout = 0.0;
double timestep = 0.0;

int run_num = 0;

genome_t *start_genome;
char curword[20];
int id;

vec3_t start_pos = { 0, 0, 0 }; // Position of the player spawn point.

// Position of the end of level trigger.
// TODO: Get the actual end-level trigger's position via engine, rather than hardcode it.
vec3_t end_pos = { 1, 1, 1 };

vector *distStorage; // Contains: vec3_t*. Stores all the final pos results of every genome ever.

//ostringstream *fnamebuf;

int evals[NQ_NUM_RUNS]; //Hold records for each run
int genes[NQ_NUM_RUNS];
int nodes[NQ_NUM_RUNS];
int winnernum;
int winnergenes;
int winnernodes;
//For averaging
int totalevals = 0;
int totalgenes = 0;
int totalnodes = 0;
int samples;  //For averaging

char currLevel[128]; // The current level's name. For determining the spawn point.
cbool gameLoaded = false;
cbool spawnSet = false;

void Neural_Init()
{
	Con_Printf("\nNeural population initialization\n");

	memset(evals, 0, NQ_NUM_RUNS * sizeof(int));
	memset(genes, 0, NQ_NUM_RUNS * sizeof(int));
	memset(nodes, 0, NQ_NUM_RUNS * sizeof(int));

	Con_Printf("   Spawning Population\n");
	genome_t *start_genome = Genome_Init_Auto(NQ_INPUT_COUNT, NQ_OUTPUT_COUNT, 0, 0);
	population = Population_Init(start_genome, NQ_POP_SIZE);

	Con_Printf("   Verifying Spawned Population\n");
	Population_Verify(population);

	network_t *network = Genome_Genesis(start_genome, population->organisms->count);
	Con_Printf("   Activating Network\n");
	Network_Activate(network);

	Con_Printf("   Building Inputs and Outputs\n");

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
		trace->plane.dist = 0;
		
		memset(trace->endpos, 0, 3 * sizeof(float));
		memset(trace->plane.normal, 0, 3 * sizeof(float));

		vector_add(inputs, trace);
	}

	distStorage = vector_init();

	outputs = malloc(sizeof(outputs)*NQ_OUTPUT_COUNT);
	memset(outputs, 0, NQ_OUTPUT_COUNT * sizeof(double*));

	//Population_Epoch(population, 1);

	/*
	for (int i = 0; i < network->inputs->count; i++)
	{
		neuron_t *node = network->inputs->data[i];
		node->value = inputs->data[i];
	}

	for (int i = 0; i < network->all_nodes->count; i++)
	{
		neuron_t *curnode = network->all_nodes->data[i];
		int sum = 0;
		for (int j = 0; j < curnode->links_in->count; j++)
		{
			nlink_t *link_in = curnode->links_in->data[j];
			neuron_t *node_in = link_in->inode;
			sum += link_in->weight * node_in->value;
		}

		if (curnode->links_in->count > 0) 
			curnode->value = Sigmoid(sum);
	}
	*/

	Con_Printf("Neural population initialized\n");
}

cbool timeoutFlag = false;

void Neural_Reload()
{

	gameLoaded = true;
	spawnSet = false;
}

const int trace_length = 1000.0f;

static int currSpecies = 0;
static int currOrganism = 0;
static int generation = 0;

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
	timeoutFlag = true;

	NQ_NextOrganism();
}

cbool win = false;

void NQ_NextOrganism()
{
	currOrganism++;

	species_t *species = population->species->data[currSpecies];
	organism_t *organism = species->organisms->data[currOrganism];

	// We've finished evaluating the population's organisms.
	if (currOrganism >= species->organisms->count)
	{
		for (int i = 0; i < population->species->count; i++)
		{
			Species_Compute_Average_Fitness(population->species->data[i]);
			Species_Compute_Max_Fitness(population->species->data[i]);
		}

		Population_Epoch(population, ++generation);
		currOrganism = 0;
	}

	// The reload the level in single player, otherwise kill the bot.
	if (m_state == m_singleplayer) 
		Cmd_ExecuteString(strcat("map ", cl.worldname), src_client);
	else if (m_state == m_multiplayer)
		Cmd_ExecuteString("kill", src_client);
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
					trace.plane.dist = 242 + (4.0 - DotProduct(up, trace.plane.normal) * 4);
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

// Used to determine timeout.
vec3_t prev_pos;

// Roughly the distance of a jump.
const double distance_to_timeout = 64.0;

void CL_NeuralThink(double frametime)
{
	if (population == NULL || !gameLoaded || cl.paused || key_dest != key_game) return;

	float timescale = (host_timescale.value == 0) ? 1 : host_timescale.value;

	if (!spawnSet)
	{
		VectorCopy(cl_entities[cl.viewentity].origin, start_pos);
		spawnSet = true;

		if (timeoutFlag)
		{
			timeoutFlag = false;
		}
		else // Add the spawn position of the player to check the sparseness of our results from.
		{
			vector_clear(distStorage);
			vec3_t *firstStorage = malloc(sizeof(vec3_t));

			*(*firstStorage+0) = cl_entities[cl.viewentity].origin[0];
			*(*firstStorage+1) = cl_entities[cl.viewentity].origin[1];
			*(*firstStorage+2) = cl_entities[cl.viewentity].origin[2];

			vector_add(distStorage, firstStorage);
		}
	}

	if (DistanceBetween2Points(prev_pos, cl_entities[cl.viewentity].origin) > distance_to_timeout)
	{
		timeout = 0;
		VectorCopy(cl_entities[cl.viewentity].origin, prev_pos);
	}
	else
	{
		timeout += frametime * timescale;
	}

	// Don't mess with the network if we die.
	if (cl.stats[STAT_HEALTH] > 0)
	{
		timestep += frametime * timescale;

		// Interact with network at 12FPS.
		if (timestep > (double)1/12)
		{

			species_t *species = population->species->data[currSpecies];
			organism_t *organism = species->organisms->data[currOrganism];

			NQ_GetInputs();
			NQ_Evaluate(organism);

			// Timeout after 2 seconds if we die.
			/*
			{
			double fitness = rightmost - pool.currentFrame / 2;

			if (fitness == 0) fitness = -1;
			organism->gnome->fitness = fitness;
			organism->fitness = fitness;
			if (fitness > pop->highest_fitness)
			{
			pop->highest_fitness = fitness;
			}

			if fitness > pool.maxFitness then
			pool.maxFitness = fitness
			forms.settext(maxFitnessLabel, "Max Fitness: " ..math.floor(pool.maxFitness))
			writeFile("backup." ..pool.generation .. "." ..forms.gettext(saveLoadFile))
			end

			console.writeline("Gen " ..pool.generation .. " species " ..pool.currentSpecies .. " genome " ..pool.currentGenome .. " fitness: " ..fitness)
			pool.currentSpecies = 1
			pool.currentGenome = 1
			while fitnessAlreadyMeasured() do
			nextGenome()
			end
			initializeRun()
			}
			*/
		}
	}
	else if (timeout + 2 < NQ_TIMEOUT)
	{
		timeout = NQ_TIMEOUT - 2;
	}

	if (sv_player != NULL && timeout >= NQ_TIMEOUT) NQ_Timeout();
}

// Output commands without +/- prefix. Prefix will be added 
// depending on the output of the network.
char* outputCmds[NQ_OUTPUT_COUNT] = {
	"forward", "back", "moveleft", "moveright",
	"left", "right", "lookup", "lookdown", "attack", "jump"
};

void CL_NeuralMove(usercmd_t *cmd) 
{
	// If the client doesn't have entities, it will 
	// not have the player to trace from. Return.
	if (!gameLoaded || sv.max_edicts == 0) return;

	// Timestep for input gathering and engine output.
	if (timestep < (double)1 / 12) return;

	// We need to disable all inputs if we're paused or otherwise not inputting to the game.
	if (cl.paused || key_dest != key_game)
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

	Con_Printf("Executing : [");
	// Execute movement commands based on the output results of the network.
	for (int i = 0; i < NQ_OUTPUT_COUNT; i++)
	{
		if (i > 0) Con_Printf(", ");
		char out_cmd[80];
		double output = outputs[i];
		strcpy(out_cmd, (output > 0.5) ? "+" : "-");
		strcat(out_cmd, outputCmds[i]);
		Cmd_ExecuteString(out_cmd, src_client);
		Con_Printf("%s", out_cmd);
	}
	Con_Printf("]\n");
}

int expcount = 0;

void R_DrawNeuralData()
{
	R_DrawPoint(prev_pos, 12, (DistanceBetween2Points(prev_pos, cl_entities[cl.viewentity].origin) > distance_to_timeout) ? 15 : 7);

	// Draw the impact point of the traces we gathered in NQ_GetInputs.
	for (int i = 0; i < inputs->count; i++)
	{
		trace_t* trace = inputs->data[i];
		// We use the otherwise unused plane.dist for debug color storage.
		R_DrawPoint(trace->endpos, fmax(8 * trace->fraction, 1), trace->plane.dist);
	}
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

		organism_t *curOrg = population->organisms->data[currOrganism];
		if (curOrg != 0)
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

			c_snprintf2(str, "Fitness      |  %4i %4i", (int)curOrg->fitness, (int)curOrg->species->peak_fitness);
			Draw_String(x, (y++) * 8 - x, str);

			c_snprintf2(str, "Genome       |  %4i %4i", currOrganism, bestOrgID);
			Draw_String(x, (y++) * 8 - x, str);

			c_snprintf2(str, "Species      |  %4i %4i", curOrg->species->id, bestSpeciesID);
			Draw_String(x, (y++) * 8 - x, str);

			c_snprintf2(str, "Generation   |  %4i %4i", curOrg->generation, population->winnergen);
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
	Draw_SetCanvas(CANVAS_TOPRIGHT);

	int y = 25 - 7.5;
	Draw_Fill(0, y * 7.5, 25 * 9, 9 * 8, 0, 0.5); //dark rectangle

	if (population != 0)
	{
		if (inputs != 0 && inputs->count > 0)
		{
			for (int i = 0; i < NQ_INPUT_COUNT; i++)
			{
				trace_t* trace = inputs->data[i];
				// We use the otherwise unused plane.dist for debug color storage.
				Draw_Square(i * 10, 0, 8, 8, 1, trace->plane.dist, 1);
			}
		}

		if (outputs != 0)
		{
			for (int i = 0; i < NQ_OUTPUT_COUNT; i++)
			{
				Draw_Square(i * 10, 80, 8, 8, 1, outputs[i] > 0.5 ? 251 : 248, 1);
			}
		}
	}
}

void SCR_DrawNeuralData()
{
	if (!neuraldisplay.value) return;

	Draw_NeuralGraph();
	Draw_NeuralStats();
}

/*
PSEUDO CODE

FLOWCHART:

START
|					
INITIALIZATION -----.	
|					|
GENETIC OPERATOR	|
[X Over, Mutation]	|
|					|
EVALUATION			|
|					|
REPRODUCTION		|
|					|
TERMINAL CONDITION -'
|
STOP

*/

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

	for (int i = 0; i < 3; i++)
	{
		b->endpos[i] = a->endpos[i];
		b->plane.normal[i] = a->plane.normal[i];
	}
}