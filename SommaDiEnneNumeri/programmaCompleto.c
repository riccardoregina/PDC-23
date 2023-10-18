#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

#define BAD_STRATEGY_VALUE 10
#define BAD_ARRAY_SIZE 20
#define ERRORE_ALLOCAZIONE_MEMORIA 30
void gestisciErrore(int error_code);

int main(int argc,char *argv[])
{    
    int pid, n_processi; //pid è il process id, n_processi è il numero di processi
    int strategy; //la strategia specificata dall'utente
    int *array_locale, start, n_locale; //il sottoarray di competenza di un processo
    //start è l'indirizzo del primo elemento di un sottovettore
    int rest; //gli elementi in esubero da ridistribuire equamente ai processi
    int tag; //tag è l'identificativo di una comunicazione tra due processi

    strategy = atoi(argv[1]);
    if (strategy != 1 && strategy != 1 && strategy != 1) {
        gestisciErrore(BAD_STRATEGY_VALUE);
    }

    MPI_Status status; //indica lo stato di una comunicazione

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processi);

    //Decido la strategia
    //Cioè vedo se n_processi è potenza di 2
    potenzaDi2 = 0;
    //Algoritmo per determinare se x è potenza di 2

    //se strategy = 2,3 AND n_processi non è potenza di 2 ALLORA procedo con la strategia 1. 
    if (potenzaDi2 == 1 && (strategy == 2 || strategy == 3)) {
        print "Strategie 1, 2 non applicabili: procedo con la strategia 1"
        strategy = 1;
    }

    //array che conterrà i numeri letti da file
    int *x;
    int n; // dimensione di x

    if (pid==0) {
        //leggi i dati dall input (n e x)
        FILE *fp;
        fp = fopen("/homes/DMA/PDC/2024/<LOGIN>/sum/sum.txt","r");

        fscanf(fp,"%d ",&n);
        if (n < n_processi || n < 0) {
            gestisciErrore(BAD_ARRAY_SIZE)
            exit(1);
        }

        x = malloc(n*sizeof(int));
        if (x == NULL) {
            gestisciErrore(ERRORE_ALLOCAZIONE_MEMORIA);
            exit(1);
        }
        
        int i;
        for (i = 0; i < n; i++){
            fscanf(fp,"%d ",&x[i]);
        }
        
        fclose(fp);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    n_locale = n / n_processi; //dividiamo la dimensione dell array per il numero di processi 
    rest = n % n_processi;     //e gestiamo il resto in modo da ottenere la dimensione
    if (pid < rest) {           // del sottoarray per ogni processo.
        n_locale++;
    }

    int tmp;
    if (pid==0) {
        array_locale = x;
        tmp = n_locale;
        start = 0;
        int i;
	    for (i = 1; i < n_processi; i++){    //spezziamo l array in piu parti e mandiamo
            start = start + tmp;              //ad ogni processo la sua parte.
            tag = 22 + i;
            if (i == rest) {	
                tmp--;
		    }
            MPI_Send(&x[start], tmp, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
    }
    else
    {
        tag = 22 + pid;
        array_locale = malloc(n_locale*sizeof(int));
	    MPI_Recv(array_locale, n_locale, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);  
        //i processi ricevono la propria parte.
    }

    //tutti i processori eseguono poi la propria somma parziale:
    int somma_parziale = 0;
    int i;
	for (i = 0; i<n_locale; i++) {
       	somma_parziale = somma_parziale + array_locale[i];
	}

    int somma_totale = 0;

    //Strategia di comunicazione
    if (strategy == 1) {
        if (pid == 0){
            for (i = 1; i < n_processi; i++){
                tag = 69 + i;
                MPI_Recv(&somma_parziale, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
                somma_totale = somma_totale + somma_parziale;
            }
        }
        else
        {
            tag = 69 + pid;
            MPI_Send(&sum, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
        }
    }
    else
    if (strategy == 2) {
        for (i = 0; i < log2(n_processi); i++) {
            if (pid % pow(2,i) == 0) {
                if (pid % pow(2,i+1) == 0) {
                    tag = pid;
                    MPI_Recv(&somma_totale, 1, MPI_Int, pid+pow(2,i), tag, MPI_COMM_WORLD, &status);
                } 
                else
                {
                    tag = abs(pow(2,i) - pid);
                    MPI_Send(&somma_parziale, 1, MPI_Int, tag, tag, MPI_COMM_WORLD, &status);
                }
            }
        }
    }
    else
    {
        for (i = 0; i < log2(n_processi); i++) {
            if (pid % pow(2,i+1) == 0) {
                MPI_Recv(&somma_totale, 1, MPI_Int, pid+pow(2,i), tag, MPI_COMM_WORLD, &status);
                MPI_Send(&somma_parziale, 1, MPI_Int, pid+pow(2,i), tag, MPI_COMM_WORLD, &status);
            } 
            else 
            {
                MPI_Recv(&somma_totale, 1, MPI_Int, pid-pow(2,i), tag, MPI_COMM_WORLD, &status);
                MPI_Send(&somma_parziale, 1, MPI_Int, pid-pow(2,i), tag, MPI_COMM_WORLD, &status);
            }
        }
    }

    //Fine delle istruzioni MPI
    MPI_Finalize();

    //Stampo la somma con il processo con pid 0
    if (pid == 0) printf("somma=%d\n",somma_totale);
}

void gestisciErrore(int error_code)
{
    if (error_code == BAD_STRATEGY_VALUE) {
        //Stampo nel file di errori l'errore trovato.
    }
    else
    if (error_code == BAD_ARRAY_SIZE) {

    }
    else
    if (error_code == ERRORE_ALLOCAZIONE_MEMORIA) {

    }
}
