#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

#define ERRORE_ALLOCAZIONE_MEMORIA 1

float **readMatrix(char *s,int *n);
void create_torus_communicator(int m,MPI_Comm *grid);
void extrapolate_row_column_comm(MPI_Comm grid,MPI_Comm *r,MPI_Comm *c);
int mod(int a,int m);
float ** allocMatrix(int m, int n);
void freeMatrix(float **M);
float** initMatrix(float** A, int m, int n);
float** addMatrix(float** A, float** B, int size);
float stdScalarProduct(float** A, float** B, const int i, const int j, int size);
float** matrixMultiplication(float** A, float** B, int size);
float** createSubmatrix(float** A, int size, int offsetRow, int offsetCol);
void gestisciErrore(int code);

int main(int argc,char *argv[]){
    float **A;
    float **B;
    float **C;
    
    int n;
    int pid,nproc;

    MPI_Init(&argc, &argv);

    MPI_Status status;

    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    MPI_Comm_size(MPI_COMM_WORLD,&nproc);
    
    A=readMatrix("/homes/DMA/PDC/2024/TRTFNC03B/mat_mat/matrix1.txt",&n);
    B=readMatrix("/homes/DMA/PDC/2024/TRTFNC03B/mat_mat/matrix2.txt",&n);

    int m=sqrt(nproc); //suppongo che nproc sia un quadrato perfetto la cui radice divide n 

    MPI_Comm grid;
    int dim = 2;                      
    int dims[2];
    dims[0]=m;
    dims[1]=m;       
    int wrap[2];
    wrap[0]=1;
    wrap[1]=1;         
    int optimize = 0;                 

    if(MPI_SUCCESS==MPI_Cart_create(MPI_COMM_WORLD, dim, dims, wrap, optimize, &grid)){
        //printf("tutto ok");
        
    }
    //create_torus_communicator(m,&grid);
    //MPI_Comm rows,cols;
    //extrapolate_row_column_comm(grid,&rows,&cols);

    int i,j;
    int grid_rank;

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Comm_rank( grid, &grid_rank);
    //printf("rank in griglia--->%d\n",grid_rank);

    int coords[2];
    MPI_Cart_coords(grid, grid_rank, 2, coords);

    int blockSize=n/m;

    //Distribuzione
    
    //allochiamo A  B  e C secondo blockSize

    C=allocMatrix(n,n);
    float **A_local_temp= allocMatrix(blockSize, blockSize);
    float **A_local= allocMatrix(blockSize, blockSize);
    float **B_local= allocMatrix(blockSize, blockSize);
    float **C_local= allocMatrix(blockSize, blockSize);  //inizializzare a 0
    C_local = initMatrix(C_local,blockSize, blockSize);
    float **C_local_temp= allocMatrix(blockSize, blockSize);

        //printf("Arrivati all esecuzione\n");
        //printf("sono il processore %d --->(%d,%d)\n",grid_rank,coords[0],coords[1]);
        //printf("m--->%d\n",m);
        //printf("blockSize--->%d\n",blockSize);
        

    

    
    if(coords[0]==0 && coords[1]==0){
        
        

        for(i=0;i<m;i++){
            for(j=0;j<m;j++){

                if(i==0 && j==0){
                    //riempi A_local da A
                    A_local = createSubmatrix(A, blockSize, 0, 0);
                    B_local = createSubmatrix(B, blockSize, 0, 0);
                }
                else{
                    //crea SottoMatrice secondo l offset
                    //float** toSend = createSubmatrix(A, blockSize, offsetRow, offsetCol); non abbiamo bisogno di creare la sottomatrice da mandare
                    //...
                    MPI_Datatype blockMatrix;
                    
                    int sizes[2]    = {n,n};  
                    int subsizes[2] = {blockSize,blockSize};  
                    int starts[2]   = {i*blockSize,j*blockSize};  

                    MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_FLOAT, &blockMatrix);
                    MPI_Type_commit(&blockMatrix);
                    MPI_Send(&(A[0][0]), 1, blockMatrix, (i*m)+j, (i*m)+j+20, grid); //opzione A: mandiamo blockSize vettori. opzioneB:creare ogni volta datatype a seconda dell offset
                    MPI_Send(&(B[0][0]), 1, blockMatrix, (i*m)+j, (i*m)+j+900, grid); //opzione A: mandiamo blockSize vettori. opzioneB:creare ogni volta datatype a seconda dell offset
                }
                
            }
        }
    }
    else{
        MPI_Recv(&(A_local[0][0]), blockSize*blockSize, MPI_FLOAT, 0, grid_rank+20, grid, &status); //opzione A: riceviamo blockSize vettori. opzioneB: riceviamo il sottoblocco diretto
        MPI_Recv(&(B_local[0][0]), blockSize*blockSize, MPI_FLOAT, 0, grid_rank+900, grid, &status);
    }

    /*
    if(coords[0]!=0 || coords[1]!=0){
    printf("sottomatrice A_locale processore %d --->\n",grid_rank);
    for(i=0;i<blockSize;i++){
            for(j=0;j<blockSize;j++){
                printf("%f ",A_local[i][j]);
            }
        printf("\n");
        }
    printf("\n");
    }
    */
    
    

    // Inizio dell'algoritmo di Fox
    
    for(i=0;i<m;i++){ //l algoritmo di FOX finisce dopo m passi (scorro le diagonali)

        //mandiamo il blocco di A_locale ai processori sulla stessa riga
        
        if(coords[0]==mod(coords[1]-i,m)){ //se sono sulla diagonale
            for(j=0;j<m;j++){
                int dest=coords[0]*m + j;
                if(dest!=grid_rank){        
                    //printf("sono il processore (%d,%d), invio a %d con tag %d\n",coords[0],coords[1],dest,dest+80*(i+1));                                                        //esegui le send sulla riga
                    MPI_Send(&(A_local[0][0]), blockSize*blockSize, MPI_FLOAT, dest, dest+80*(i+1),grid);
                }
            }
        }
        else{
            int sorgente= (coords[0]*m) + (mod(coords[0]+i,blockSize));
            //printf("sono il processore (%d,%d), ricevo da %d con tag %d\n",coords[0],coords[1], sorgente,grid_rank+80*(i+1));
            MPI_Recv(&(A_local_temp[0][0]), blockSize*blockSize, MPI_FLOAT, sorgente, grid_rank+80*(i+1), grid, &status); //ricevi sulla tua riga 
        }
        
        

        if(coords[0]==mod(coords[1]-i,m)){ //se sono sulla diagonale
            C_local_temp=matrixMultiplication(A_local,B_local,blockSize);         //ho quello che mi serve in A_local
        }else{
            C_local_temp=matrixMultiplication(A_local_temp,B_local,blockSize);    //altrimenti lo ho in A_local_temp
        }

        C_local=addMatrix(C_local,C_local_temp,blockSize);
        

        //mandiamo il blocco di B_locale ai processori sulla stessa colonna ma sulla riga precedente

        MPI_Send(&(B_local[0][0]), blockSize*blockSize, MPI_FLOAT, mod(coords[0]-1,m)*m + coords[1], mod(coords[0]-1,m)*m + coords[1]+100, grid);
        MPI_Recv(&(B_local[0][0]), blockSize*blockSize, MPI_FLOAT, mod(coords[0]+1,m)*m + coords[1], grid_rank+100, grid, &status);

    }

    //fine algoritmo
    /*
    printf("sottomatrice C_locale processore %d --->\n",grid_rank);
    for(i=0;i<blockSize;i++){
            for(j=0;j<blockSize;j++){
                printf("%f ",C_local[i][j]);
            }
        printf("\n");
    }
    printf("\n");
    */




    if(coords[0]==0 && coords[1]==0){
        MPI_Datatype blockMatrix;

        for(i=0;i<m;i++){
            for(j=0;j<m;j++){
                if(i!=0 || j!=0){
                    int sizes[2]    = {n,n};  
                    int subsizes[2] = {blockSize,blockSize};  
                    int starts[2]   = {i*blockSize,j*blockSize};  

                    MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_FLOAT, &blockMatrix);
                    MPI_Type_commit(&blockMatrix);
                    MPI_Recv(&(C[0][0]), 1, blockMatrix, (i*m)+j, (i*m)+j+500, grid, &status);

                }else{
                    int k,h;
                    for(k=0;k<blockSize;k++){
                        for(h=0;h<blockSize;h++){
                            C[k][h]=C_local[k][h];
                        }
                    }
                }
            }
        }
        
    }
    else
    {
        MPI_Send(&(C_local[0][0]), blockSize*blockSize, MPI_FLOAT, 0, grid_rank+500, grid);
    }


    


    if(coords[0]==0&&coords[1]==0){
        for(i=0;i<n;i++){
            for(j=0;j<n;j++){
                printf("%f ",C[i][j]);
            }
        printf("\n");
        }
        printf("\n");
    }


    printf("Arrivati all esecuzione\n");


    MPI_Finalize();
}

float **readMatrix(char *s,int *n){
    FILE *fp;
    fp=fopen(s,"r");
    
    fscanf(fp,"%d",n);

    int i,j;

    float **M=allocMatrix((*n),(*n));
    

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
    int dims[2] = { m , m };        //The size of each Dimension
    int wrap[2] = { 1 , 1};         //Wraparound?
    int optimize = 1;                 //Optimize?

    MPI_Cart_create(MPI_COMM_WORLD, dim, dims, wrap, optimize, grid);

}

void extrapolate_row_column_comm(MPI_Comm grid,MPI_Comm *r,MPI_Comm *c){

  int direction[2] = { 0, 1 };
  MPI_Cart_sub(grid, direction, r);

 
  direction[0]=1; direction[1]=0;

  MPI_Cart_sub(grid, direction, c);


}

int mod(int a,int m){
    int ret;
    if(a<0){
        while(a<0){
            a+=m;
        }
        return a;
    }
    else{
        ret= a % m;
        return ret;
    }
}

float ** allocMatrix(int m, int n) {
    float **M;

    M = (float **) malloc(m*sizeof(float *));
    if(M==NULL) {
        gestisciErrore(ERRORE_ALLOCAZIONE_MEMORIA);
        free(M);
        return NULL;
    }

    M[0] = (float *) malloc(m*n*sizeof(float));
    if (M[0]==NULL) {
        gestisciErrore(ERRORE_ALLOCAZIONE_MEMORIA);
        free(M[0]);
        free(M);
        return NULL;
    }

    int i;
    for(i=1; i<m; i++) {
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

void gestisciErrore(int code){
    switch(code){
        case ERRORE_ALLOCAZIONE_MEMORIA:
            printf("Errore nell allocazione della matrice in memoria");
            exit(-1);
        default:
            exit(-1);
    }
}
