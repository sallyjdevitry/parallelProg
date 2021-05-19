#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#define MCW MPI_COMM_WORLD

using namespace std;

//  a) Use MPI_Allreduce to find the sum immediately.
void addALLREDUCE() { 
    int rank, size;
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size);
    int data;

    int answer=-1;

    data=rank;
    MPI_Allreduce(&data,&answer,1,MPI_INT,MPI_SUM,MCW);
    cout<<"ALLREDUCE- rank " << rank<<": data is equal to " <<answer<<endl;
}

//  b) Use only MPI_Gather and MPI_Bcast to communicate among processes.
void addGATHER() {
    int rank, size;
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size);
    int data;
    int totalAns=0;
    int recvdata[size];
    if(1){
        for(int i=0;i<size;++i)recvdata[i]=-1;
    }
    data=rank;
    MPI_Gather(&data,1,MPI_INT,recvdata,1,MPI_INT,1,MCW);
    if(rank==1){
        for(int i=0;i<size;++i) {
            totalAns += recvdata[i];
        }
        data = totalAns;
    }
    MPI_Bcast(&data,1,MPI_INT,1,MCW);
    cout<<"GATHER- rank " << rank<<": data is equal to "<<data<<endl;
}

//  c) Use only MPI_Send and MPI_Recv to send all the integers to rank 0, where the sum is calculated and the result is send back to each process.
void addSENDZERO() {
    int rank, size;
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size);
    int data;
    int totalAns=0;
    int totalMessages=0;

    if (rank != 0) {
        data = rank;
        MPI_Send(&data,1,MPI_INT,0,0,MCW);
        MPI_Recv(&data,1,MPI_INT,0,0,MCW,MPI_STATUS_IGNORE);
        cout<<"SEND&RECV- rank " << rank<<": data is equal to "<<data<<endl;
    }
    else {
        while(1) {
            MPI_Recv(&data,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,MPI_STATUS_IGNORE);
            totalAns += data;
            totalMessages += 1;
            if (totalMessages == size-1) {
                data = totalAns;
                for (int rank=0; rank<size; rank++) {
                    MPI_Send(&data,1,MPI_INT,rank,0,MCW);
                }
                break;
            }
        }
        cout<<"SEND&RECV- rank " << rank<<": data is equal to "<<totalAns<<endl;
    }
}


void cube(int *data, int m){
    int rank, size;
    int dest;
    unsigned int mask=1;
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 

    dest = rank^(mask<<m);

    MPI_Send(data,1,MPI_INT,dest,0,MCW);
    MPI_Recv(data,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,MPI_STATUS_IGNORE);

    return;
}

int ring(int *data, int plusMinus){
    int rank, size;
    int dest;
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 
    if(plusMinus==1){
        dest = (rank+1)%size;
    }else{
        dest = ((rank-1)+size)%size;
    }

    MPI_Send(data,1,MPI_INT,dest,0,MCW);
    MPI_Recv(data,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,MPI_STATUS_IGNORE);

    return *data;
}

//  d) Use only MPI_Send and MPI_Recv in a ring interconnection topology.
void addSENDRING() {
    int rank, size;
    int dest;
    int data;
    int totalAns;
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 
    data = rank;
    for (int i = 0; i < size; i++){
        totalAns += ring(&data,1);
    }
    cout<<"RING- rank " << rank<<": data is equal to "<<totalAns<<endl;
}

//  e) Use only MPI_Send and MPI_Recv in a hypercube topology.
void addSENDHYPERCUBE() {
    int rank, size;
    int dest;
    int data;
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 
    int totalAns = rank;
    int temp;
    int m = static_cast<int>(log2(size));
    for (int i=0; i<m; i++) {
        temp = totalAns;
        cube(&temp, i);
        totalAns += temp;
        MPI_Barrier(MCW);
    }
    cout<<"HYPERCUBE- rank " << rank<<": data is equal to "<<totalAns<<endl;
}

int main(int argc, char **argv){
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size);

    addALLREDUCE();
    MPI_Barrier(MCW);
    addGATHER();
    MPI_Barrier(MCW);
    addSENDRING();
    MPI_Barrier(MCW);
    addSENDHYPERCUBE();
    MPI_Barrier(MCW);
    addSENDZERO();

    MPI_Finalize();

    return 0;
}