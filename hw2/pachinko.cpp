#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

using namespace std;

int main(int argc, char **argv){

    int rank, size;
    int data;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 
    srand(rank);

int numBalls = 100;
int totalRows = 5;
int ballLevel = 0;
int localCount = 0;
int middleCol = size/2;
int totalCount = 0;
int dest = 0;
int ballsDoneCount = 0;
int proc1Ct = 0;
int proc2Ct = 0;
int proc3Ct = 0;
int proc4Ct = 0;
int proc5Ct = 0;
int proc6Ct = 0;
int proc7Ct = 0;

//first process sends all balls to middle column
if (rank == 0) {
    for (int i=0; i < numBalls; i++) {
        ballLevel = 1;
        MPI_Send(&ballLevel,1,MPI_INT,middleCol,0,MCW);
    }
    while (true) {
        MPI_Recv(&ballLevel,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,MPI_STATUS_IGNORE);
        if (ballLevel == -1) {
            break;
        }
        if (ballLevel == 1) {
            proc1Ct++;
        }
        else if (ballLevel == 2) {
            proc2Ct++;
        }
        else if (ballLevel == 3) {
            proc3Ct++;
        }
        else if (ballLevel == 4) {
            proc4Ct++;
        }
        else if (ballLevel == 5) {
            proc5Ct++;
        }
        else if (ballLevel == 6) {
            proc6Ct++;
        }
        else if (ballLevel == 7) {
            proc7Ct++;
        }
        ballsDoneCount++;
        if (ballsDoneCount == numBalls) {
            //done, send poison to all processes
            for (int i=0; i<size; i++) {
                ballLevel = -1;
                MPI_Send(&ballLevel,1,MPI_INT,i,0,MCW);
            }
        cout << "slot1: " << proc1Ct << endl;
        cout << "slot2: " << proc2Ct << endl;
        cout << "slot3: " << proc3Ct << endl;
        cout << "slot4: " << proc4Ct << endl;
        cout << "slot5: " << proc5Ct << endl;
        cout << "slot6: " << proc6Ct << endl;
        cout << "slot7: " << proc7Ct << endl;
        }
    }
}

while (rank !=0) {
    MPI_Recv(&ballLevel,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,MPI_STATUS_IGNORE);
    int randNum = rand() % 3;
    int whichWay = randNum -1;
    if (ballLevel == -1) {
        break;
    }
    //uncomment the following line to see ball moves
    // cout << "rank" << rank << "received ball at level " << ballLevel << endl;
    ballLevel++;
    if (ballLevel == totalRows) {
        //send to process 0 that a ball finished (-2).
        dest = 0;
        ballLevel = rank;
    }
    else if(rank==1 || rank==size-1){
        //far left or far right peg column always send straight down, to self.
        dest = rank;
    }
    else {
        dest = rank + whichWay;
    }
    MPI_Send(&ballLevel,1,MPI_INT,dest,0,MCW);
}

    MPI_Finalize();
    return 0;
}