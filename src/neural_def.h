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

#define NQ_POP_SIZE 17
#define NQ_NUM_RUNS 1

#define NQ_PRINT_AFTER 4 // Defines how often population data should be printed.

#define NQ_INPUT_ROWS 9 // Total rows ray cast in the player's vision cone.
#define NQ_INPUT_COLS 15  // Total columns ray cast in the player's vision cone.

#define NQ_INPUT_COUNT 135 // Input is NQ_INPUT_ROWS * NQ_INPUT_COLS rays cast outwards from the player to their immediate view.
#define NQ_OUTPUT_COUNT 10 // Output is [Forward, Backward, MoveLeft, MoveRight, LookLeft, LookRight, LookUp, LookDown, Jump, Shoot]

#define NQ_TIMEOUT 2 // Time in seconds before savestate is reloaded after player idles.

#define NQ_RECENT_AVERAGE_FACTOR 100 // Number of previous training samples to average.

#define NQ_TRAIT_NUM_PARAMS 8 // Number of parameters in each trait.
#define NQ_TRAIT_PARAM_MUT_PROB 0.5 // Probability of trait parameter to mutate during reproduction.

#define NQ_TRAIT_MUT_POWER 1.0
#define NQ_WEIGHT_MUT_POWER 1.0

#define NQ_DISJOINT_COEFF 1.0 // Coefficient for genome compatibility testing.
#define NQ_EXCESS_COEFF 1.0 // Coefficient for genome compatibility testing.
#define NQ_MUTDIFF_COEFF 0.4 // Coefficient for genome compatibility testing.

#define NQ_NOVELTY_COEFF 15 // k constant for novelty search
#define NQ_NOVELTY_P_MIN 0.5

#define NQ_MUTATE_RAND_TRAIT_PROB 0.1 // Chance for random trait to mutate during reproduction.
#define NQ_MUTATE_LINK_TRAIT_PROB 0.1 // Chance for link trait to mutate during reproduction.
#define NQ_MUTATE_NODE_TRAIT_PROB 0.1 // Chance for node trait to mutate during reproduction.
#define NQ_MUTATE_BIAS_TRAIT_PROB 0.4 // Chance for bias to mutate during reproduction.

#define NQ_MUTATE_ADD_LINK_PROB 0.03 // Chance for link to be added to a genome during reproduction.
#define NQ_MUTATE_ADD_NODE_PROB 0.01 // Chance for node to be added to a genome during reproduction.

#define NQ_MUTATE_LINK_WEIGHTS_PROB 0.9 // Chance for link weights to mutate.

#define NQ_STEP_SIZE 0.1

#define NQ_DISABLE_MUTATION_CHANCE 0.4
#define NQ_ENABLE_MUTATION_CHANCE 0.2

#define NQ_MUTATE_TOGGLE_ENABLE_PROB 0.1
#define NQ_MUTATE_GENE_REENABLE_PROB 0.05

#define NQ_AGE_SIGNIFICANCE 1.0

#define NQ_DROPOFF_AGE 15

#define NQ_SURVIVAL_THRESHOLD 0.20
#define NQ_COMPAT_THRESHOLD 3.0

#define NQ_RECUR_ONLY_CHANCE 0.2

#define NQ_NEWLINK_TRIES 20 // Dictates how many times should a new link mutation be attempted during species reproduction.

#define NQ_BABIES_STOLEN 0

#define NQ_MATE_ONLY_PROB 0.2
#define NQ_MUTATE_ONLY_PROB 0.25 // Probability of non-mating reproduction.
#define NQ_INTERSPECIES_MATE_RATE 0.05 // Probability that reproduction will occur within a species, as opposed to outside of it.

#define NQ_MATE_MULTIPOINT_PROB 0.6 // Probability to perform a multipoint reproduction.
#define NQ_MATE_MULTIPOINT_AVG_PROB 0.4 // Probability to perform a multipoint average reproduction.
#define NQ_MATE_SINGLEPOINT_PROB 0.0 // Reduces the chance of multipoint average reproduction probability check succeeding.

#endif // !__NEURAL_DEF_H__