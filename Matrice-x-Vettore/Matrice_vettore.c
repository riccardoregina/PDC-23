#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

void copiaInSottomatrice(int** M, int** localM, int startX, int startY, int localRow, int localCol);
void crea_griglia(MPI_Comm *griglia, MPI_Comm *griglia_r, MPI_Comm *griglia_c,int pid,int rowGriglia, int colGriglia, int *coordinate);
int **readMatrixFromFile(int **M,char *s,int *row,int *col);
void stampaSottoMatrice(int** M, int localRow, int localCol, int pid, int x, int y);


int main(int argc, char *argv[]){

    int pid;
    int n_processi;
    MPI_Status status;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    MPI_Comm_size(MPI_COMM_WORLD,&n_processi);

    int rowGriglia=atoi(argv[1]);
    n_processi=atoi(argv[2]);

    int colGriglia= ((n_processi%rowGriglia)==0)? n_processi/rowGriglia : n_processi/(rowGriglia=rowGriglia-(n_processi%rowGriglia));

    MPI_Comm griglia,griglia_r,griglia_c;
    int coordinate[2];

    crea_griglia(&griglia,&griglia_r,&griglia_c,pid,rowGriglia,colGriglia,coordinate);

    //leggo la matrice da file
    int **M;
    int row,col;
    /*
    if(coordinate[0] == 0 && coordinate[1] == 0){
        M=readMatrixFromFile(M,"...",&row,&col);
    }
    */
    
    M=readMatrixFromFile(M,"/homes/DMA/PDC/2024/RGNRCR04F/prodottoMatriceVettore/matrix.txt",&row,&col);

    //capisco le dimensioni dei sottoblocchi
    int localRow,localCol;  
    localRow= row/rowGriglia;
    localCol= col/colGriglia;

    int restRow= row%rowGriglia; //resto delle righe
    int restCol= col%colGriglia; //resto delle colonne

    //distribuisco il resto delle righe, colonne
    if(coordinate[0] < restRow) {
        localRow++;
    }
    if (coordinate[1] < restCol) {
        localCol++;
    }

    //ogni processo alloca la sua sottomatrice
    int **M_locale=malloc(localRow*sizeof(int *));
    int i=0;
    for(i=0;i<localRow;i++){
        M_locale[i]=malloc(localCol*sizeof(int));
    }

    //divido la matrice per blocchi ed assegno ciascun blocco ad un processo
    int dest, tag1, tag2;
    int startX, startY;
    #define OFFSET 10
    if(coordinate[0] == 0 && coordinate[1] == 0){
        int x, y;
        startX = 0;
        for (x = 0; x < rowGriglia; x++) {
            startY = 0;
            for (y = 0; y < colGriglia; y++) {
                if (x != 0 || y != 0) {
                    dest = x*colGriglia + y;
                    tag1 = OFFSET + dest;
                    tag2 = OFFSET - dest;
                    MPI_Send(&startX, 1, MPI_INT, dest, tag1, griglia); //modifica
                    MPI_Send(&startY, 1, MPI_INT, dest, tag2, griglia);
                }
                if (y < restCol) {
                    startY += col/colGriglia + 1;
                } else {
                    startY += col/colGriglia;
                }
            }
            if (x < restRow) { //potra essere sostituita da una variabile temp
                startX += row/rowGriglia + 1;
            } else {
                startX += row/rowGriglia;
            }
        }
    } else {
        tag1 = OFFSET + pid;
        tag2 = OFFSET - pid;
        MPI_Recv(&startX, 1, MPI_INT, 0, tag1, griglia, &status);
        MPI_Recv(&startY, 1, MPI_INT, 0, tag2, griglia, &status);
    }
    
    if (coordinate[0] == 0 && coordinate[1] == 0) {
        copiaInSottomatrice(M, M_locale, 0, 0, localRow, localCol);
    } else {
        copiaInSottomatrice(M, M_locale, startX, startY, localRow, localCol);
    }

    stampaSottoMatrice(M_locale, localRow, localCol, pid, coordinate[0], coordinate[1]);

    MPI_Finalize();

    return 0;
}

void crea_griglia(MPI_Comm *griglia, MPI_Comm *griglia_r, MPI_Comm *griglia_c,int pid,int rowGriglia, int colGriglia, int *coordinate){
    int dim=2;

    int ndims[2];
    ndims[0]=rowGriglia;
    ndims[1]=colGriglia;

    int periods[2];
    periods[0]=periods[1]=0;

    int reorder=0;


    MPI_Cart_create(MPI_COMM_WORLD,dim,ndims,periods,reorder,griglia);

    MPI_Cart_coords(*griglia,pid,2,coordinate);

    int vc[2];
    vc[0]=0;
    vc[1]=1;

    MPI_Cart_sub(*griglia,vc,griglia_r);

    vc[0]=1;
    vc[1]=0;

    MPI_Cart_sub(*griglia,vc,griglia_c);

    return;
}

int **readMatrixFromFile(int **M,char *s,int *row,int *col){
    FILE *fp;
    fp=fopen(s,"r");

    fscanf(fp, "%d %d",row,col);

    M=malloc((*row)*sizeof(int *));
    int k;
    for(k=0;k<(*row);k++){
        M[k]=malloc((*col)*sizeof(int));
    }

    int i,j;
    for(i=0;i<(*row);i++){
        for(j=0;j<(*col);j++){
            fscanf(fp, "%d",&(M[i][j]));
        }
    }

    fclose(fp);
    return M;
}

void copiaInSottomatrice(int** M, int** localM, int startX, int startY, int localRow, int localCol) {
    int i, j;
    for (i = 0; i < localRow; i++) {
        for (j = 0; j < localCol; j++) {
            localM[i][j] = M[i + startX][j + startY];
        }
    }
}

void stampaSottoMatrice(int** M, int localRow, int localCol, int pid, int x, int y) {
    printf("Sono %d con coordinate (%d,%d).\n", pid, x, y);
    int i, j;
    for (i = 0; i < localRow; i++) {
        for (j = 0; j < localCol; j++) {
            printf("%d ", M[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
