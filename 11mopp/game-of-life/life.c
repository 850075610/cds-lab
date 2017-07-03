/*
 * The Game of Life
 *
 * a cell is born, if it has exactly three neighbours 
 * a cell dies of loneliness, if it has less than two neighbours 
 * a cell dies of overcrowding, if it has more than three neighbours 
 * a cell survives to the next generation, if it does not die of loneliness 
 * or overcrowding 
 *
 * In this version, a 2D array of ints is used.  A 1 cell is on, a 0 cell is off.
 * The game plays a number of steps (given by the input), printing to the screen each time.  'x' printed
 * means on, space means off.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//#define MAX_CPUS 
#define NUM_THREADS 24
//get_env("MAX_CPUS")

typedef unsigned char cell_t;
cell_t ** prev;
cell_t ** next;
cell_t ** tmp;

struct Para
{
	cell_t **prev;
	cell_t **next;
	int size;
	int i_start;
	int i_end;		 	
};

cell_t ** allocate_board (int size) {
	cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
	int i;
	for (i=0; i<size; i++)
		board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
	return board;
}

void free_board (cell_t ** board, int size) {
	
	int i;
	for (i=0; i<size; i++)
		free(board[i]);
	free(board);
}

/* return the number of on cells adjacent to the i,j cell */
int adjacent_to (cell_t ** board, int size, int i, int j) {
	int	k, l, count=0;
	
	int sk = (i>0) ? i-1 : i;
	int ek = (i+1 < size) ? i+1 : i;
	int sl = (j>0) ? j-1 : j;
	int el = (j+1 < size) ? j+1 : j;


	for (k=sk; k<=ek; k++)
		for (l=sl; l<=el; l++)			
			count+=board[k][l];
		count-=board[i][j];

		return count;
	}

	void play (cell_t ** board, cell_t ** newboard, int size, int i_start, int i_end) {
		int	i, j, a;
	/* for each cell, apply the rules of Life */

		for (i=i_start; i<i_end; i++)
		{
			for (j=0; j<size; j++) {
				a = adjacent_to (board, size, i, j);
				if (a == 2) newboard[i][j] = board[i][j];
				if (a == 3) newboard[i][j] = 1;
				if (a < 2) newboard[i][j] = 0;
				if (a > 3) newboard[i][j] = 0;
			}
		}
	}

	void *new_play(void * arg)
	{		
		struct Para * tmp = (struct Para *)arg;

		play(prev, next, tmp->size, tmp->i_start, tmp->i_end);
		return NULL;
	}

/* print the life board */
	void print(cell_t ** board, int size) {
		int	i, j;
	/* for each row */		
		for (j=0; j<size; j++) {
		/* print each column position... */
			for (i=0; i<size; i++) 
				printf ("%c", board[i][j] ? 'x' : ' ');
		/* followed by a carriage return */
			printf ("\n");
		}
	}

/* read a file into the life board */
	void read_file (FILE * f, cell_t ** board, int size) {
		int	i, j;
		char *s = (char *) malloc(size+10);
	//char c;
		
		for (j=0; j<size; j++) {
		/* get a string */
			fgets (s, size+10,f);
		/* copy the string to the life board */

			for (i=0; i<size; i++)
			{
		 	//c=fgetc(f);
			//putchar(c);
				board[i][j] = s[i] == 'x';
			}
		//fscanf(f,"\n");
		}
	}

	int i_start[NUM_THREADS], i_end[NUM_THREADS];

	void coordinate_allocate(int size)
	{
		//printf("Please input the size of the board :\n");
		//scanf("%d", &size);
		// int i_start[NUM_THREADS], i_end[NUM_THREADS];
		int final_number_to_process[NUM_THREADS];
		for (int i = 0; i < NUM_THREADS; ++i)
		{		
			i_start[i] = 0;
			i_end[i] = 0;
			final_number_to_process[i] = 0;
		}
		if (size < NUM_THREADS)
		{		
			for (int i = 0; i < size; i++)
			{			
				final_number_to_process[i] = 1;
			}
		}
		int base_number_to_process = size/NUM_THREADS;
		//printf("Base number to process is : %d\n", base_number_to_process);
		int extra_number_to_allocate = size%NUM_THREADS;
		//printf("Extra number to allocate is : %d\n", extra_number_to_allocate);	

		for (int i = 0; i < NUM_THREADS - 1; i++)
		{		
			i_end[i] = i_start[i] + base_number_to_process;
			i_start[i + 1] = i_end[i];
		}

		i_end[NUM_THREADS - 1] = i_start[NUM_THREADS - 1] + base_number_to_process + extra_number_to_allocate;
	/*
	for (int i = 0; i < NUM_THREADS; i++)
	{		
		printf("When the board size is %d, i_start[%d] is %d, i_end[%d] is %d.\n", size, i, i_start[i],i, i_end[i]);
	}
	*/
	//return 0;
	}
	int main (int argc, char *argv[]) {

		int size, steps;
		FILE *f;
		f = stdin;
		fscanf(f,"%d %d", &size, &steps);

		coordinate_allocate(size);

		pthread_t *tid = (pthread_t *)malloc(NUM_THREADS*sizeof(pthread_t));
		
		while (fgetc(f) != '\n') /* no-op */;
		prev = allocate_board (size);
		read_file (f, prev,size);
		fclose(f);
		next = allocate_board (size);
		//cell_t ** tmp;

	#ifdef DEBUG
		printf("Initial \n");
		print(prev,size);
		printf("----------\n");
	#endif
		
		struct Para new_play_para[NUM_THREADS];

		for (int i=0; i<steps; i++) {
			if (size < NUM_THREADS)
			{		
				play (prev,next,size, 0, size);
			        #ifdef DEBUG
				printf("%d ----------\n", i);
				print (next,size);
					#endif
				tmp = next;
				next = prev;
				prev = tmp;
			}
			else
			{
				for (int i = 0; i < NUM_THREADS; i++)
				{	
					// new_play_para.prev = prev;
					// new_play_para.next = next;   
					new_play_para[i].size = size;
					new_play_para[i].i_start =  i_start[i];
					new_play_para[i].i_end = i_end[i];
					pthread_create(&tid[i], NULL, new_play, (void *)&new_play_para[i]);					
				}		

				for (int i = 0; i < NUM_THREADS; i++)
				{		
					pthread_join(tid[i], NULL);					
				}
					#ifdef DEBUG
				printf("%d ----------\n", i);
				print (next,size);
					#endif
				tmp = next;
				next = prev;
				prev = tmp;
			}	
		}

		print (prev,size);

		free_board(prev,size);

		free_board(next,size);

		return 0;
	}
