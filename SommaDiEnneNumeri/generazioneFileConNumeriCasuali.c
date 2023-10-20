#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define INVARIANTE 3

int main()
{
    int* array;
    int dim;

    printf("Inserisci quanti elementi vuoi stampare su file:\n");
    scanf("%d", &dim);

    array = malloc(sizeof(int)*dim);
    if (array == NULL) {
        exit(1);
    }

    srand(time(NULL));

    FILE* fp = fopen("sum.txt", "w");
    if (fp == NULL) {
        exit(1);
    }
    fprintf(fp, "%d ", dim);

    int i, x;
    for (i = 0; i < dim; i++) {
        x = rand() % INVARIANTE;
        fprintf(fp, "%d ", x);
    }

    return 0;
}
