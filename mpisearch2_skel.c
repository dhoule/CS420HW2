#include "mpisearch.h"

/*
  Function returns a portion of the dirty array.
*/
int *slice_array(int *dirty, int start, int end) {
  int tot = (end - start) + 1;
  int bytes = sizeof(int) * tot;
  int *slice = (int *) malloc(bytes);
  memcpy(slice, dirty + start, bytes);
  return slice;
}

search_result linear_search(int* query_list, int qcount, int* data, int size) {
  search_result result;
  int *possible = (int *) malloc(size * sizeof(int));
  int found = 0;
  
  for(int i = 0; i < size; i++) {
    for(int q = 0; q < qcount; q++){
      if(data[i] == query_list[q]) {
        possible[found] = query_list[q];
        found++;
        break;
      }
    }
  }
  result.count = found;
  result.list = slice_array(possible, 0, (found - 1));
  free(possible);
  return  result;
}
      
void print_found(int *data, int size, int source) {
  // printf("\nFound query data items from slave %d, count = %d:\n",source, size);

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
  // printf("\nNo. of procs = %d, proc ID = %d initialized...", np, myrank);

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

    // variables used for keeping track of elapsed time for linear and distributed searches...
    double lStarttime, dStarttime, lEndtime, dEndtime;

    // 1st, sequential linear search
    lStarttime = MPI_Wtime();
    search_result linear = linear_search(query_vector, QUERY_SIZE, search_array, SIZE);
    // print_found(linear.list, linear.count, myrank);
    lEndtime = MPI_Wtime();
    printf("Linear:%f\n",lEndtime-lStarttime);
    fflush(stdout);

    // First broadcast search value to all slave processes
    MPI_Request *send_request = (MPI_Request *) malloc((np - 1)* sizeof(MPI_Request));
    for (int i = 1; i < np; i++) {
      MPI_Isend(query_vector, QUERY_SIZE, MPI_INT, i, QUERY_MSG_TAG, comm, &send_request[i - 1]);
    }

    MPI_Status *status_list = (MPI_Status *) malloc(sizeof(MPI_Status) * (np - 1));
    MPI_Waitall(np - 1, send_request, status_list);

    int size = SIZE / (np - 1); // size of each chunk search_array will be split into, expect maybe the last
    int i, start_index, end_index;
    // start of distributed search
    dStarttime = MPI_Wtime();
    for (i = 1; i < (np - 1); i++) {
      start_index = (i - 1) * size;
      end_index = ((start_index + size) - 1);
      int *temp = slice_array(search_array, start_index, end_index);
      MPI_Isend(temp, size, MPI_INT, i, QUERY_MSG_TAG, comm, &send_request[i - 1]);
    }
    start_index = (i - 1) * size;
    end_index = SIZE;
    int *temp = slice_array(search_array, start_index, end_index);
    MPI_Isend(temp, (end_index - start_index), MPI_INT, i, QUERY_MSG_TAG, comm, &send_request[i - 1]);
    
    int *index = (int *) malloc((np - 1) * sizeof(int));
    MPI_Waitany(np - 1, send_request, index, status_list);
    free(index);
    free(temp);

    MPI_Status status;
    int recv_size;
    for(i = 0; i < (np - 1); i++) {
      MPI_Probe(MPI_ANY_SOURCE, RESULT_MSG_TAG, comm, &status);
      MPI_Get_count(&status, MPI_INT, &recv_size);
      int *temp = (int *) calloc(recv_size, sizeof(int));
      MPI_Recv(temp, recv_size, MPI_INT, MPI_ANY_SOURCE, RESULT_MSG_TAG, comm, &status);
      // print_found( temp, recv_size, status.MPI_SOURCE);
      free(temp);
    }
    dEndtime = MPI_Wtime();
    printf("Distributed:%f\n",dEndtime-dStarttime);
    fflush(stdout);

    free(search_array);
    free(query_vector);
    free(send_request);
    free(status_list); 
  } else { 
    // printf("\n[Proc #%d] - Starting to work.\n", myrank);
    // fflush(stdout);
    MPI_Status status;
    MPI_Request request;
    int query_size;
    int search_size;
    int *recv_query = (int *) malloc(QUERY_SIZE * sizeof(int));

    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
    MPI_Get_count(&status, MPI_INT, &query_size);
    MPI_Irecv(recv_query, query_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &request);
    MPI_Wait(&request, &status);

    // The current slave has received the quesry_vector from the master node. Need to return ACK msg.
    int ack = ACK;
    MPI_Isend(&ack, 1, MPI_INT, status.MPI_SOURCE, ACK_MSG_TAG, comm, &request);
    MPI_Wait(&request, &status);
    
    // Slave `i` (1 <= i < n) receives chunk with start_index = (i - 1) * size and end_index = start + size - 1.
      // Slave `n` receives size + SIZE % n elements.
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
    MPI_Get_count(&status, MPI_INT, &search_size);

    // int *search_vector = (int *) malloc(search_size * sizeof(int));
    int *search_vector = (int *) calloc(search_size, sizeof(int));
    // The max size of possible seraches is the search array. The min, is the last 0 value minus 1.
    int *possible = (int *) malloc(search_size * sizeof(int));
    int found = 0;
    MPI_Irecv(search_vector, search_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &request);
    MPI_Wait(&request, &status);

    // Need to search `recv_query`, for the elements in `search_vector`, via linear search.
    for(int i = 0; i < search_size; i++) {
      for(int q = 0; q < query_size; q++){
        if(search_vector[i] == recv_query[q]) {
          possible[found] = recv_query[q];
          found++;
          break;
        }
      }
    }

    // 1: Each slave sends using a blocking MPI_Send, ONLY the list of integers from the query list that
      // it finds in the chunk received from the master.
    // Get only the elements that matter
    int *temp = slice_array(possible, 0, (found - 1));
    MPI_Send(temp, found, MPI_INT, status.MPI_SOURCE, RESULT_MSG_TAG, comm);

    free(temp);
    free(recv_query);
    free(search_vector);
    free(possible); 
  }

  MPI_Finalize();
  
  return 0;
} 


