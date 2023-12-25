#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

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

    int i,j;
    int grid_rank;

    MPI_Comm_rank(grid,&grid_rank);

    int coords[2];
    MPI_Cart_coords(grid, grid_rank, 2, coords);

    int blockSize=n/m;

    //Distribuzione
    
    //allochiamo A  B  e C secondo blockSize
    A_local_temp= allocMatrix(blockSize, blockSize);
    A_local= allocMatrix(blockSize, blockSize);
    B_local= allocMatrix(blockSize, blockSize);
    C_local= allocMatrix(blockSize, blockSize);  //inizializzare a 0
    C_local = initMatrix(blockSize, blockSize);
    C_local_temp= allocMatrix(blockSize, blockSize);
    
    
    if(coords[0]==0 && coords[1]==0){
        
        int offsetRow=0;
        int offsetCol=0;

        for(i=0;i<m;i++){
            for(j=0;j<m;j++){

                if(i==0 && j==0){
                    //riempi A_local da A
                    A_local = createSubmatrix(A, blockSize, offsetRow, offsetCol);
                }
                else{
                    //crea SottoMatrice secondo l offset
                    float** toSend = createSubmatrix(A, blockSize, offsetRow, offsetCol);
                    //...
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

    // Inizio dell'algoritmo di Fox
    
    for(i=0;i<m;i++){ //l algoritmo di FOX finisce dopo m passi (scorro le diagonali)

        //mandiamo il blocco di A_locale ai processori sulla stessa riga

        int source=(coords[0]*m)+ ((coords[0]+i) % m); //espressione algebrica per trovare il processore sulla diagonale di riferimento
        MPI_Bcast( A_local, int count, MPI_Datatype datatype, source, rows);

        if(grid_rank==source){
            C_local_temp=matrixMultiplication(A_local,B_local);
        }else{
            C_local_temp=matrixMultiplication(A_local_temp,B_local);
        }

        C_local=addMatrix(C_local,C_local_temp,blockSize);
        

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

float ** allocMatrix(int m, int n) {
    float **M;

    M = (float **) malloc(m*sizeof(float *));
    if(M==NULL) {
        //gestisciErrore(ERRORE_ALLOCAZIONE_MEMORIA);
        free(M);
        return NULL;
    }

    M[0] = (float *) malloc(m*n*sizeof(float));
    if (M[0]==NULL) {
        //gestisciErrore(ERRORE_ALLOCAZIONE_MEMORIA);
        free(M[0]);
        free(M);
        return NULL;
    }

    for(int i=1; i<m; i++) {
        M[i] = M[i-1] + n;
    }

    return M;
}

void freeMatrix(float **M) {
    free(M[0]);
    free(M);
}

float** initMatrix(float** A, int m, int n) {
    int i, j;
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            A[i][j] = 0;
        }
    }
    return A;
}

float** addMatrix(float** A, float** B, int size) {
    int i, j;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            A[i][j] += B[i][j];
        }
    }
    return A;
}

float stdScalarProduct(float** A, float** B, const int i, const int j, int size) { //const per enfatizzare il loro non essere variabili in questo blocco.
    int h, ret;
    ret = 0;
    for (h = 0; h < size; h++) {
        ret += A[i][h] * B[h][j];
    }
    return ret;
}

float** matrixMultiplication(float** A, float** B, int size) {
    float** C = allocMatrix(size, size);
    int i, j, k;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            C[i][j] = stdScalarProduct(A, B, i, j, size);
        }
    }
    return C;
}

float** createSubmatrix(float** A, int size, int offsetRow, int offsetCol) {
    float** R = allocMatrix(size, size);
    int i, j;
    for (i = offsetRow; i < size; i++) {
        for (j = offsetCol; j < size; j++) {
            R[i - offsetRow][j - offsetCol] = A[i][j];
        }
    }
    return R;
}