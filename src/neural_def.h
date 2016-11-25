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

#ifndef __NEURAL_DEF_H__
#define __NEURAL_DEF_H__

// Genetic algorithm definitions 

#define NQ_POP_SIZE 100
#define NQ_TIMEOUT 2 // Time in seconds before reload after player idles.
#define NQ_TIMEOUT_MOVING 5 // Time in seconds before reload whilst player is moving in a small area.  

#define NQ_INPUT_ROWS 7 // Total rows ray cast in the player's vision cone.
#define NQ_INPUT_COLS 11  // Total columns ray cast in the player's vision cone.

#define NQ_INPUT_COUNT NQ_INPUT_ROWS * NQ_INPUT_COLS // Input is NQ_INPUT_ROWS * NQ_INPUT_COLS rays cast outwards from the player to their immediate view.
#define NQ_OUTPUT_COUNT 10 // Output is [Forward, Backward, MoveLeft, MoveRight, LookLeft, LookRight, LookUp, LookDown, Jump, Shoot]

#define NQ_WEIGHT_MUT_POWER 1.0 // Power multiplier of weight mutations.

#define NQ_COMPAT_THRESHOLD 3.0 // The threshold at which genomes will not be considered of the same species.
#define NQ_DELTA_DISJOINT 2.0 // Max allowable interspecies difference in genome innovation.
#define NQ_DELTA_WEIGHTS 0.4 // Max allowable interspecies difference in total genome weight.

#define NQ_NOVELTY_COEFF 15 // k constant for novelty search

#define NQ_MUTATE_RAND_TRAIT_PROB 0.1 // Chance for random trait to mutate during reproduction.
#define NQ_MUTATE_LINK_TRAIT_PROB 0.1 // Chance for link trait to mutate during reproduction.
#define NQ_MUTATE_NODE_TRAIT_PROB 0.1 // Chance for node trait to mutate during reproduction.
#define NQ_MUTATE_BIAS_TRAIT_PROB 0.4 // Chance for bias to mutate during reproduction.

#define NQ_MUTATE_ADD_LINK_PROB 0.03 // Chance for link to be added to a genome during reproduction.
#define NQ_MUTATE_ADD_NODE_PROB 0.01 // Chance for node to be added to a genome during reproduction.

#define NQ_MUTATE_LINK_WEIGHTS_PROB 0.9 // Chance for link weights to mutate.

#define NQ_MUTATE_GENE_TOGGLE_PROB 0.1 // Chance for mutation to toggle a gene on or off.
#define NQ_MUTATE_GENE_ENABLE_PROB 0.05 // Change for mutation to re-enable a disable gene.

#define NQ_DROPOFF_AGE 15 // The age at which a specie will be killed off.

#define NQ_NEWLINK_TRIES 20 // Dictates how many times a new link mutation should be attempted during species reproduction.
#define NQ_NEWNODE_TRIES 20 // Dictates how many times a new node mutation should be attempted during species reproduction.

#define NQ_BABIES_STOLEN 0
#define NQ_MATE_CROSSOVER_PROB 0.75 // Probability to perform crossover reproduction.

/*
#define NQ_MATE_MULTIPOINT_PROB 0.6 // Probability to perform a multipoint reproduction.
#define NQ_MATE_MULTIPOINT_AVG_PROB 0.4 // Probability to perform a multipoint average reproduction.
#define NQ_MATE_SINGLEPOINT_PROB 0.0 // Reduces the chance of multipoint average reproduction probability check succeeding.
*/

// Neural Graph variable definitions.

#define NQ_GRAPH_POSX 0  // X Position of the graph.
#define NQ_GRAPH_POSY 80 // Y Position of the graph.

#define NQ_GRAPH_WIDTH 360  // Width of the graph.
#define NQ_GRAPH_HEIGHT 480 // Height of the graph.

#define NQ_INPUT_GRAPH_HEIGHT 220 // Height of the input layer in the graph.

#define NQ_GRAPH_BOX_PADDING 2 // Size of space between input / hidden nodes.

#define NQ_GRAPH_INBOX_WIDTH NQ_GRAPH_WIDTH / NQ_INPUT_COLS // Width of input / hidden node.
#define NQ_GRAPH_INBOX_HEIGHT NQ_INPUT_GRAPH_HEIGHT / NQ_INPUT_ROWS // Height of input / hidden node.

#define NQ_GRAPH_OUTBOX_SIZE NQ_GRAPH_WIDTH / 15 // Size of output node on the graph.

#define NQ_GRAPH_HIDDEN_Y 280 // Y-Offset of the hidden layer in the graph.
#define NQ_GRAPH_OUTPUT_Y 440 // Y-Offset of the output layer in the graph.

#endif // !__NEURAL_DEF_H__