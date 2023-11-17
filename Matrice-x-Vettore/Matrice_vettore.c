#include <stdio.h>
#include "mpi.h"

void crea_griglia(MPI_Comm *griglia, MPI_Comm *griglia_r, MPI_Comm *griglia_c,int pid,int row, int col, int *coordinate){
    int dim=2;

    int ndims[2];
    ndims[0]=row;
    ndims[1]=col;

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

int **readMatrixFromFile(int **M,char *s,int *r_m,int *c_m){
    FILE *fp;
    fp=fopen(s,"r");

    fscanf("%d %d",r_m,c_m);

    int **M=malloc((*r_m)*sizeof(int *));
    int k;
    for(k=0;k<(*r_m);k++){
        M[k]=malloc((*c_m)*sizeof(int));
    }

    int i,j;
    for(i=0;i<(*r_m);i++){
        for(j=0;j<(*c_m);j++){
            fscanf("%d",&(M[i][j]));
        }
    }

    fclose(fp);
    return M;
}

int main(int argc, char *argv[]){

    int pid;
    int n_processi;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    MPI_Comm_size(MPI_COMM_WORLD,&n_processi);

    int row=atoi(argv[1]);

    int col= ((n_processi%row)==0)? n_processi/row : n_processi/(row=row-(n_processi%row));


    MPI_Comm griglia,griglia_r,griglia_c;
    int coordinate[2];

    crea_griglia(&griglia,&griglia_r,&griglia_c,pid,row,col,coordinate);

    //leggo la matrice da file
    int **M;
    int r_m,c_m;
    if(pid==0){
        M=readMatrixFromFile(M,"...",&r_m,&c_m);
    }
    //divido la matrice per blocchi

    int loc_r_m,loc_c_m;
        //capisco le dimensioni dei sottoblocchi
    
    loc_r_m= r_m/row;
    loc_c_m= c_m/col;

    rest1= r_m%row; //resto delle righe
    rest2= c_m%col; //resto delle colonne

    if(pid<rest1){
        loc_r_m++;  
    }               //assegno le colonne e righe in piu
    if(pid<rest2){
        loc_c_m++;
    }

    int **M_loc=malloc(loc_r_m*sizeof(int *))
    int i=0;
    for(i=0;i<loc_r_m;i++){
        M_loc[i]=malloc(loc_c_m*sizeof(int));
    }

    if(pid==0){
        int tag;
        int temp1=loc_r_m;
        int temp2=loc_c_m;
        int start1=0;
        int start2=0;
        for(i=1;i<n_processi;i++){
            if(i==rest1)
                temp1--;
            int k;
            for(k=0;k<temp1; k++) {
                tag = 30 + i*loc_r_m + k; //i*loc_r_m per evitare l intersezione dei tag
                MPI_Send(M[start+k]);
            }
            start+temp1;
        }
    }
    else{
        //ricevi
        for(i=0;i<loc_r_m;i++){
            tag=30 + pid*loc_r_m + i;
            MPI_Recv(&M_loc[i],tag,...); 
        }
    }




    MPI_Finalize();


}
