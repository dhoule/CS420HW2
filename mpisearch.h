#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define SIZE 10000      // size of search array
#define QUERY_SIZE 20000    // size of query vector

#define OFFSET 1000     // offset for elements in search array
#define RANGE  10000      // MOD operation for generating random numbers in search array

#define ERR_BUF_SIZE 256    // size of buffer to hold MPI error string

/* Now define the message tags and values */
#define ACK 9999      // ACK message code
#define ACK_MSG_TAG 10      // ACK message tag

#define QUERY_MSG_TAG 100   // query message tag
#define RESULT_MSG_TAG 1000   // query results message tag

MPI_Comm comm = MPI_COMM_WORLD;

typedef struct {
  int count;
  int *list;
} search_result;


