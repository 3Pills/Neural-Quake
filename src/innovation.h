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
#ifndef __INNOVATION_H__
#define __INNOVATION_H__

#include "environment.h"

enum innov_type_e
{
	NQ_NEWNODE = 0,
	NQ_NEWLINK = 1
};

typedef struct innovation_s
{
	enum innov_type_e innovation_type;

	int node_in_id;
	int node_out_id;

	double innovation_num1;
	double innovation_num2;

	double new_weight;
	int new_trait_num;
	int new_node_id;

	double old_innov_num;
	
	cbool recur_flag;
} innovation_t;

//Constructor for the new node case
innovation_t* Innovation_Init(int nin, int nout, double num1, double num2, int newid, double oldinnov);

//Constructor for new link case
innovation_t* Innovation_Init_Link(int nin, int nout, double num1, double w);

//Constructor for a recur link
innovation_t* Innovation_Init_Link_Recur(int nin, int nout, double num1, double w, cbool recur);
/*
//Constructor for new link case
innovation_t* Innovation_Init_Link(int nin, int nout, double num1, double w, int t);

//Constructor for a recur link
innovation_t* Innovation_Init_Link_Recur(int nin, int nout, double num1, double w, int t, cbool recur);
*/
#endif //! __INNOVATION_H__