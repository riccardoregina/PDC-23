#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
double *matrixVector(int m,int n,double *x,double **A);
double **readMatrixFromFile(char *path,int *m,int *n);
double *readVector(char *path,int *m);
void printVector(double *y,int m);

int main(){
    int m,n;
    double **A=readMatrixFromFile("/homes/DMA/PDC/2024/TRTFNC03B/mat_vet_omp/matrice.txt",&m,&n);

    int m2;
    double *x=readVector("/homes/DMA/PDC/2024/TRTFNC03B/mat_vet_omp/vettore.txt",&m2);

    if(m2!=n){
        printf("Vettore e matrice non hanno dimensioni adeguate per una moltiplicazione...");
        exit(-1);
    }
    else{
        double *y=matrixVector(m,n,x,A);
        printVector(y,m);
    }

}

double *matrixVector(int m,int n,double *x,double **A){
    
    double *y=calloc(m,sizeof(double));
    int i,j;
    #pragma omp parallel for default(none) shared(m,n,x,A,y) private(i,j)
        for(i=0;i<m;i++){
            for(j=0;j<n;j++){
                y[i]+=A[i][j]*x[j];
            }
        }
    return y;
}

double **readMatrixFromFile(char *path,int *m,int *n){
    FILE *fp;
    fp=fopen(path,"r");
    int i,j;

    fscanf(fp,"%d %d",m,n);

    double **A=malloc((*m)*sizeof(double *));
    for(i=0;i<(*m);i++)
        A[i]=malloc((*n)*sizeof(double));

    
    for(i=0;i<(*m);i++){
        for(j=0;j<(*n);j++){
            fscanf(fp,"%lf",&(A[i][j]));
        }
    }

    fclose(fp);
    return A;

}

double *readVector(char *path,int *m){
    FILE *fp;
    fp=fopen(path,"r");
    int i;
    fscanf(fp,"%d",m);

    double *x=malloc((*m)*sizeof(double));

    for(i=0;i<(*m);i++){
        fscanf(fp,"%lf",x+i);
    }
    fclose(fp);
    return x;
}

void printVector(double *y,int m){
    int i;
    for(i=0;i<m;i++){
        printf("%f ",y[i]);
    }
    printf("\n");
}