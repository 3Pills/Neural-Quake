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
#include "neuron.h"
#include "r_neural.h"

void Neural_Init() 
{
	Con_Printf("Neural AI successfully initialized!");
}

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
	*/

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
		*/

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

void CL_NeuralThink(double frametime)
{
	// Define directional vectors.
	vec3_t start, end, impact, direction;
	VectorCopy(r_refdef.vieworg, start);

	// Translate the player's view angles into directional vectors.
	vec3_t forward, right, up;
	AngleVectors(cl.viewangles, forward, right, up);

	// Move the end position forward by the falloff distance amount.
	VectorScale(forward, 200.0f, forward);
	VectorAdd(start, forward, end);

	trace_t trace;
	TraceLine(start, end, impact);
}

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

const int trace_length = 500.0f;

void R_DrawNeuralData()
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

	const int size = 8;

	TraceLine(start, end, impact);

	if (impact[0] || impact[1] || impact[2])
		VectorCopy(impact, end);

	eglBegin(GL_LINES);
	eglVertex3f(end[0] - size, end[1], end[2]);
	eglVertex3f(end[0] + size, end[1], end[2]);
	eglVertex3f(end[0], end[1] - size, end[2]);
	eglVertex3f(end[0], end[1] + size, end[2]);
	eglVertex3f(end[0], end[1], end[2] - size);
	eglVertex3f(end[0], end[1], end[2] + size);
	eglEnd();
	*/


	//double angX = r_fovx / 2 - 90, angY = r_fovy / 2 - 90;
	//double deltaX = (90 - r_fovx / 2) / NQ_INPUT_COLS * 2, deltaY = (90 - r_fovy / 2) / NQ_INPUT_ROWS * 2;

	//Rotation delta per cell.
	double deltaX = r_fovx / NQ_INPUT_COLS, deltaY = r_fovy / NQ_INPUT_ROWS;

	//Starting angle offset from centre. 
	double angX = (-r_fovx + deltaX) / 2, angY = (-r_fovy + deltaY) / 2;

	// Define directional vectors.
	vec3_t start, end, impact, direction;
	VectorCopy(r_refdef.vieworg, start);

	// Translate the player's view angles into directional vectors.
	vec3_t final_dir, forward, right, up;
	AngleVectors(cl.viewangles, forward, right, up);

	for (int i = 0; i < NQ_INPUT_ROWS; i++)
	{
		for (int j = 0; j < NQ_INPUT_COLS; j++)
		{
			//Rotate forward to the final pitch.
			TurnVector(final_dir, forward, right, angX + deltaX * j);

			//Calculate the new up vector using the new forward.
			CrossProduct(final_dir, right, up);

			//Rotate final_dir to the final yaw.
			TurnVector(final_dir, final_dir, up, angY + deltaY * i);

			// Move the end position forward by the falloff distance amount.
			VectorScale(final_dir, trace_length, final_dir);
			VectorAdd(start, final_dir, final_dir);

			const int size = 4;

			trace_t trace;
			if (sv.max_edicts > 0)
			{
				int c = 63;

				trace = SV_Move(start, vec3_origin, vec3_origin, final_dir, false, EDICT_NUM(1));
				if (trace.fraction != 1.0) // fraction is 1.0 if nothing was hit.
				{
					//Scale the direction vector
					VectorSubtract(final_dir, start, final_dir);
					VectorScale(final_dir, trace.fraction, final_dir);
					VectorAdd(start, final_dir, final_dir);


					if (trace.ent->v.solid == SOLID_BSP) // We traced a world clip.
					{
					
					}
					else // It's an entity of some kind!
					{
						c = 79;
					}
				}


				eglDisable(GL_TEXTURE_2D);
				eglDisable(GL_ALPHA_TEST);
				eglEnable(GL_BLEND);

				byte *pal = (byte *)vid.d_8to24table;
				eglColor3f(pal[c * 4] / 255.0, pal[c * 4 + 1] / 255.0, pal[c * 4 + 2] / 255.0);
				eglBegin(GL_LINES);
				eglVertex3f(final_dir[0] - size, final_dir[1], final_dir[2]);
				eglVertex3f(final_dir[0] + size, final_dir[1], final_dir[2]);
				eglVertex3f(final_dir[0], final_dir[1] - size, final_dir[2]);
				eglVertex3f(final_dir[0], final_dir[1] + size, final_dir[2]);
				eglVertex3f(final_dir[0], final_dir[1], final_dir[2] - size);
				eglVertex3f(final_dir[0], final_dir[1], final_dir[2] + size);
				eglEnd();

				eglDisable(GL_BLEND);
				eglEnable(GL_ALPHA_TEST);
				eglEnable(GL_TEXTURE_2D);
			}
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