#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(){
    float **A;
    float **B;
    float **C;
    
    int n;
    int pid,nproc;

    MPI_Status status;

    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    MPI_Comm_size(MPI_COMM_WORLD,&nproc);

    
    A=readMatrix("matrix1.txt",&n);
    B=readMatrix("matrix2.txt",&n);

    int m=sqrt(nproc); //suppongo che nproc sia un quadrato perfetto la cui radice divide n 

    MPI_Comm grid;
    create_torus_communicator(m,&grid);
    MPI_Comm rows,cols;
    extrapolate_row_column_comm(grid,&rows,&cols);

    //algoritmo vero e proprio

    ...distribuzione...

    int i,j;
    int grid_rank;

    MPI_Comm_rank(grid,&grid_rank);

    int coords[2];
    MPI_Cart_coords(grid, grid_rank, 2, coords);

    int blockSize=n/m;
    
    A_local_temp=
    A_local=         //allochiamo A  B  e C secondo blockSize
    B_local=
    C_local= //inizializzare a 0
    C_local_temp= 
    
    if(coords[0]==0 && coords[1]==0){
        
        int offsetRow=0;
        int offsetCol=0;

        for(i=0;i<m;i++){
            for(j=0;j<m;j++){

                if(i==0 && j==0){
                    riempi A_local da A
                }
                else{
                    crea SottoMatrice secondo l offset
                    MPI_Send(const void *buf, int count, MPI_Datatype datatype, (i*m)+j, (i*m)+j+20, grid); //opzione A: mandiamo blockSize vettori. opzioneB:creare ogni volta datatype a seconda dell offset
                }
                offsetCol+=blockSize;
            }
            offsetRow+=blockSize;
        }
    }
    else{
        MPI_Recv(A_local, int count, MPI_Datatype datatype, 0, grid_rank+20, grid, &status); //opzione A: riceviamo blockSize vettori. opzioneB: riceviamo il sottoblocco diretto
    }

    
    for(i=0;i<m;i++){ //l algoritmo di FOX finisce dopo m passi (scorro le diagonali)

        //mandiamo il blocco di A_locale ai processori sulla stessa riga

        int source=(coords[0]*m)+ ((coords[0]+i) % m); //espressione algebrica per trovare il processore sulla diagonale di riferimento
        MPI_Bcast( A_local, int count, MPI_Datatype datatype, source, rows);

        if(grid_rank==source){
            C_local_temp=matrixMultiplication(A_local,B_local);
        }else{
            C_local_temp=matrixMultiplication(A_local_temp,B_local);
        }

        C_local=addMatrix(C_local,C_local_temp);
        

        //mandiamo il blocco di B_locale ai processori sulla stessa colonna ma sulla riga precedente

        MPI_Send(B_local, int count, MPI_Datatype datatype, mod(coords[0]-1,m)*m + coords[1], mod(coords[0]-1,m)*m + coords[1]+20, grid);
        MPI_Recv(B_local, int count, MPI_Datatype datatype, mod(coords[0]+1,m)*m + coords[1], grid_rank+20, grid, &status);

    }         

    //fine algoritmo

    MPI_Finalize();
}

float **readMatrix(char *s,int *n){
    FILE *fp;
    fp=fopen(s,"r");
    
    fscanf(fp,"%d",n);

    int i,j;

    float **M=malloc((*n)*sizeof(float *));
    for(i=0;i<(*n);i++){
        M[i]=malloc((*n)*sizeof(float));
    }

    for(i=0;i<(*n);i++){
        for(j=0;j<(*n);j++){
            fscanf(fp,"%f",&(M[i][j]));
        }
    }

    fclose(fp);

    return M;
}

void create_torus_communicator(int m,MPI_Comm *grid){

    int dim = 2;                      //The number of Dimensions
    int dims[dim] = { m , m };        //The size of each Dimension
    int wrap[dim] = { 1 , 1};         //Wraparound?
    int optimize = 1;                 //Optimize?

    MPI_Cart_create(MPI_COMM_WORLD, dim, dims, wrap, optimize, grid);

}

void extrapolate_row_column_comm(MPI_Comm grid,MPI_Comm *r,MPI_Comm *c){

  int direction[2] = { 0, 1 };
  MPI_Cart_sub(grid, direction, r);

 
  direction[0]=1; direction[1]=0;

  MPI_Cart_sub(grid, direction, c);


}

int mod(a,m){
    int ret;
    if(a<0){
        while(a<0){
            a+=m;
        }
        return a;
    }
    else{
        return a % m;
    }
}

