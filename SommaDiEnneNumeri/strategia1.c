#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

int main(int argc,char *argv[]){
    
    int pid,n_processi;
    int strategy;
    int n_locale,rest;
    int *array_locale;
    int tmp,start,tag;

    MPI_Status status;

    MPI_Init(&argc,&argv);

    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    MPI_Comm_size(MPI_COMM_WORLD,&n_processi);

    int *x;
    int n;

    if(pid==0){
        //leggi i dati dall input (n e x)
        FILE *fp;
        fp=fopen("/homes/DMA/PDC/2024/TRTFNC03B/sum/sum.txt","r");

        fscanf(fp,"%d ",&n);

        x=malloc(n*sizeof(int));
        int i;
    for(i=0;i<n;i++){
        fscanf(fp,"%d ",&x[i]);
    }


        fclose(fp);

    }


    MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);

    n_locale=n/n_processi; //dividiamo la dimensione dell array per il numero di processi 
    rest=n%n_processi;     //e gestiamo il resto in modo da ottenere la dimensione
    if(pid<rest){           // del sottoarray per ogni processo.
        n_locale++;
    }




    if(pid==0){
        array_locale=x;
        tmp=n_locale;
        start=0;
        int i;
	    for(i=1;i<n_processi;i++){    //spezziamo l array in piu parti e mandiamo
          start=start+tmp;              //ad ogni processo la sua parte.
          tag=22+i;
          if(i==rest) {	
              tmp--;
		      }
          MPI_Send(&x[start],tmp,MPI_INT,i,tag,MPI_COMM_WORLD);
      }
    }
    else{
        tag=22+pid;
        array_locale=malloc(n_locale*sizeof(int));
	      MPI_Recv(array_locale,n_locale,MPI_INT,0,tag,MPI_COMM_WORLD,&status);  
        //i processi ricevono la propria parte.
    }




    //tutti i processori eseguono poi la propria somma parziale:
    int sum=0;
    int i;
	  for(i=0;i<n_locale;i++) {
       		sum=sum+array_locale[i];
	  }

    //dobbiamo ora scegliere le diverse strategie di comunicazione per il calcolo finale.

/* 
    int sum_parziale;
    if(pid==0){
	int i;
        for(i=1;i<n_processi;i++){
            tag=69+i;
            MPI_Recv(&sum_parziale,1,MPI_INT,i,tag,MPI_COMM_WORLD,&status);
            sum=sum+sum_parziale;
        }
    }
    else{
        tag=69+pid;
        MPI_Send(&sum,1,MPI_INT,0,tag,MPI_COMM_WORLD);
    }

    if(pid==0)
        printf("somma=%d\n",sum);
*/

    MPI_Finalize();
}

