#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

double *matrixVector(int m,int n,double *x,double **A,double *time,int *num_threads);
double **readMatrixFromFile(char *path,int *m,int *n);
double *readVector(char *path,int *m);
void printVector(double *y,int m);
void writeResultOnFile(char *path,int m,int n,double time,int num_threads);

int main(){
    int m,n;
    double **A=readMatrixFromFile("matrice.txt",&m,&n);
    double time;
    int num_threads;

    int m2;
    double *x=readVector("vettore.txt",&m2);

    if(m2!=n){
        printf("Vettore e matrice non hanno dimensioni adeguate per una moltiplicazione...");
        exit(-1);
    }
    else{
        double *y=matrixVector(m,n,x,A,&time,&num_threads);
        printVector(y,m);
        writeResultOnFile("results.txt",m,n,time,num_threads);
        printf("execution time--->%.10lf",time);
    }

}

double *matrixVector(int m,int n,double *x,double **A,double *time,int *num_threads){
    
    double *y=calloc(m,sizeof(double));
    int i,j;
    struct timeval t;

    (*num_threads)=omp_get_max_threads();//prendo il numero di threds del team nel for parallelo

    gettimeofday(&t,NULL);
    double start=t.tv_sec+(t.tv_usec/1000000.0);//prendo i tempi iniziali

    #pragma omp parallel for default(none) shared(m,n,x,A,y) private(i,j)

        for(i=0;i<m;i++){
            for(j=0;j<n;j++){
                y[i]+=A[i][j]*x[j];
            }
        }

    gettimeofday(&t,NULL);
    double end=t.tv_sec+(t.tv_usec/1000000.0);//prendo i tempi finali

    (*time)=end-start; //calcolo l'intervallo totale
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

void writeResultOnFile(char *path,int m,int n,double time,int num_threads){
    FILE *fp;
    fp=fopen(path,"a");

    fprintf(fp,"Dimensione in Input: %dx%d \nTempo di esecuzione: (%.10lf secondi) \nNumero di thread nel team: %d\n\n\n",m,n,time,num_threads);

    fclose(fp);
}