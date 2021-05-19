#include <iostream>
#include <fstream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD

using namespace std;
using namespace std::chrono;

struct Complex {
    double r;
    double i;
};

Complex operator + (Complex s, Complex t){
    Complex v;
    v.r = s.r + t.r;
    v.i = s.i + t.i;
    return v;
}

Complex operator * (Complex s, Complex t){
    Complex v;
    v.r = s.r*t.r - s.i*t.i;
    v.i = s.r*t.i + s.i*t.r;
    return v;
}

int rcolor(int iters){
    if(iters == 255) return 0;
    return 32*(iters%8);
}

int gcolor(int iters){
    if(iters == 255) return 0;
    return 32*(iters%8);
}

int bcolor(int iters){
    if(iters == 255) return 0;
    return 32*(iters%8);
}

int mbrot(Complex c, int maxIters){
    int i=0;
    Complex z;
    z=c;
    while(i<maxIters && z.r*z.r + z.i*z.i < 4){
        z = z*z + c;
        i++;
    }
    return i;
}

int main(int argc, char **argv){
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 
    int DIM = 512;
    MPI_Request request; 

    ofstream fout;

    Complex c1,c2,c;
    c1.r = -1;
    c1.i = -1;
    c2.r = 1;
    c2.i = 1;

    int finalArr[DIM][DIM];
    int dest;
    int row[DIM];
    int poison;

    if (rank == 0) {
        //process 0 opens the file
        auto start = high_resolution_clock::now();
        cout << "working...." << endl;
        fout.open("mbrot.ppm");
        fout << "P3"<<endl;
        fout << DIM << " " << DIM << endl;
        fout << "255" << endl;

        for(int j=0;j<DIM;++j){
            //send this row to a process
            dest = (j % (size-1))+1;
            MPI_Send(&j,1,MPI_INT,dest,0,MCW);
        }

        for(int j=0;j<DIM;++j) {
            MPI_Recv(row,DIM,MPI_INT,MPI_ANY_SOURCE,j,MCW,MPI_STATUS_IGNORE);
            for (int i=0; i<DIM; i++) {
                finalArr[j][i] = row[i];
            }
        }
        int iters;
        for (int i = 0; i < DIM; ++i){
            for (int j = 0; j < DIM; ++j) {
                iters = finalArr[i][j];
                fout << rcolor(iters+20)<<" ";
                fout << gcolor(iters-30)<<" ";
                fout << bcolor(iters+5)<<" ";
            }
        }
        fout << endl;
        fout.close();

        //send poison telling the processes to stop
        for (int i=0; i<size; i++) {
            poison = -1;
            MPI_Send(&poison,1,MPI_INT,i,0,MCW);
        }
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        cout << "Execution took " << duration.count() << " ms." << endl;
    }
    else {
        while(true) {
            int j;
            int row [DIM];
            MPI_Recv(&j,1,MPI_INT,0,0,MCW,MPI_STATUS_IGNORE);
            if (j == -1) {
                break;
            }
            for(int i=1;i<DIM;++i){
                // calculate one pixel of the DIM x DIM image
                c.r = (i*(c1.r-c2.r)/DIM)+c2.r;
                c.i = (j*(c1.i-c2.i)/DIM)+c2.i;
                int iters = mbrot(c,255);
                row[i] = iters;
            }
            MPI_Isend(row,DIM,MPI_INT,0,j,MCW,&request);
        }
    }

    MPI_Finalize();
    return 0;
}
