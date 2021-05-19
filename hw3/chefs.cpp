#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

using namespace std;

int main(int argc, char **argv){

    int rank, size;
    int data;
    float t1, t2;
    MPI_Status mystatus;
    MPI_Request myrequest;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 
    srand(rank);

    if(rank){    // the chefs
        while(1){
            int cookDoneFlag;
            MPI_Iprobe(0,MPI_ANY_TAG,MCW,&cookDoneFlag,&mystatus);
            if (cookDoneFlag) {
                break;
            }
            int orderTime = 1;
            MPI_Isend(&orderTime,1,MPI_INT,0,0,MCW,&myrequest);
            int sleepTime = rand()%5 + 1;
            sleep(sleepTime);
        }

    }else{   //the cook
        while (true) {
            int hasOrders;
            int orderTime;
            int orderCt = 0;
            int totalOrderTime = 0;
            MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,MCW,&hasOrders,&mystatus);
            while(hasOrders) {
                MPI_Irecv(&orderTime,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,&myrequest);
                orderCt++;
                totalOrderTime+=orderTime;
                MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,MCW,&hasOrders,&mystatus);
            }
            if (orderCt >= 20) {
                cout << "Cook: 'ARGH, I'm outta here.'" << endl;
                //send poison to kill other processes
                int poison = -1;
                for (int proc=1; proc<size; proc++) {
                    MPI_Isend(&poison,1,MPI_INT,proc,0,MCW,&myrequest);
                }
                break;
            }
            else if (totalOrderTime > 0) {
                cout << "Cook: 'I'm going to cook for " << totalOrderTime << " seconds.'" << endl;
                sleep(totalOrderTime);
            }
            else {
                cout << "Cook: 'I'm going out for a smoke'" << endl;
                sleep(1);
            }
        }
    }
    MPI_Finalize();
    return 0;
}
