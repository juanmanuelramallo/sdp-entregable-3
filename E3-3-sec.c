#include <stdio.h>
#include <sys/time.h>
#include "uthash.h"

typedef struct contador {
  int numero;
  int cuenta;
  UT_hash_handle hh;
} Contador;

double dwalltime();
void imprimeVector(int *v, int cant);
void imprimeContador(Contador **cuentas);
void sumar_numero(Contador **cuentas, int numero);
int sort_by_cuenta(Contador *a, Contador *b);
void delete_all(Contador **cuentas);

int main(int argc, char const *argv[]) {
  if (argc < 3) {
    printf("\n Falta un argumento : \n");
    printf("\t * N cantidad de elementos del vector\n");
    printf("\t * M numero maximo de cada elemento del vector\n");
    printf("\t * I imprimir?\n");
  	return 1;
	}

	int N, M, I, i, j;
  int mas_frecuentes[100];
  int *arreglo;
  double total_time, start_time;
	N = atoi(argv[1]);
	M = atoi(argv[2]);
	I = atoi(argv[3]);
  arreglo = (int*) malloc(sizeof(int) * N);
  Contador *cuentas;
  Contador *c;

  for (i = 0; i < N; i++)
    arreglo[i] = rand() % M;

  start_time = dwalltime();
  for (i = 0; i < N; i++) {
    sumar_numero(&cuentas, arreglo[i]);
  }
  if (I) {
    printf("Cuentas hechas\n");
    imprimeContador(&cuentas);
  }

  HASH_SORT(cuentas, sort_by_cuenta);
  if (I) {
    printf("Cuentas ordenadas\n");
    imprimeContador(&cuentas);
  }

  c = cuentas;
  for (i = 0; i < 100; i++) {
    if (c != NULL) {
      mas_frecuentes[i] = c -> numero;
      c = c -> hh.next;
    }
  }
  if (I) {
    printf("100 mas frecuentes\n");
    imprimeVector(mas_frecuentes, 100);
  }
  total_time = dwalltime() - start_time;

  printf("Programa terminado en %f segundos\n", total_time);
  delete_all(&cuentas);
  return 0;
}

/* -------------------------------------------------------------------------- */

double dwalltime() {
  double sec;
  struct timeval tv;

  gettimeofday(&tv,NULL);
  sec = tv.tv_sec + tv.tv_usec/1000000.0;
  return sec;
}

void imprimeVector(int *v, int cant) {
	int i;
	for ( i = 0; i<cant; i++) {
		printf(" %u |", v[i]);
	}
  printf("\n");
}

void sumar_numero(Contador **cuentas, int numero) {
  Contador *c;
  HASH_FIND_INT(*cuentas, &numero, c);
  if (c == NULL) {
    c = (Contador *) malloc(sizeof(Contador));
    c -> numero = numero;
    c -> cuenta = 0;
    HASH_ADD_INT(*cuentas, numero, c);
  }
  c -> cuenta = c -> cuenta + 1;
}

void imprimeContador(Contador **cuentas) {
  Contador *c;
  printf("{ ");
  for (c = *cuentas; c != NULL; c = c -> hh.next) {
    printf("%d => %d", c -> numero, c -> cuenta);
    if (c -> hh.next)
      printf(", ");
  }
  printf(" }");
}

int sort_by_cuenta(Contador *a, Contador *b) {
  return (b -> cuenta - a -> cuenta);
}

void delete_all(Contador **cuentas) {
  Contador *cuenta, *tmp;
  HASH_ITER(hh, *cuentas, cuenta, tmp) {
    HASH_DEL(*cuentas, cuenta);
    free(cuenta);
  }
}
