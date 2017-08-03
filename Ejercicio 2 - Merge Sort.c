#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <mpi.h>

double dwalltime();
void imprimeVector(int *v, int cant);
char IsSorted(int data[], int size);
void Merge(int datos[], int posIni1, int posFin1, int posIni2 , int posFin2, int *L, int largoMax);
void root(int N, int M, int procesos, int I);
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

char IsSorted(int data[], int size) {
	int i;
	for (i = 0; i<(size - 1); i++)
		if (data[i] > data[i + 1])
			return 0;
	return 1;
}

void Merge(int datos[], int posIni1, int posFin1, int posIni2 , int posFin2, int *L, int largoMax) {
	int indiceL, indiceR;
	int i;

	// Verifica que no se desborde el maximo
	if (posFin1 >= largoMax){
		posFin1 = (posFin2 + posIni1) / 2;
		posIni2 = posFin1 + 1;
	}

	// Almacena los elementos a hacer el merge en un arreglo distinto
	for( i = posIni1 ; i <= posFin2 ; i++ ){
		L[i] = datos[i];
	}

	// Efectua el merge, almacenando el resultado en "datos"
	indiceL = posIni1;
	indiceR = posIni2;

	for ( i = posIni1; i <= posFin2; i++) {
		if ( (indiceR > posFin2) || ((L[indiceL] <= L[indiceR]) && (indiceL <= posFin1)) ) {
			datos[i] = L[indiceL];
			indiceL++;
		}
		else {
			datos[i] = L[indiceR];
			indiceR++;
		}
	}
}

void root(int N , int M, int procesos, int I) {
  int cantidad_elementos = N / procesos; // cantidad de grupos de elementos del arreglo a repartir por procesos
  int cantidad_recibida; // variable auxiliar para verificar la cantidad de elementos enviados de un proceso a otro
  int i, j;
  double overhead_time = 0;
  double overhead_start_time = 0;
  double total_time = 0;
  double total_start_time = 0;
  int *arreglo = (int*) malloc(sizeof(int) * N);
  int *L = (int*) malloc(sizeof(int) * N);
  int *porcion_arreglo = arreglo;

  // Inicialización del arreglo (N números, cada uno M como máximo)
  for (i = 0; i < N; i++)
    arreglo[i] = rand() % M;
  if (I)
    imprimeVector(arreglo, N);

  total_start_time = dwalltime();
  overhead_start_time = dwalltime();
  // Distribuye una porción del arreglo a cada proceso
  MPI_Scatter(arreglo, cantidad_elementos, MPI_INT, porcion_arreglo, cantidad_elementos, MPI_INT, 0, MPI_COMM_WORLD);
  overhead_time = overhead_time + dwalltime() - overhead_start_time;
  // El proceso root ordena su parte
  for (i = 1; i < cantidad_elementos; i *= 2)
    for (j = 0; j < cantidad_elementos; j = j + 2 * i)
      Merge(arreglo, j, j + i - 1, min(j + i, cantidad_elementos - 1), min(j + 2 * i - 1, cantidad_elementos - 1), L, cantidad_elementos);

  if (I) {
    printf("Proceso ID 0: \t Primer ordenación\n");
    imprimeVector(arreglo, cantidad_elementos);
  }

  for (i = 1; i < procesos; i *= 2) {
    overhead_start_time = dwalltime();
    MPI_Recv(&cantidad_recibida, 1, MPI_INT, i, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&arreglo[cantidad_elementos], cantidad_recibida, MPI_INT, i, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    overhead_time = overhead_time + dwalltime() - overhead_start_time;

    if (I)
      printf("Proceso ID 0: \t Merging ID %d\n", i);

    Merge(arreglo, 0, cantidad_elementos - 1, cantidad_elementos, cantidad_elementos + cantidad_recibida - 1, L, cantidad_elementos);
    cantidad_elementos = cantidad_elementos + cantidad_recibida;

    if (I)
      imprimeVector(porcion_arreglo, cantidad_elementos);
  }

  total_time = dwalltime() - total_start_time;
  printf("\n----------------------------------------------------------- \n");
  printf("\tOrdenación terminada: \t %f segundos\n", total_time);
  printf("\tTrabajo: \t\t %f\n", total_time - overhead_time);
  printf("\tComunicación: \t\t %f\n", overhead_time);

  if (I)
    imprimeVector(arreglo, cantidad_elementos);

  if (IsSorted(arreglo, N))
    printf("\tOrdenación correcta!\n");
  else
    printf("Ordenación fallida\n");
  printf("-----------------------------------------------------------\n");
  free(arreglo);
  free(L);
}

void worker(int id, int N, int procesos, int I) {
	int cantidad_elementos = N/procesos;
	int cantidad_maxima, cantidad_recibida, i, j;
  bool ultimo_o_impar = (id % 2 == 1) || (id == (procesos - 1));
  // Si es un grupo de elementos de posicion impar o si es el ultimo
  if (ultimo_o_impar)
  	cantidad_maxima = cantidad_elementos;
  else
  	cantidad_maxima = N;
  int *arreglo;
	int *L = (int*) malloc(sizeof(int) * N);
	int *porcion_arreglo = (int*) malloc(sizeof(int) * cantidad_maxima);
	// El proceso recibe su parte del arreglo para ordernar
	MPI_Scatter(arreglo, cantidad_elementos, MPI_INT, porcion_arreglo, cantidad_elementos, MPI_INT, 0, MPI_COMM_WORLD);

	for (i = 1; i < cantidad_elementos; i *= 2)
		for (j = 0; j < cantidad_elementos; j += (2 * i))
			Merge(porcion_arreglo, j, j + i - 1, min(j + i, cantidad_elementos - 1), min(j + 2 * i - 1, cantidad_elementos - 1), L, cantidad_elementos);

	if( I ){
		printf("Proceso ID %d: \t Primer ordenación\n" , id);
		imprimeVector(porcion_arreglo , cantidad_elementos);
	}

	int aux_id = id;

	if (ultimo_o_impar){
		if ( I ) {
			printf("Proceso ID %d: \t Porción ordenada - ultimo o impar\n" , id);
    }
		MPI_Send(&cantidad_elementos, 1, MPI_INT, id-1, 99, MPI_COMM_WORLD);
		MPI_Send(porcion_arreglo, cantidad_elementos, MPI_INT, id-1, 99, MPI_COMM_WORLD);
		free(L);
		free(porcion_arreglo);

		return;
	}

	for(i = 1; i < procesos; i *= 2 ) {
		if( I )
			printf("Proceso ID %d [%d]: \t Recibiendo de ID %d\n", id, aux_id, id + i);

		MPI_Recv(&cantidad_recibida, 1, MPI_INT, id + i, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&porcion_arreglo[cantidad_elementos], cantidad_recibida, MPI_INT, id + i, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		if( I )
			printf("Proceso ID %d: \t Merging ID %d\n", id, id + i);

		Merge(porcion_arreglo, 0, cantidad_elementos - 1, cantidad_elementos, cantidad_elementos + cantidad_recibida - 1, L, cantidad_elementos);
		cantidad_elementos = cantidad_elementos + cantidad_recibida;

		if( I )
			imprimeVector(porcion_arreglo,cantidad_elementos);

		aux_id = aux_id / 2;
		if( aux_id % 2 == 1) {
			if( I )
				printf("Proceso ID %d: \t Mando datos a ID %d\n" , id, aux_id);

			MPI_Send(&cantidad_elementos, 1, MPI_INT, id - 2 * i, 99, MPI_COMM_WORLD);
			MPI_Send(porcion_arreglo, cantidad_elementos, MPI_INT, id - 2 * i, 99, MPI_COMM_WORLD);
			free( L );
			free( porcion_arreglo );
			return;
		}
	}

	free( L );
	free( porcion_arreglo );
}
