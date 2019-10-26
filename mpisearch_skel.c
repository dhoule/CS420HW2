#include "mpisearch.h"

void print_found( int *data, int size, int source) {
  printf("\nFound query data items from slave %d, count = %d:\n", source, size);

  for (int i = 0; i < size; i++) {
    printf("%d\t", data[i]);
  }

  printf("\n\n");

  return;
}

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

int main(int argc, char *argv[]) {
  int np; // number of processes
  int myrank; //rank of process
  int error = -1000, errclass, resultlen; // used to retrieve any MPI status codes and messages

  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &np);
  MPI_Comm_rank(comm, &myrank);
  // Install a new error handler
  MPI_Comm_set_errhandler(comm,MPI_ERRORS_RETURN); // return info about errors
  char err_buffer[ERR_BUF_SIZE];
  printf("\nNo. of procs = %d, proc ID = %d initialized...", np, myrank);

  if (0 == myrank) {
    // Create an array of random integers for test purpose
    int *search_array = (int *) malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
      search_array[i] = OFFSET + rand() % RANGE;
    }
    fflush(stdout);

    //Now create an array of search queries
    int *query_vector = (int *) malloc(QUERY_SIZE * sizeof(int));
    for (int i = 0; i < QUERY_SIZE; i++) {
      query_vector[i] = OFFSET + rand() % RANGE;
    }

    // First broadcast search value to all slave processes

    // 1: Master broadcasts query list to all the slave nodes (MPI_Isend())
    MPI_Request *send_request = (MPI_Request *) malloc((np - 1)* sizeof(MPI_Request));
    for (int i = 1; i < np; i++) {
      MPI_Isend(query_vector, QUERY_SIZE, MPI_INT, i, QUERY_MSG_TAG, comm, &send_request[i - 1]);
      // printf("\n[%d] sent query_vector to node %d.\n", myrank,i);
      // fflush(stdout);
    }
    // printf("\nFinished sending query_vector to all nodes.\n");
    // fflush(stdout);

    // 2: Master then issues n non-blocking receive calls and waits for an ACK msg from each slave
      // to indicate that it has received the query list using MPI_Waitall()
    // printf("\nMaster node waiting for all ACK msgs.\n");
    // fflush(stdout);
    MPI_Status *status_list = (MPI_Status *) malloc(sizeof(MPI_Status) * (np - 1));
    MPI_Waitall(np - 1, send_request, status_list);
    // printf("\nMaster node received all ACK msgs.\n");
    // fflush(stdout);

    // 3: On receiving all ACKs from the slave nodes, the master then sends to each slave (using
      // non-blocking MPI_Isend) a chunk of size = SIZE/n the search array.
    // printf("\nMaster node breaking up search_array into chunks to be sent to slaves.\n");
    // fflush(stdout);

    int size = SIZE / (np - 1); // size of each chunk search_array will be split into, expect maybe the last
    int i, start_index, end_index;
    for (i = 1; i < (np - 1); i++) {
      start_index = (i - 1) * size;
      end_index = ((start_index + size) - 1);
      int *temp = slice_array(search_array, start_index, end_index);
      MPI_Isend(temp, size, MPI_INT, i, QUERY_MSG_TAG, comm, &send_request[i - 1]);
      // printf("\n[%d] sent search_array chunk of size %d to node %d.\n", myrank,size,i);
      // fflush(stdout);
    }
    start_index = (i - 1) * size;
    end_index = SIZE;
    int *temp = slice_array(search_array, start_index, end_index);
    MPI_Isend(temp, (end_index - start_index), MPI_INT, i, QUERY_MSG_TAG, comm, &send_request[i - 1]);
    // printf("\n[%d] sent search_array chunk of size %d to node %d.\n", myrank,(end_index - start_index),i);
    // fflush(stdout);

    int *index = (int *) malloc((np - 1) * sizeof(int));
    MPI_Waitany(np - 1, send_request, index, status_list);
    free(index);
    free(temp);
    // printf("\nFinished sending search_array chunks to all nodes.\n");
    // fflush(stdout);

    // Master now receives the results of the linear searches and displays them
    MPI_Status status;
    int recv_size;
    for(i = 1; i < np; i++) {
      MPI_Probe(MPI_ANY_SOURCE, RESULT_MSG_TAG, comm, &status);
      MPI_Get_count(&status, MPI_INT, &recv_size);
      int *temp = (int *) malloc(recv_size * sizeof(int));
      error = MPI_Recv(temp, recv_size, MPI_INT, status.MPI_SOURCE, RESULT_MSG_TAG, comm, &status);
      if(error != MPI_SUCCESS){
        MPI_Error_class(error,&errclass);
        MPI_Error_string(error,err_buffer,&resultlen);
        fprintf(stderr,err_buffer);
        fflush(stderr);
        free(temp);
        MPI_Finalize();
        exit(errclass);
      }
      print_found( temp, recv_size, status.MPI_SOURCE);
      free(temp);
    }

    free(search_array);
    free(query_vector);
    free(send_request);
    free(status_list);
    
  } else {
    printf("\n[Proc #%d] - Starting to work.\n", myrank);
    fflush(stdout);
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
    // printf("\n[Proc #%d] - Sent ACK msg to node %d.\n", myrank, status.MPI_SOURCE);
    // fflush(stdout);
    MPI_Wait(&request, &status);
    
    

    // Slave `i` (1 <= i < n) receives chunk with start_index = (i - 1) * size and end_index = start + size - 1.
      // Slave `n` receives size + SIZE % n elements.
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
    MPI_Get_count(&status, MPI_INT, &search_size);
    // printf("\n[Proc #%d] - Receiving search_vector from %d, of size %d.\n", myrank, status.MPI_SOURCE, search_size);
    // fflush(stdout);


    // int *search_vector = (int *) malloc(search_size * sizeof(int));
    int *search_vector = (int *) calloc(search_size, sizeof(int));
    // The max size of possible seraches is the search array. The min, is the last 0 value minus 1.
    int *possible = (int *) malloc(search_size * sizeof(int));
    int found = 0;
    MPI_Irecv(search_vector, search_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &request);
    MPI_Wait(&request, &status);
    // printf("\n[Proc #%d] - Finished receiving search_vector from %d, of size %d.\n", myrank, status.MPI_SOURCE, search_size);
    // fflush(stdout);

    // printf("\n[Proc #%d] - Starting linear search.\n", myrank);
    // fflush(stdout);
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
    // printf("\n[Proc #%d] - Finished linear search. Found %d matches.\n", myrank, found);
    // fflush(stdout);

    // 1: Each slave sends using a blocking MPI_Send, ONLY the list of integers from the query list that
      // it finds in the chunk received from the master.
    // Get only the elements that matter
    int *temp = slice_array(possible, 0, (found - 1));
    error = MPI_Send(temp, found, MPI_INT, status.MPI_SOURCE, RESULT_MSG_TAG, comm);
    if(error != MPI_SUCCESS){
      MPI_Error_class(error,&errclass);
      MPI_Error_string(error,err_buffer,&resultlen);
      fprintf(stderr,err_buffer);
      fflush(stderr);
      free(temp);
      free(recv_query);
      free(search_vector);
      free(possible); 
      MPI_Finalize();
      exit(errclass);
    }
    
    free(temp);
    free(recv_query);
    free(search_vector);
    free(possible);
  }

  MPI_Finalize();

  return 0;
}