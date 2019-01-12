#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <math.h>
#include <limits.h>

/**
 * READ THIS DESCRIPTION
 *
 * node data structure: containing state, g, f
 * you can extend it with more information if needed
 */
typedef struct node{
	int state[16];
	int g;
	int f;
} node;

/**
 * Global Variables
 */

// dimensions of the 4x4 15-puzzle layout
#define ROW_LEN 4
#define COL_LEN 4
#define N_SQUARES 16

#define BLANK_TILE 0
#define HEURISTIC_END_STATE 0

// used to mark start and end, when iterating over the direction IDs
#define OP_START 0
#define OP_LEN 4

// Invalid direction IDs (i.e. not LEFT, RIGHT, DOWN, UP)
#define IMPOSSIBLE_DIR_1 -1
#define IMPOSSIBLE_DIR_2 -2

// used to track the position of the blank in a state,
// so it doesn't have to be searched every time we check if an operator is applicable
// When we apply an operator, blank_pos is updated
int blank_pos;

// Initial node of the problem
node initial_node;

// Statistics about the number of generated and expanded nodes
unsigned long generated;
unsigned long expanded;


/**
 * The id of the four available actions for moving the blank (empty slot). e.x.
 * Left: moves the blank to the left, etc.
 */

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

/*
 * Helper arrays for the applicable function
 * applicability of operators: 0 = left, 1 = right, 2 = up, 3 = down
 */
int ap_opLeft[]  = { 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 };
int ap_opRight[]  = { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 };
int ap_opUp[]  = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int ap_opDown[]  = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
int *ap_ops[] = { ap_opLeft, ap_opRight, ap_opUp, ap_opDown };


/* print state */
void print_state( int* s )
{
	int i;

	for( i = 0; i < 16; i++ )
		printf( "%2d%c", s[i], ((i+1) % 4 == 0 ? '\n' : ' ') );
}

void printf_comma (long unsigned int n) {
    if (n < 0) {
        printf ("-");
        printf_comma (-n);
        return;
    }
    if (n < 1000) {
        printf ("%lu", n);
        return;
    }
    printf_comma (n/1000);
    printf (",%03lu", n%1000);
}

/* return the sum of manhattan distances from state to goal */
int manhattan( int* state )
{
	int sum = 0;
	int i;

	// iteratively sum the manhattan distances of each non-blank tile
	for (i = 0; i < N_SQUARES; i++) {
		if (state[i] != BLANK_TILE) {
			sum += abs(state[i]/ROW_LEN - i/ROW_LEN) + abs(state[i]%COL_LEN - i%COL_LEN);
		}
	}
	return( sum );
}


/* return 1 if op is applicable in state, otherwise return 0 */
int applicable( int op )
{
       	return( ap_ops[op][blank_pos] );
}


/* apply operator */
void apply( node* n, int op )
{
	int t;

	//find tile that has to be moved given the op and blank_pos
	t = blank_pos + (op == 0 ? -1 : (op == 1 ? 1 : (op == 2 ? -4 : 4)));

	//apply op
	n->state[blank_pos] = n->state[t];
	n->state[t] = 0;

	//update blank pos
	blank_pos = t;
}

/* Returns the opposite direction, given a valid direction */
int opposite_op(int op) {
	return (op == LEFT ? RIGHT :
			(op == RIGHT ? LEFT :
			(op == UP ? DOWN :
			(op == DOWN ? UP : IMPOSSIBLE_DIR_2))));
}

/* Calculates the change in the Manhattan distance after an action */
int heuristic_increment(int* state, int blank_pos, int swapped_tile) {
	// this is the tile which swapped after the action
	int t = state[swapped_tile];
	// return the difference in manhattan distance of the tile after the action
	return abs(t/ROW_LEN - swapped_tile/ROW_LEN) - abs(t/ROW_LEN - blank_pos/ROW_LEN)
			+ abs(t%COL_LEN - swapped_tile%COL_LEN) - abs(t%COL_LEN - blank_pos%COL_LEN);
}


/* Recursive IDA */
node* ida( node* current_node, int threshold, int* newThreshold, int prev,
	unsigned long* generated_count, unsigned long* expanded_count)
{
	/**
 	 * FILL WITH YOUR CODE
 	 *
 	 * Algorithm in Figure 2 of handout
 	 */

	/*
	 * search over next possible actions iteratively, avoiding the inverse
	 * action which returns the search to the parent node
	 */

	int swapped_tile;
	int tmp_g, tmp_f, heuristic;
	int op;
	node *r = NULL;
	for (op = OP_START; op < OP_LEN; op++) {
		// check if the action is valid
		// avoids the inverse move to prune duplicate nodes
		if (op != opposite_op(prev) && applicable(op)) {

			// hold the temporary data of the parent node before the swapping action
			tmp_g = current_node->g;
			tmp_f = current_node->f;
			swapped_tile = blank_pos;
			// apply the swapping action on the node to generate a successor state
			apply(current_node, op);
			// increment the count for generated nodes
			*generated_count += 1;

			/*
			 * calculate the heuristic, and update the new node with cost
			 * of the path (g) and evaluation function (f)
			 */
			heuristic = tmp_f - tmp_g
			+ heuristic_increment(current_node->state, blank_pos, swapped_tile);

			current_node->g += 1;
			current_node->f = current_node->g + heuristic;

			// update B' (new threshold) when a search exceeds the B (threshold) depth
			if (current_node->f > threshold)
			{
				if (current_node->f < *newThreshold)
				{
					*newThreshold = current_node->f;
				}
			} else {
				// if the original state is reached by the search, return it
				if (heuristic == HEURISTIC_END_STATE)
				{
					return current_node;
				}
				// increment the count of expanded nodes as the current search increments
				*expanded_count += 1;
				r = ida(current_node, threshold, newThreshold, op, generated_count, expanded_count);

				// if the original state is found, return it
				if (r != NULL) {
					return r;
				}
			}

			/*
			 * Applies the inverse swapping action to return the swapped node
			 * back to the previous state/parent node. Then reassigns the
			 * previous g and f values.
			 */
			apply(current_node, opposite_op(op));
			current_node->g = tmp_g;
			current_node->f = tmp_f;
		}
	}
	return( NULL );
}

/* main IDA control loop */
int IDA_control_loop(  ){
	node* r = NULL;

	int threshold;

	/* initialize statistics */
	generated = 0;
	expanded = 0;

	/* compute initial threshold B */
	initial_node.f = threshold = manhattan( initial_node.state );

	printf( "Initial Estimate = %d\nThreshold = ", threshold );

	/**
	 * FILL WITH YOUR CODE
	 *
	 * Algorithm in Figure 1 of handout
	 */

	int *newThreshold = (int*)malloc(sizeof(int));

	// the node pointer used in the recursive IDA* loop and main search loop
	node* search_node = NULL;
	search_node = &initial_node;

	// if the initial state is already solved, we do not need to search
	if (threshold == HEURISTIC_END_STATE) {
		r = search_node;
	}

	// while the original state is not found, repeat the main search loop
	while (r==NULL) {
		printf("%d ", threshold);
		// set B' (new threshold) as infinity
		*newThreshold = INT_MAX;
		// call the recursive IDA* function
		r = ida(search_node, threshold, newThreshold,
			IMPOSSIBLE_DIR_1, &generated, &expanded);
		// if the original state is not found, update B (threshold) with B' (new threshold)
		if (r==NULL) {
			threshold = *newThreshold;
		}
	}

	// free all allocated memory
	free(newThreshold);

	if(r)
		return r->g;
	else
		return -1;
}


static inline float compute_current_time()
{
	struct rusage r_usage;

	getrusage( RUSAGE_SELF, &r_usage );
	float diff_time = (float) r_usage.ru_utime.tv_sec;
	diff_time += (float) r_usage.ru_stime.tv_sec;
	diff_time += (float) r_usage.ru_utime.tv_usec / (float)1000000;
	diff_time += (float) r_usage.ru_stime.tv_usec / (float)1000000;
	return diff_time;
}

int main( int argc, char **argv )
{
	int i, solution_length;

	/* check we have a initial state as parameter */
	if( argc != 2 )
	{
		fprintf( stderr, "usage: %s \"<initial-state-file>\"\n", argv[0] );
		return( -1 );
	}


	/* read initial state */
	FILE* initFile = fopen( argv[1], "r" );
	char buffer[256];

	if( fgets(buffer, sizeof(buffer), initFile) != NULL ){
		char* tile = strtok( buffer, " " );
		for( i = 0; tile != NULL; ++i )
			{
				initial_node.state[i] = atoi( tile );
				blank_pos = (initial_node.state[i] == 0 ? i : blank_pos);
				tile = strtok( NULL, " " );
			}
	}
	else{
		fprintf( stderr, "Filename empty\"\n" );
		return( -2 );

	}

	if( i != 16 )
	{
		fprintf( stderr, "invalid initial state\n" );
		return( -1 );
	}

	fclose(initFile);

	/* initialize the initial node */
	initial_node.g=0;
	initial_node.f=0;

	print_state( initial_node.state );


	/* solve */
	float t0 = compute_current_time();

	solution_length = IDA_control_loop();

	float tf = compute_current_time();

	/* report results */
	printf( "\nSolution = %d\n", solution_length);
	printf( "Generated = ");
	printf_comma(generated);
	printf("\nExpanded = ");
	printf_comma(expanded);
	printf( "\nTime (seconds) = %.2f\nExpanded/Second = ", tf-t0 );
	printf_comma((unsigned long int) expanded/(tf+0.00000001-t0));
	printf("\n\n");

	/* aggregate all executions in a file named report.dat, for marking purposes */
	FILE* report = fopen( "report.dat", "a" );

	fprintf( report, "%s", argv[1] );
	fprintf( report, "\n\tSoulution = %d, Generated = %lu, Expanded = %lu", solution_length, generated, expanded);
	fprintf( report, ", Time = %f, Expanded/Second = %f\n\n", tf-t0, (float)expanded/(tf-t0));
	fclose(report);

	return( 0 );
}
