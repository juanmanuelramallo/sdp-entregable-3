#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

typedef struct counter {
  int num;
  int count;
} Counter;

double dwalltime();
void imprimeVector(int *v, int cant);
char IsSorted(int data[], int size);

double dwalltime(){
    double sec;
    struct timeval tv;

    gettimeofday(&tv,NULL);
    sec = tv.tv_sec + tv.tv_usec/1000000.0;
    return sec;
}

char IsSorted(Counter *cuentas, int size) {
	int i, isSorted = 0;
	for (i = 0; i<(size - 1); i++) {
		if (cuentas[i].count >= cuentas[i + 1].count)
			isSorted = 1;
    else
      isSorted = 0;
  }
	return isSorted;
}

void imprimir_cuentas(Counter *cuentas, int size) {
  int i;
  printf(" { ");
  for (i = 0; i < size; i++) {
    printf(" %d -> %d \t", cuentas[i].num, cuentas[i].count);
  }
  printf(" }\n");
}

int agregar_ordenado(Counter *cuentas, int num, int count, int size) {
  int i = 0;
  int j;
  int new_size = size;
  while ( i < size && cuentas[i].num != num)
    i++;

  // Si llego al final
  if (i == size) {
    cuentas[i].num = num;
    cuentas[i].count = count;
    new_size++;
  } else if (cuentas[i].num == num) { // si encontre el numero que buscaba
    cuentas[i].count += count; // actualizo la cuenta
    // ordeno el arreglo de cuentas
    Counter c;
    for (j = i; j > 0; j--) {
      if (cuentas[j].count > cuentas[j - 1].count) {
        c = cuentas[j - 1];
        cuentas[j - 1] = cuentas[j];
        cuentas[j] = c;
      }
    }
  }
  return new_size;
}

int main(int argc, char** argv) {
	if (argc < 3) {
    printf("\n Falta un argumento : \n");
    printf("\t * N cantidad de elementos del vector\n");
    printf("\t * M numero maximo de cada elemento del vector\n");
    printf("\t * I imprimir?\n");
  	return 1;
	}

	int N, M, I, i, j;
  double timetick;
	N = atoi(argv[1]);
	M = atoi(argv[2]);
	I = atoi(argv[3]);

  int *arreglo = (int*) malloc(sizeof(int) * N);
  Counter *cuentas = (Counter*) malloc(sizeof(Counter) * M);
  int cuentas_size = 0;

  for (i = 0; i < N; i++) {
    if (i < M) {
      cuentas[i].num = 0;
      cuentas[i].count = 0;
    }
    arreglo[i] = rand() % M;
  }

  timetick = dwalltime();

  for (i = 0; i < N; i++) {
    cuentas_size = agregar_ordenado(cuentas, arreglo[i], 1, cuentas_size);
  }

  printf("Tiempo de conteo y ordenacion: %f segundos\n", dwalltime() - timetick);
  if (IsSorted(cuentas, cuentas_size))
    printf("\tOrdenación correcta\n");
  else
    printf("\tNo está ordenado!\n");

  if (I)
    imprimir_cuentas(cuentas, cuentas_size);

  free(arreglo);
  free(cuentas);
	return 0;
}
