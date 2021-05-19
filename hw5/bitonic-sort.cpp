#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#define MCW MPI_COMM_WORLD

using namespace std;

int comparison(const void * a, const void * b) {
    return ( * (int *)a - * (int *)b );
}

//sends the biggest of the list and receive the smallest of the list
void compareDown(int jth_bit, int * myArray ) {
    int rank, size;
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size);
    int i, min;

    // swap with a neighbor whose processor number differs only at the jth bit.
    int send_counter = 0;
    int * buffer_send = new int[(size + 1) * sizeof(int)];
    MPI_Send(&myArray[size - 1],1,MPI_INT,rank ^ (1 << jth_bit),0,MCW);

    // receive new min
    int recv_counter;
    int * buffer_recieve = new int[(size + 1) * sizeof(int)];
    MPI_Recv(&min,1,MPI_INT,rank ^ (1 << jth_bit),0,MCW,MPI_STATUS_IGNORE);

    // buffers all values which are greater than min
    for (i = 0; i < size; i++) {
        if (myArray[i] > min) {
            buffer_send[send_counter + 1] = myArray[i];
            send_counter++;
        }
    }

    buffer_send[0] = send_counter;

    // send and receive from paired process 
    MPI_Send(buffer_send,send_counter,MPI_INT,rank ^ (1 << jth_bit),0,MCW);
    MPI_Recv(buffer_recieve,size,MPI_INT,rank ^ (1 << jth_bit),0,MCW,MPI_STATUS_IGNORE);

    // take received buffer of values from other process which are smaller than current max
    for (i = 1; i < buffer_recieve[0] + 1; i++) {
        if (myArray[size - 1] < buffer_recieve[i]) {
            myArray[size - 1] = buffer_recieve[i];
        }
    }

    qsort(myArray, size, sizeof(int), comparison);
    free(buffer_send);
    free(buffer_recieve);

    return;
}

//sends the smallest of the list and receive the biggest of the list
void compareUp(int jth_bit, int * myArray) {
    int i, max;
    int rank, size;
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size);

    // receive max from low process
    int recv_counter;
    int * buffer_recieve = new int[(size + 1) * sizeof(int)];
    MPI_Recv(&max,1,MPI_INT,rank ^ (1 << jth_bit),0,MCW,MPI_STATUS_IGNORE);

    // send to low process of current process's array
    int send_counter = 0;
    int * buffer_send = new int[(size + 1) * sizeof(int)];
    MPI_Send(&myArray[0],1,MPI_INT,rank ^ (1 << jth_bit),0,MCW);

    // buffer a list of values which are smaller than max value
    for (i = 0; i < size; i++) {
        if (myArray[i] < max) {
            buffer_send[send_counter + 1] = myArray[i];
            send_counter++;
        }
    }

    // receive blocks greater than min from paired process
    MPI_Recv(buffer_recieve,size,MPI_INT,rank ^ (1 << jth_bit),0,MCW,MPI_STATUS_IGNORE);
    recv_counter = buffer_recieve[0];

    // send piece to paired worker
    buffer_send[0] = send_counter;
    MPI_Send(buffer_send,send_counter,MPI_INT,rank ^ (1 << jth_bit),0,MCW);

    // take received values from low process which are greater than current min
    for (i = 1; i < recv_counter + 1; i++) {
        if (buffer_recieve[i] > myArray[0]) {
            myArray[0] = buffer_recieve[i];
        }
    }

    qsort(myArray, size, sizeof(int), comparison);
    free(buffer_send);
    free(buffer_recieve);

    return;
} 

int main(int argc, char **argv){

    int rank, size;
    int i, j;
    int data;
    srand(time(NULL));
    int * myArray;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 

    //create array for storing the random numbers
    myArray = (int *) new int[(size * sizeof(int))];
    int dimensions = (int)(log2(size));

    //generate random numbers within each process
    for (i = 0; i<size; i++) {
        myArray[i] = rand() % size;
    }
    MPI_Barrier(MCW);

    qsort(myArray, size, sizeof(int), comparison);

    //decide to trade with high or low
    for (i = 0; i < dimensions; i++) {
        for (j = i; j >= 0; j--) {
            if (((rank >> (i + 1)) % 2 == 0 && (rank >> j) % 2 == 0) || ((rank >> (i + 1)) % 2 != 0 && (rank >> j) % 2 != 0)) {
                compareDown(j, myArray);
            } else {
                compareUp(j, myArray);
            }
        }
    }

    MPI_Barrier(MCW);

    //process 0 prints the sorted resulting array
    if (rank == 0) {

        printf("sorted array: ");
        for (i = 0; i < size; i++) {
            printf("%d ",myArray[i]);
        }
        printf("\n\n");
    }

    free(myArray);

    MPI_Finalize();

    return 0;
}
