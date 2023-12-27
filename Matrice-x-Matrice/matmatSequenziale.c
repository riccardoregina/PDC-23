#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

float **allocMatrix(int m, int n);
float **readMatrix(char *s, int *n);
float stdScalarProduct(float **A, float **B, const int i, const int j, int size);
void writeResultOnFile(char *path,int m,int n,double time,int numProcessi);

int main() {
    float **A;
    float **B;
    float **C;

    struct timeval t;
    double time;
    
    int n;
    A = readMatrix("matrix1.txt", &n);
    B = readMatrix("matrix2.txt", &n);
    C = allocMatrix(n, n);

    int i, j;

    gettimeofday(&t,NULL);
    double start=t.tv_sec+(t.tv_usec/1000000.0);//prendo i tempi iniziali

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            C[i][j] = stdScalarProduct(A, B, i, j, n);
        }
    }

    gettimeofday(&t,NULL);
    double end=t.tv_sec+(t.tv_usec/1000000.0);//prendo i tempi finali

    time = end - start;

    writeResultOnFile("results.txt", n, n, time, 1);

    return 0;
}

float **allocMatrix(int m, int n) {
  float **M;

  M = (float **)malloc(m * sizeof(float *));
  if (M == NULL) {
    //gestisciErrore(ERRORE_ALLOCAZIONE_MEMORIA);
    free(M);
    return NULL;
  }

  M[0] = (float *)malloc(m * n * sizeof(float));
  if (M[0] == NULL) {
    //gestisciErrore(ERRORE_ALLOCAZIONE_MEMORIA);
    free(M[0]);
    free(M);
    return NULL;
  }

  int i;
  for (i = 1; i < m; i++) {
    M[i] = M[i - 1] + n;
  }

  return M;
}

float **readMatrix(char *s, int *n) {
  FILE *fp;
  fp = fopen(s, "r");

  fscanf(fp, "%d", n);

  int i, j;

  float **M = allocMatrix((*n), (*n));

  for (i = 0; i < (*n); i++) {
    for (j = 0; j < (*n); j++) {
      fscanf(fp, "%f", &(M[i][j]));
    }
  }

  fclose(fp);

  return M;
}

float stdScalarProduct(float **A, float **B, const int i, const int j, int size) {
  int h, ret;
  ret = 0;
  for (h = 0; h < size; h++) {
    ret += A[i][h] * B[h][j];
  }
  return ret;
}

void writeResultOnFile(char *path,int m,int n,double time,int numProcessi){
    FILE *fp;
    fp=fopen(path,"a");

    fprintf(fp,"Dimensione in Input: %dx%d \nTempo di esecuzione: (%.10lf secondi) \nNumero di processori: %d\n\n\n",m,n,time,numProcessi);

    fclose(fp);
}