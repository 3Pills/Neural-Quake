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

void CL_NeuralThink(double frametime) 
{

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

void SCR_DrawNeuralData()
{
	if (!neuraldisplay.value) return;

	Draw_SetCanvas(CANVAS_BOTTOMLEFT);

	int		y = 25 - 7.5; //9=number of lines to print
	int		x = 4; //margin
	char	str[60];

	Draw_Fill(0, y * 7.5, 25 * 9, 9 * 8, 0, 0.5); //dark rectangle

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


	// Set to draw from the top left.
	Draw_SetCanvas(CANVAS_DEFAULT);

	//Draw_Line(0, 0, 400, 400, 4, (int)r_neural_color_1.value, 1);
	//Draw_Line(0, 0, 400, 200, 4, (int)r_neural_color_1.value, 1);
	//Draw_Line(0, 0, 200, 400, 4, (int)r_neural_color_1.value, 1);

	//for (int i = 0; i < 200; i++)
	//{
	//	Draw_Line((i * 4), (i * 4), (i * 4) + 1, (i * 4) + 1, 4, (int)r_neural_color_1.value, 1);
	//}

	//Draw_Fill(400, 0, 80, 80, 45, 254);

	//Draw_Line(120, 0, 200, 0, 4, (int)r_neural_color_1.value, 1);
	//Draw_Square(200, 0, 80, 80, 2, (int)r_neural_color_2.value, 1);
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