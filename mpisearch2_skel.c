#include "mpisearch.h"


search_result linear_search(int* query_list, int qcount, int* data, int size) {
  search_result result;
  
  /* YOU NEED TO PUT YOUR CODE HERE */

  return  result;
}
      
void print_found(int *data, int size, int source) {
  printf("\nFound query data items from slave %d, count = %d:\n",source, size);

  for (int i = 0;i < size;i++) {
          printf("%d\t",data[i]);
  }

  printf("\n\n");

  return;
}

int main(int argc, char *argv[]) {
  int np;     // number of processes
  int myrank;   // rank of process 

  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &np);
  MPI_Comm_rank(comm, &myrank);
  printf("\nNo. of procs = %d, proc ID = %d initialized...", np, myrank);

  if (0 == myrank) {
    
    // Create an array of random integers for test purpose
    int *search_array = (int *) malloc(SIZE * sizeof(int));
    for (int i = 0;i < SIZE;i++) {
      search_array[i] = OFFSET + rand() % RANGE;
    }

    // Now create an array of search queries
    int *query_vector = (int *) malloc(QUERY_SIZE * sizeof(int));
    for (int i = 0;i < QUERY_SIZE;i++) {
      query_vector[i] = OFFSET + rand() % RANGE;
    }

    // First broadcast search value to all slave processes
  
    /* YOUR MASTER CODE GOES FROM HERE */
  

  } else {
    /* YOUR SLAVE CODE GOES FROM HERE */


  }

  MPI_Finalize();
  
  return 0;
} 


