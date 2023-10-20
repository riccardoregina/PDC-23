#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

#define BAD_STRATEGY_VALUE 10
#define BAD_ARRAY_SIZE 20
#define ERRORE_ALLOCAZIONE_MEMORIA 30
void gestisciErrore(int error_code);
int isPowerOf2(int x);

int main(int argc,char *argv[])
{    
    /*
        1) lettura di strategia
        2) inizio delle comunicazioni MPI
        3) Decido se la strategia e' applicabile
        4) leggo i dati da file
        5) allineo i processi
        6) ogni processore esegue le somme parziali
        7) applico una strategia di comunicazione
        8) chiudo le comunicazioni MPI
        9) stampo il risultato con un solo processo
    */

    int pid, n_processi; //pid è il process id, n_processi è il numero di processi
    int strategy; //la strategia specificata dall'utente
    int *array; //array che conterrà i numeri letti da file
    int dim; // dimensione di array
    int *array_locale, start, dim_locale; //il sottoarray di competenza di un processo
    //start è l'indirizzo del primo elemento di un sottoarray
    //dim_locale è la dimensione del sottoarray
    int rest; //gli elementi in esubero da ridistribuire equamente ai processi
    int tag; //tag è l'identificativo di una comunicazione tra due processi
    double tempo_tot; //tempo totale serve al processo 0 per calcolare il massimo fra tutti i tempi di esecuzione
    double t_0; // serve ad ogni processo per prendere il tempo iniziale
    double t_1; // serve ad ogni processo per prendere il tempo finale
    double time; // serve ad ogni processo per calcolare t_1 - t_0
    MPI_Status status; //indica lo stato di una comunicazione

    MPI_Init(&argc, &argv); //Inizio delle comunicazioni

    strategy = atoi(argv[1]);
    if (strategy != 1 || strategy != 2 || strategy != 3) {
        gestisciErrore(BAD_STRATEGY_VALUE);
        strategy = 1;
        //continuo lo stesso con la strategia 1
    }


    MPI_Comm_rank(MPI_COMM_WORLD, &pid); //ogni processo inserisce il proprio identificativo nella variabile pid. 
    MPI_Comm_size(MPI_COMM_WORLD, &n_processi); //Ogni processo viene a conoscenza del numero di processi.

    //Decido la strategia
    int potenzaDi2 = isPowerOf2(n_processi);
    //se strategy = 2,3 AND n_processi non è potenza di 2 ALLORA procedo con la strategia 1. 
    if (potenzaDi2 == 0 && (strategy == 2 || strategy == 3)) {
        printf("Strategie 1, 2 non applicabili: procedo con la strategia 1\n");
        strategy = 1;
    }

    if (pid==0) {
        //leggi i dati dall input (n e x)
        FILE *fp;
        fp = fopen("/homes/DMA/PDC/2024/TRTFNC03B/sum_n_numeri/sum.txt","r"); //e' richiesto il PATH intero

        fscanf(fp,"%d ",&dim);
        if (dim < n_processi || dim < 0) {
            gestisciErrore(BAD_ARRAY_SIZE);
            exit(1);
        }

        array = malloc(dim*sizeof(int));
        if (array == NULL) {
            gestisciErrore(ERRORE_ALLOCAZIONE_MEMORIA);
            exit(1);
        }
        
        int i;
        for (i = 0; i < dim; i++){
            fscanf(fp,"%d ",&array[i]);
        }
        
        fclose(fp);
    }

    MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD); //ogni processo ottiene la dimensione dell array di input (i processi si sincronizzano).

    dim_locale = dim / n_processi; //dividiamo la dimensione dell array per il numero di processi 
    rest = dim % n_processi;     //e gestiamo il resto in modo da ottenere la dimensione
    if (pid < rest) {           // del sottoarray per ogni processo.
        dim_locale++;
    }

    int tmp;
    if (pid==0) {
        array_locale = array;
        tmp = dim_locale;
        start = 0;
        int i;
	    for (i = 1; i < n_processi; i++){    //spezziamo l array in piu parti e mandiamo
            start = start + tmp;              //ad ogni processo la sua parte.
            tag = 22 + i;
            if (i == rest) {	
                tmp--;
		    }
            MPI_Send(&array[start], tmp, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
    }
    else
    {
        tag = 22 + pid;
        array_locale = malloc(dim_locale*sizeof(int));
	    MPI_Recv(array_locale, dim_locale, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);  
        //i processi ricevono la propria parte.
    }


    MPI_Barrier(MPI_COMM_WORLD); //sincronizzo i processi in modo che tutti partano dallo stesso istante. (kekko: ipotizzo che funzioni anche senza ma vabbe)
    t_0=MPI_Wtime(); // tutti i processi prendono l'istante iniziale t_0.


    //--------------------------------------
    //inizio dell'algoritmo vero e proprio.
    //--------------------------------------

    //tutti i processori eseguono la propria somma parziale:
    int somma_parziale;
    int i;
    int sum=0;
	for (i = 0; i<dim_locale; i++) {
       	sum = sum + array_locale[i];
	}
    printf("sono %d e la mia somma parziale--->%d\n",pid,sum);

    int somma_totale = 0;
    //Strategia di comunicazione
    if(pid==0)printf("eseguo startegia:%d\n",strategy);
    if (strategy == 1) {
        if (pid == 0){
            for (i = 1; i < n_processi; i++){
                tag = 69 + i;
                MPI_Recv(&somma_parziale, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
                sum = sum + somma_parziale;
            }
        }
        else
        {
            tag = 69 + pid;
            MPI_Send(&sum, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
        }
    }
    else if (strategy == 2) {
        for (i = 0; i < log2(n_processi); i++) {
            if (pid % (int)pow(2,i) == 0) {
                if (pid % (int)pow(2,i+1) == 0) {
                    tag = pid;
                    MPI_Recv(&somma_totale, 1, MPI_INT, pid+pow(2,i), tag, MPI_COMM_WORLD, &status);
                } 
                else
                {
                    tag = abs(pow(2,i) - pid);
                    MPI_Send(&somma_parziale, 1, MPI_INT, tag, tag, MPI_COMM_WORLD);
                }
            }
        }
    }
    else if(strategy == 3){
        for (i = 0; i < log2(n_processi); i++) {
            if (pid % (int)pow(2,i+1) == 0) {
                MPI_Recv(&somma_totale, 1, MPI_INT, pid+pow(2,i), tag, MPI_COMM_WORLD, &status);
                MPI_Send(&somma_parziale, 1, MPI_INT, pid+pow(2,i), tag, MPI_COMM_WORLD);
            } 
            else 
            {
                MPI_Recv(&somma_totale, 1, MPI_INT, pid-pow(2,i), tag, MPI_COMM_WORLD, &status);
                MPI_Send(&somma_parziale, 1, MPI_INT, pid-pow(2,i), tag, MPI_COMM_WORLD);
            }
        }
    }

    t_1=MPI_Wtime();//tutti i processi prendono il tempo finale.

    time= t_1 - t_0;//ogni processo calcola il proprio tempo.
    printf("sono il processo %d, tempo impiegato:%e\n",pid,time);
    
    MPI_Reduce(&time,&tempo_tot,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);  // utilizziamo la reduce sui vari tempi per ottenere il massimo (contenuto dal processo 0).

    //Stampo la somma e il tempo totale di esecuzione con il processo con pid 0.
    if(pid==0){
        printf("\nsomma=%d\n",sum);
        printf("tempo totale di esecuzione--->%e\n",tempo_tot);
    }

    //Fine delle istruzioni MPI
    MPI_Finalize();

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

int isPowerOf2(int x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}
