#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <mpi.h>
#include "uthash.h"

typedef struct contador {
  int numero;
  int cuenta;
  UT_hash_handle hh;
} Contador;

typedef struct contador_envio {
  int numero;
  int cuenta;
} Contador_envio;

double dwalltime();
void imprimeVector(int *v, int cant);
void imprimeContador(Contador **cuentas);
void sumar_numero(Contador **cuentas, int numero);
void sum_hashes(Contador **cuentas_A, Contador_envio *cuentas_B, int tamano_B);
int sort_by_cuenta(Contador *a, Contador *b);
void delete_all(Contador **cuentas);
void root( int N , int M, int procesos, int I );
void worker(int id, int N, int procesos, int I);

int main(int argc, char** argv) {
	if (argc < 3) {
    printf("\n Falta un argumento : \n");
    printf("\t * N cantidad de elementos del vector\n");
    printf("\t * M numero maximo de cada elemento del vector\n");
    printf("\t * I imprimir?\n");
  	return 1;
	}

	int N, M, I, ID, cantidad_procesos;
	N = atoi(argv[1]);
	M = atoi(argv[2]);
	I = atoi(argv[3]);

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &ID);
	MPI_Comm_size(MPI_COMM_WORLD, &cantidad_procesos);

	if (ID == 0)
		root(N, M, cantidad_procesos, I);

	if (ID > 0)
		worker(ID, N, cantidad_procesos, I);

	MPI_Finalize();
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

void sumar_numero_ordenado(Contador **cuentas, int numero) {
  Contador *c;
  HASH_FIND_INT(*cuentas, &numero, c);
  if (c == NULL) {
    c = (Contador *) malloc(sizeof(Contador));
    c -> numero = numero;
    c -> cuenta = 0;
    HASH_ADD_INT(*cuentas, numero, c);
  }
  c -> cuenta = c -> cuenta + 1;
  HASH_SORT(*cuentas, sort_by_cuenta);
}

void sum_hashes(Contador **cuentas_A, Contador_envio *cuentas_B, int tamano_B) {
  // Inserta las claves/valor de cuentas_B en cuentas_A
  Contador *c;
  Contador *aux;
  int i;
  for (i = 0; i < tamano_B; i++) {
    int numero = cuentas_B[i].numero;
    HASH_FIND_INT(*cuentas_A, &numero, aux);
    if (aux == NULL) {
      aux = (Contador*) malloc(sizeof(Contador));
      aux -> numero = numero;
      HASH_ADD_INT(*cuentas_A, numero, aux);
    }
    aux -> cuenta = aux -> cuenta + cuentas_B[i].cuenta;
  }
}

void imprimeContador(Contador **cuentas) {
  Contador *c;
  printf("{ ");
  for (c = *cuentas; c != NULL; c = c -> hh.next) {
    printf("%d => %d", c -> numero, c -> cuenta);
    if (c -> hh.next)
      printf(", ");
  }
  printf(" }\n");
}

void imprimeContadorEnvio(Contador_envio *cuentas, int tamano) {
  int i;
  printf("{ ");
  for (i = 0; i < tamano; i++) {
    printf(" %d => %d |", cuentas[i].numero, cuentas[i].cuenta);
  }
  printf(" }\n");
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

int min(int a, int b) {
	if( a < b)
		return a;
	else
		return b;
}

void imprimeVector(int *v, int cant) {
	int i;
	for ( i = 0; i<cant; i++) {
		printf(" %u |", v[i]);
	}
  printf("\n");
}

void root(int N , int M, int procesos, int I) {
  MPI_Status status;
  int cantidad_elementos = N / procesos;
  int i, j, estado;
  double total_time = 0;
  double total_start_time = 0;
  double overhead_time = 0;
  double overhead_start_time = 0;
  int *arreglo = (int*) malloc(sizeof(int) * N);
  int *porcion = arreglo;
  Contador *cuentas = NULL;
  Contador *porcion_cuentas;
  Contador_envio *recibe_cuentas = (Contador_envio*) malloc(sizeof(Contador_envio) * M);
  int cantidad_recibida, cantidad_cuentas, cuentas_por_proceso;

  for (i = 0; i < N; i++)
    arreglo[i] = rand() % M;

  if (I)
    imprimeVector(arreglo, N);

  total_start_time = dwalltime();
  overhead_start_time = dwalltime();
  MPI_Scatter(arreglo, cantidad_elementos, MPI_INT, porcion, cantidad_elementos, MPI_INT, 0, MPI_COMM_WORLD);
  overhead_time = dwalltime() - overhead_start_time;

  // El root hace su parte
  for (i = 0; i < cantidad_elementos; i++)
    sumar_numero_ordenado(&cuentas, arreglo[i]);

  if (I) {
    printf("Proceso ID 0: Cuentas hechas\n");
    imprimeContador(&cuentas);
  }

  // Espera que todos los procesos envíe al root sus cuentas respectivas
  for (i = 1; i < procesos; i++) {
    overhead_start_time = dwalltime();
    MPI_Recv(&cantidad_recibida, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Recv(recibe_cuentas, cantidad_recibida, MPI_BYTE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    overhead_time = overhead_time + dwalltime() - overhead_start_time;
    printf("Root recibe cuentas: cantidad %d\n", cantidad_recibida);
    if (I)
      imprimeContadorEnvio(recibe_cuentas, cantidad_recibida / sizeof(Contador_envio));
    sum_hashes(&cuentas, recibe_cuentas, cantidad_recibida / sizeof(Contador_envio));
  }

  HASH_SORT(cuentas, sort_by_cuenta);

  total_time = dwalltime() - total_start_time;
  printf("\n----------------------------------------------------------- \n");
  printf("\tPrograma terminado en %f segundos\n", total_time);
  printf("\tTrabajo \t %f\n", total_time - overhead_time);
  printf("\tComunicación \t %f\n", overhead_time);

  if (I)
    imprimeContador(&cuentas);

  free(arreglo);
  delete_all(&cuentas);
}

void worker(int id, int N, int procesos, int I) {
  MPI_Status status;
  int cantidad_elementos = N / procesos;
  int i, j, estado;
  double total_time = 0;
  int *arreglo;
  int *porcion = (int*) malloc(sizeof(int) * cantidad_elementos);
  Contador *cuentas = NULL;
  Contador *porcion_cuentas;
  Contador *recibe_cuentas;
  Contador *c;
  int cantidad_enviar;

  total_time = dwalltime();
  MPI_Scatter(arreglo, cantidad_elementos, MPI_INT, porcion, cantidad_elementos, MPI_INT, 0, MPI_COMM_WORLD);

  for (i = 0; i < cantidad_elementos; i++)
    sumar_numero_ordenado(&cuentas, porcion[i]);

  if (I) {
    printf("Proceso ID %d: Cuentas hechas\n", id);
    imprimeContador(&cuentas);
  }

  // Arma cuentas para enviar a root
  Contador_envio *cuentas_envio = (Contador_envio*) malloc(sizeof(Contador_envio) * HASH_COUNT(cuentas));
  i = 0;
  for (c = cuentas; c != NULL; c = c -> hh.next) {
    cuentas_envio[i].numero = c -> numero;
    cuentas_envio[i].cuenta = c -> cuenta;
    i++;
  }

  // Envia al root sus cuentas
  cantidad_enviar = sizeof(Contador_envio) * HASH_COUNT(cuentas);
  printf("Cantidad a enviar %d", cantidad_enviar);
  MPI_Send(&cantidad_enviar, 1, MPI_INT, 0, 99, MPI_COMM_WORLD);
  MPI_Send(cuentas_envio, cantidad_enviar, MPI_BYTE, 0, 99, MPI_COMM_WORLD);

  free(porcion);
  delete_all(&cuentas);
}
