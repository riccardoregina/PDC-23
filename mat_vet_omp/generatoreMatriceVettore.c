#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generaMatrice(int m, int n, double matrice[m][n]) {
    srand(time(NULL));

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            matrice[i][j] = ((double)rand() / RAND_MAX) * 2.0; // Genera un double nel range [0, 2]
        }
    }
}

void stampaMatrice(int m, int n, double matrice[m][n]) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%.2f\t", matrice[i][j]);
        }
        printf("\n");
    }
}

void salvaMatriceSuFile(char *nomeFile, int m, int n, double matrice[m][n]) {
    FILE *file = fopen(nomeFile, "w");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%d %d\n", m, n);

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(file, "%.2f ", matrice[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

void salvaVettoreSuFile(char *nomeFile, int m, int n, double matrice[m][n]) {
    FILE *file = fopen(nomeFile, "w");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%d\n", m); // Il vettore è rappresentato come una colonna (1 colonna)

    for (int i = 0; i < m; i++) {
        fprintf(file, "%.2f\n", matrice[i][0]); // Salva ogni elemento come una riga del vettore
    }

    fclose(file);
}

int main() {
    int m, n;

    printf("Inserisci il numero di righe della matrice: ");
    scanf("%d", &m);

    printf("Inserisci il numero di colonne della matrice: ");
    scanf("%d", &n);

    double matrice[m][n];

    generaMatrice(m, n, matrice);

    printf("\nMatrice generata:\n");
    stampaMatrice(m, n, matrice);

    salvaMatriceSuFile("matrice.txt", m, n, matrice);
    salvaVettoreSuFile("vettore.txt", m, n, matrice);

    printf("\nLe matrice sono state salvate nei file 'matrice.txt' e 'vettore.txt'.\n");

    return 0;
}