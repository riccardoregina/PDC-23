#include <stdio.h>
#include "mpi.h"

int main()
{
    //Acquisizione dei dati

    //Distribuzione dei dati

    //Algoritmo
    int i;
    for (i = 0; i < log2(n_processi); i++) {
        if (pid % pow(2,i) == 0) {
            if (pid % pow(2,i+1) == 0) {
                tag = pid;
                //riceve
                MPI_Recv
            } else {
                tag = abs(pow(2,i) - pid) //da controllare funzione abs
                //manda
                MPI_Send
            }
        }
    }
}
