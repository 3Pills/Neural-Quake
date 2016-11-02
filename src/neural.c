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
#include <math.h>
#include "population.h"
#include "organism.h"
#include "genome.h"
#include "species.h"
#include "r_neural.h"

population_t *population;

void Neural_Init() 
{
	population = Population_Init(Genome_Init_Auto(NQ_INPUT_COUNT, NQ_OUTPUT_COUNT, 0, 0), NQ_POP_SIZE);

	Con_Printf("Neural population successfully initialized!");
}

void Trace_Loop()
{

}

/*
void SV_NeuralThink(double frametime) 
{
	/*
	// Define directional vectors.
	vec3_t start, end, impact, direction;
	VectorCopy(r_refdef.vieworg, start);

	// Translate the player's view angles into directional vectors.
	vec3_t forward, right, up;
	AngleVectors(cl.viewangles, forward, right, up);

	// Move the end position forward by the falloff distance amount.
	VectorScale(forward, 200.0f, forward);
	VectorAdd(start, forward, end);

	// SV_Move returns a trace with all the data we need.
	if (sv_player != NULL)
	{
		/*
		trace = SV_Move(start, vec3_origin, vec3_origin, end, false, sv_player);

		if (trace.fraction != 1.0) // fraction is 1.0 if nothing was hit.
		{
			if (trace.ent->v.solid == SOLID_BSP) // We traced a world clip.
			{
				Con_Printf("Traced world! | Impact normal: [%f, %f, %f]\n", trace.plane.normal[0], trace.plane.normal[1], trace.plane.normal[2]);
			}
			else // It's an entity of some kind!
			{
				Con_Printf("Traced entity! | Entity class: %s | impact normal: [%f, %f, %f]\n", PR_GetString(trace.ent->v.classname), trace.plane.normal[0], trace.plane.normal[1], trace.plane.normal[2]);
			}
			//Con_Printf("Traceline impact! ent: %s, normal: %s", trace.ent->v.classname, trace.plane.normal);
		}

		double angX = 90 - r_fovx / 2, angY = 90 - r_fovy;
		double deltaX = r_fovx / NQ_INPUT_COLS, deltaY = r_fovy / NQ_INPUT_ROWS;

		for (int i = 0; i < NQ_INPUT_ROWS; i++)
		{
			cbool yMid = (i == (NQ_INPUT_ROWS + 1) / 2);

			for (int j = 0; j < NQ_INPUT_COLS; j++)
			{
				// Define directional vectors.
				vec3_t start, end, impact, direction;
				VectorCopy(r_refdef.vieworg, start);

				// Translate the player's view angles into directional vectors.
				vec3_t final_dir, forward, right, up;
				AngleVectors(cl.viewangles, forward, right, up);

				// Move the end position forward by the falloff distance amount.
				VectorScale(forward, 200.0f, forward);
				VectorAdd(start, forward, end);

				cbool xMid = (j == (NQ_INPUT_COLS + 1) / 2);


				TurnVector(final_dir, forward, up, angY + deltaY * i);
				TurnVector(final_dir, forward, right, angX + deltaX * j);


				trace_t trace = SV_Move(start, vec3_origin, vec3_origin, final_dir, false, sv_player);

				if (trace.fraction != 1.0) // fraction is 1.0 if nothing was hit.
				{
					if (trace.ent->v.solid == SOLID_BSP) // We traced a world clip.
					{
						//Con_Printf("Traced world! | Impact normal: [%f, %f, %f]\n", trace.plane.normal[0], trace.plane.normal[1], trace.plane.normal[2]);
					}
					else // It's an entity of some kind!
					{
						//Con_Printf("Traced entity! | Entity class: %s | impact normal: [%f, %f, %f]\n", PR_GetString(trace.ent->v.classname), trace.plane.normal[0], trace.plane.normal[1], trace.plane.normal[2]);
					}
					//Con_Printf("Traceline impact! ent: %s, normal: %s", trace.ent->v.classname, trace.plane.normal);
				}
				

				//int xFactor = (j < ceil(NQ_INPUT_COLS / 2)) ? -1 : (j == ceil(NQ_INPUT_COLS / 2)) ? 0 : 1;
				
				//TurnVector(final_dir, forward, right, r_fovx / 2 - cl.viewangles[Q_YAW]);
				//TurnVector(frustum[0].normal, vpn, vright, r_fovx / 2 - 90); //left plane
				//TurnVector(frustum[1].normal, vpn, vright, 90 - r_fovx / 2); //right plane
				//TurnVector(frustum[2].normal, vpn, vup, 90 - r_fovy / 2); //bottom plane
				//TurnVector(frustum[3].normal, vpn, vup, r_fovy / 2 - 90); //top plane
			}
		}
	}
}
*/

const int trace_length = 1000.0f;

void SV_NeuralThink(double frametime)
{

}

void NQ_Timeout()
{

	// the kill command reloads the level in single player.
	Cmd_ExecuteString("kill", src_client);
}

double timeout = 0;
int run_num = 0;
vector *inputs, *outputs;

void CL_NeuralThink(double frametime)
{
	//run_num++;
	//if (run_num > NQ_NUM_RUNS) return;
	if (sv.max_edicts == 0) return;

	//timeout += frametime;
	if (timeout > NQ_TIMEOUT)
		NQ_Timeout();

	species_t *currSpecies;
	genome_t *currGenome;


	int measured = 0;
	int total = 0;
	//for _, species in pairs(pool.species) do
	//	for _, genome in pairs(species.genomes) do
	//		total = total + 1
	//		if genome.fitness ~= 0 then
	//			measured = measured + 1
	//			end
	//			end
	//			end
}
/*
void CL_NeuralThink(double frametime)
{
	//Rotation delta per cell.
	double deltaX = r_fovx / NQ_INPUT_COLS, deltaY = r_fovy / NQ_INPUT_ROWS;

	//Starting angle offset from centre. 
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
		//Rotate final_dir to the final pitch.
		TurnVector(final_pitch, view_f, view_u, angY + deltaY * i);

		for (int j = 0; j < NQ_INPUT_COLS; j++)
		{
			//Calculate the new right vector using the new forward.
			CrossProduct(final_pitch, view_u, final_r);

			//Rotate forward to the final yaw and subsequently final direction.
			TurnVector(final_dir, final_pitch, final_r, angX + deltaX * j);

			// Scale the end direction forward by the falloff distance amount.
			VectorScale(final_dir, trace_length, final_dir);

			// Move the direction to world space and to the final pos.
			VectorAdd(final_dir, view_pos, final_pos);

			// If the client has entities, it will have the player.
			if (sv.max_edicts > 0)
			{
				// Complete a trace, ignoring EDICT_NUM(1), which will always be the client's player entity.
				trace_t trace = SV_Move(view_pos, vec3_origin, vec3_origin, final_pos, false, EDICT_NUM(1));

				// Calculate impact point.
				vec3_t impact;
				VectorScale(final_dir, trace.fraction, impact);
				VectorAdd(impact, view_pos, impact);

				// Calculate whether this trace is aimed at the crosshair.
				cbool mid = (i == (int)(NQ_INPUT_ROWS / 2) && j == (int)(NQ_INPUT_COLS / 2));

				if (trace.fraction != 1.0) // fraction is 1.0 if nothing was hit.
				{
					if (mid)
						Con_Printf("Trace | Entity class: %s | impact normal: [%f, %f, %f]\n", PR_GetString(trace.ent->v.classname), trace.plane.normal[0], trace.plane.normal[1], trace.plane.normal[2]);

					if (trace.ent->v.solid == SOLID_BSP) // We traced a world clip.
					{

					}
					else // It's an entity of some kind!
					{

					}
				}
			}
		}

	}
}
*/

void CL_NeuralMove() 
{
	//Execute Input Example: Cmd_ExecuteString("+forward", src_client);
	//Cmd_ExecuteString("+forward", src_client);

	/*
	// Simple jump controller. Jump every second... second.
	if ((int)cl.time % 2 == 0) 
	{
		Cmd_ExecuteString("+jump", src_client);
	}
	else 
	{
		Cmd_ExecuteString("-jump", src_client);
	}
	*/
}

void R_DrawNeuralData()
{
	// If the client doesn't have entities, it will not have the player to trace from. Return.
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
					// Right hasn't changed.
					VectorCopy(view_r, final_r);
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

			// The final color of the point to draw, as defined on the quake pallete.
			int c = 15;

			// Complete a trace, ignoring EDICT_NUM(1).
			// This will always be the client's player entity.
			trace_t trace = SV_Move(view_pos, vec3_origin, vec3_origin,
				final_pos, false, EDICT_NUM(1));

			// Calculate impact point.
			vec3_t impact;
			VectorScale(final_dir, trace.fraction, impact);
			VectorAdd(impact, view_pos, impact);

			if (!(midX && midY))
			{
				if (trace.fraction != 1.0) // fraction is 1.0 if nothing was hit.
				{
					if (trace.ent->v.solid == SOLID_BSP) // We traced a world clip.
					{
						c = 61;
					}
					else // It's an entity of some kind!
					{
						c = 79;
					}
				}
				else // Hit nothing
				{
					c = 40;
				}
			}

			R_DrawPoint(impact, max(8 * trace.fraction, 1), c);
		}
	}
}

void SCR_DrawNeuralData()
{
	if (!neuraldisplay.value) return;

	// Set to draw from the top left.
	Draw_SetCanvas(CANVAS_DEFAULT);

	//Draw_Line(0, 0, 400, 400, 4, (int)r_neural_color_1.value, 1);
	//Draw_Line(0, 0, 400, 200, 4, (int)r_neural_color_1.value, 1);
	//Draw_Line(0, 0, 200, 400, 4, (int)r_neural_color_1.value, 1);

	//for (int i = 0; i < 200; i++)
	//{
	//	Draw_Line((i * 4), (i * 4), (i * 4) + 1, (i * 4) + 1, 4, (int)r_neural_color_1.value, 1);
	//}

	//float squW = vid.client_window.width / NQ_INPUT_COLS;
	//float squH = vid.client_window.height / NQ_INPUT_ROWS;

	//for (int i = 0; i < NQ_INPUT_ROWS; i++)
	//{
	//	for (int j = 0; j < NQ_INPUT_COLS; j++)
	//	{
	//		if ((j + 1) * (i + 1) > NQ_INPUT_COUNT) break;
	//		Draw_Square(squW * j, squH * i, squW, squH, 2, 0, 40);
	//	}
	//}

	//Draw_Fill(400, 0, 80, 80, 45, 254);

	//Draw_Line(120, 0, 200, 0, 4, (int)r_neural_color_1.value, 1);
	//Draw_Square(200, 0, 80, 80, 2, (int)r_neural_color_2.value, 1);


	// Drawing Neural statistics here.
	Draw_SetCanvas(CANVAS_BOTTOMLEFT);

	int		y = 25 - 7.5; //9=number of lines to print
	int		x = 4; //margin
	char	str[40];

	Draw_Fill(0, y * 7.5, 25 * 9, 9 * 8, 0, 0.5); //dark rectangle
	
	c_snprintf2(str,	"Input Row/Col|  %4i %4i", NQ_INPUT_ROWS, NQ_INPUT_COLS);
	Draw_String(x, (y++) * 8 - x, str);

	c_strlcpy(str,		"NEURAL STATS |  Curr Best  ");
	Draw_String(x, (y++) * 8 - x, str);

	c_strlcpy(str,		"-------------+-------------");
	Draw_String(x, (y++) * 8 - x, str);
	
	c_snprintf2(str,	"Fitness      |  %4i %4i", 1, 1);
	Draw_String(x, (y++) * 8 - x, str);

	c_snprintf2(str,	"Genome       |  %4i %4i", 1, 1);
	Draw_String(x, (y++) * 8 - x, str);

	c_snprintf2(str,	"Species      |  %4i %4i", 1, 1);
	Draw_String(x, (y++) * 8 - x, str);

	c_snprintf2(str,	"Generation   |  %4i %4i", 1, 1);
	Draw_String(x, (y++) * 8 - x, str);

	c_snprintf2(str,	"Percentage   |  %4i %4i", 1, 1);
	Draw_String(x, (y++) * 8 - x, str);
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

int Random_Float()
{
	return (float)rand() / (float)RAND_MAX;
}

int Random_Sign() 
{
	return (rand() % 2) ? 1 : -1; 
}

void Quicksort(int first, int last, void** array, cbool(*sort_func)(void*, void*))
{
	if (first < last)
	{
		int i = first, pivot = i, j = last;

		while (i < j)
		{
			while (sort_func(array[pivot], array[i]))
				i++;
			while (sort_func(array[j], array[pivot]))
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