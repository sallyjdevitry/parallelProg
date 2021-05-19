#include <iostream>
#include <queue> 
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define MCW MPI_COMM_WORLD

using namespace std;
using namespace std::chrono;

int main(int argc, char **argv){
    queue<int> myWork;
    int work;
    int rank, size;
    int data;
    int tempWork;
    int randRange = 1024;
    bool white = true;
    int token = -2; //-2 for black, -1 for white
    int dest1;
    int dest2;
    int tokenDest;
    int totalGenerated = 0;
    MPI_Status mystatus;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 
    srand(time(NULL));
    MPI_Request request; 

    if (rank == 0) {
        work = rand()%randRange;
        cout << "process 0 sending out initial work of: " << work << endl;
        MPI_Send(&work,1,MPI_INT,2,0,MCW);
    }
    if (rank == 2) {
        MPI_Recv(&work,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,MPI_STATUS_IGNORE);
        cout << "rank: " << rank << " recieved " << work << endl;
        myWork.push(work);
    }

    while (true) {
        int msgRecv;
        MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,MCW,&msgRecv,&mystatus);

        if (msgRecv) {
            MPI_Recv(&work,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,MPI_STATUS_IGNORE);
            cout << "rank " << rank << "recieved " << work << endl;
            if (work == -3) {
                //recieved poison
                break;
            }
            myWork.push(work);
        }
        //recieve token if there
        MPI_Irecv(&token,1,MPI_INT,MPI_ANY_SOURCE,1,MCW,&request);
        token = token;
        if (rank == 0 && token == -1) {
            //send poison
            token = -3;
            for (int proc=1; proc<size; proc++) {
                MPI_Isend(&token,1,MPI_INT,proc,MPI_ANY_TAG,MCW,&request);
            }

            break;
        }

        if (myWork.size() != 0) {
            cout << "I am rank " << rank << " with task queue size: " << myWork.size() << endl;
            if (myWork.size() > 10) {
                work = myWork.front();
                myWork.pop();
                dest1 = rand()%size;
                if (dest1 < rank) {
                    white = false;
                }
                MPI_Isend(&work,1,MPI_INT,dest1,0,MCW,&request);
                work = myWork.front();
                myWork.pop();
                dest2 = rand()%size;
                if (dest2 < rank) {
                    white = false;
                }
                MPI_Isend(&work,1,MPI_INT,dest2,0,MCW,&request);
            }
            //perform some work
            cout << "process: " << rank << " doing work: " << myWork.front() << endl; 
            for (int i=0; i<myWork.front(); i++) {
                tempWork += rand()%size;
            }
            myWork.pop();
        }

        if (rank % 2 == 0 && totalGenerated < rand() % randRange+100) {
            myWork.push(rand() % randRange);
            myWork.push(rand() % randRange);
            totalGenerated+=1;
        }
        if (rank == 0) {
            token = -1;
        }
        if (myWork.size() == 0 && token < 0) {
            if (token == -3) {
                break;
            }
            if (rank == size-1) {
                tokenDest = 0;
            }
            else {
                tokenDest = rank+1;
            }
            MPI_Isend(&token,1,MPI_INT,tokenDest,1,MCW,&request);
        }
    }

    MPI_Finalize();

    return 0;
}
