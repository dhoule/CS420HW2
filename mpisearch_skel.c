#include "mpisearch.h"


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
    // int *search_array = (int *) malloc(SIZE * sizeof(int));
    // for (int i = 0;i < SIZE;i++) {
    //   search_array[i] = OFFSET + rand() % RANGE;
    // }

    // Now create an array of search queries
    int *query_vector = (int *) malloc(QUERY_SIZE * sizeof(int));
    for (int i = 0;i < QUERY_SIZE;i++) {
      query_vector[i] = OFFSET + rand() % RANGE;
    }

    // First broadcast search value to all slave processes
  
    /* YOUR MASTER CODE GOES FROM HERE */
  
    // 1: Master broadcasts query list to all the slave nodes(MPI_Isend())
    MPI_Request *send_request = (MPI_Request *) malloc((np - 1)* sizeof(MPI_Request));
    for(int i = 1; i < np; i++){
      MPI_Isend(query_vector, QUERY_SIZE, MPI_INT, i, QUERY_MSG_TAG, comm, &send_request[i-1]);
      printf("\nSent query_vector to node %d.\n", i);
    }
    printf("\nFinished sending query_vector to all nodes.\n");
    fflush(stdout);

    printf("\nMaster node waiting for all ACK msgs.\n");
    fflush(stdout);
    MPI_Status *status_list = (MPI_Status *) malloc(sizeof(MPI_Status) * (np - 1));
    MPI_Waitall(np - 1,send_request, status_list);
    printf("\nMaster node received all ACK msgs.\n");
    fflush(stdout);
    // 2: Master then issues ‘n’ non-blocking receive calls and waits for an ACK message from each slave
      //  to indicate that it has received the query list using MPI_Waitall()
    // 3: On receiving all ACKS from the slave nodes, the master then sends to each slave (using non-
      // blocking MPI Isend) a chunk of size = SIZE/n the search array. Slave ‘i’ (1 <= i < n) receives 
      // chunk with start_index = (i-1) * size and end_index = start_index + size – 1. Slave ‘n’ receives 
      // size + SIZE % n elements
    // 4: Master then issues ‘n’ non-blocking receive calls and waits for response from each slave using 
      // either MPI_Waitall() or MPI_Waitany() [ MPI_Waitany() preferred over MPI_Waitall() ]
    // 5: Master then prints out the list of elements found by each slave, one line per each slave

  } else {
    /* YOUR SLAVE CODE GOES FROM HERE */
    printf("\n[Proc #%d] - Starting to work.\n", myrank);
    fflush(stdout);
    MPI_Status status;
    MPI_Request request;
    int recv_size;
    int *recv_query = (int *) malloc(QUERY_SIZE * sizeof(int));

    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
    MPI_Get_count(&status, MPI_INT, &recv_size);
    MPI_Irecv(recv_query, recv_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &request);
    MPI_Wait(&request, &status);

    // The current slave has received the `query_vector` from the master node. Need to return ACK msg.
    int ack = ACK;
    MPI_Isend(&ack, 1, MPI_INT, status.MPI_SOURCE, ACK_MSG_TAG, comm, &request);
    printf("\n[Proc #%d] - Sent ACK msg to node %d.\n", myrank, status.MPI_SOURCE);
    fflush(stdout);
    
    // 1: Each slave sends using a blocking MPI send, ONLY the list of integers from the query list that 
      // it finds in the chunk received from the master.

  }

  MPI_Finalize();
  
  return 0;
} 


