#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MOD 3

int main(int argc, char* argv[]) {
    FILE* fp = fopen("PATH/matrix.txt", "w");
    
    int dim = atoi(argv[1]);
    int dimMatrix = dim * dim;

    srand(time(NULL));

    fprintf(fp, "%d\n", dim);
    
    int i;
    for (i = 0; i < dimMatrix; i++) {
        fprintf(fp, "%f ", ((double)rand() / RAND_MAX) * MOD);
    }

    fclose(fp);

    return 0;
}