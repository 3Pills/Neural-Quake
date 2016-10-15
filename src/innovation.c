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
#include "innovation.h"

innovation_t Innovation_Init_Node(int nin, int nout, double num1, double num2, int newid, double oldinnov) {
	innovation_t innovation;

	innovation.innovation_type = NEWNODE;
	innovation.node_in_id = nin;
	innovation.node_out_id = nout;
	innovation.innovation_num1 = num1;
	innovation.innovation_num2 = num2;
	innovation.new_node_id = newid;
	innovation.old_innov_num = oldinnov;

	//Unused parameters set to zero
	innovation.new_weight = 0;
	innovation.new_trait_num = 0;
	innovation.recur_flag = false;

	return innovation;
}

innovation_t Innovation_Init_Link(int nin, int nout, double num1, double w, int t) {
	innovation_t innovation;

	innovation.innovation_type = NEWLINK;
	innovation.node_in_id = nin;
	innovation.node_out_id = nout;
	innovation.innovation_num1 = num1;
	innovation.new_weight = w;
	innovation.new_trait_num = t;

	//Unused parameters set to zero
	innovation.innovation_num2 = 0;
	innovation.new_node_id = 0;
	innovation.recur_flag = false;

	return innovation;
}

innovation_t Innovation_Init_Link_Recur(int nin, int nout, double num1, double w, int t, cbool recur) {
	innovation_t innovation;

	innovation.innovation_type = NEWLINK;
	innovation.node_in_id = nin;
	innovation.node_out_id = nout;
	innovation.innovation_num1 = num1;
	innovation.new_weight = w;
	innovation.new_trait_num = t;

	//Unused parameters set to zero
	innovation.innovation_num2 = 0;
	innovation.new_node_id = 0;
	innovation.recur_flag = recur;

	return innovation;
}
