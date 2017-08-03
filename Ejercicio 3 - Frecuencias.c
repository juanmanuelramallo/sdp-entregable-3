#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <mpi.h>

typedef struct counter {
  int num;
  int count;
} Counter;

double dwalltime();
void imprimeVector(int *v, int cant);
char IsSorted(Counter *cuentas, int size);
void Merge(int datos[], int posIni1, int posFin1, int posIni2 , int posFin2, int *L, int largoMax);
void root(int N, int M, int procesos, int I);
void worker(int id, int N, int M, int procesos, int I);

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
		worker(ID, N, M, cantidad_procesos, I);

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

void imprimir_cuentas(Counter *cuentas, int size) {
  int i;
  printf(" { ");
  for (i = 0; i < size; i++) {
    printf(" %d -> %d \t", cuentas[i].num, cuentas[i].count);
  }
  printf(" }\n");
}

void root(int N , int M, int procesos, int I) {
  int cantidad_elementos = N / procesos; // cantidad de grupos de elementos del arreglo a repartir por procesos
  int cantidad_recibida; // variable auxiliar para verificar la cantidad de elementos enviados de un proceso a otro
  int i, j, k;
  double overhead_time = 0;
  double overhead_start_time = 0;
  double total_time = 0;
  double total_start_time = 0;
  int *arreglo = (int*) malloc(sizeof(int) * N);
  int *porcion_arreglo = arreglo;
  Counter *cuentas = (Counter *) malloc(sizeof(Counter) * M);
  Counter *recibe_cuentas = (Counter *) malloc(sizeof(Counter) * M);
  int cuentas_size = 0;
  int recibe_cuentas_size = 0;

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

  // El proceso root cuenta su parte
  for (i = 0; i < cantidad_elementos; i++) {
    cuentas_size = agregar_ordenado(cuentas, porcion_arreglo[i], 1, cuentas_size);
  }

  if (I) {
    printf("Proceso ID 0: Porcio ordenada\n");
    imprimir_cuentas(cuentas, cuentas_size);
  }

  for (i = 1; i < procesos; i *= 2) {
    overhead_start_time = dwalltime();
    MPI_Recv(&cantidad_recibida, 1, MPI_INT, i, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(recibe_cuentas, cantidad_recibida, MPI_BYTE, i, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    overhead_time = overhead_time + dwalltime() - overhead_start_time;
    recibe_cuentas_size = cantidad_recibida / sizeof(Counter); // cantidad de objetos Counter recibidos
    if (I)
      printf("Proceso ID 0: Recibió %d cuentas de proceso ID %d\n", recibe_cuentas_size, i);

    //Agregar a las cuentas del root
    for (j = 0; j < recibe_cuentas_size; j++) {
      cuentas_size = agregar_ordenado(cuentas, recibe_cuentas[j].num, recibe_cuentas[j].count, cuentas_size);
    }

    if (I) {
      printf("Root agregó cuentas\n");
      imprimir_cuentas(cuentas, cuentas_size);
    }
  }

  total_time = dwalltime() - total_start_time;
  printf("\n----------------------------------------------------------- \n");
  printf("\tPrograma terminado: \t %f seg\n", total_time);
  printf("\tTrabajo: \t\t %f seg\n", total_time - overhead_time);
  printf("\tComunicación: \t\t %f seg\n", overhead_time);

  if (I) {
    printf("\n\nFINAL\n");
    imprimir_cuentas(cuentas, cuentas_size);
  }

  if (IsSorted(cuentas, cuentas_size))
    printf("\tOrdenación correcta\n");
  else
    printf("\tNo está ordenado!\n");

  free(arreglo);
  free(cuentas);
  free(recibe_cuentas);
}

void worker(int id, int N, int M, int procesos, int I) {
	int cantidad_elementos = N/procesos;
	int cantidad_maxima, cantidad_recibida, i, j;
  bool ultimo_o_impar = (id % 2 == 1) || (id == (procesos - 1));

  if (ultimo_o_impar)
  	cantidad_maxima = cantidad_elementos;
  else
  	cantidad_maxima = N;

  int bytes_a_enviar;
  int *arreglo;
	int *porcion_arreglo = (int*) malloc(sizeof(int) * cantidad_maxima);

  Counter *cuentas = (Counter *) malloc(sizeof(Counter) * M);
  Counter *recibe_cuentas;

  if (!ultimo_o_impar)
    recibe_cuentas = (Counter *) malloc(sizeof(Counter) * M);

  int cuentas_size = 0;
  int recibe_cuentas_size = 0;

	// El proceso recibe su parte del arreglo para contar
	MPI_Scatter(arreglo, cantidad_elementos, MPI_INT, porcion_arreglo, cantidad_elementos, MPI_INT, 0, MPI_COMM_WORLD);

  // Realiza las cuentas
  for (i = 0; i < cantidad_elementos; i++) {
    cuentas_size = agregar_ordenado(cuentas, porcion_arreglo[i], 1, cuentas_size);
  }

	if( I ){
		printf("Proceso ID %d: \t Conteo primero\n" , id);
		imprimir_cuentas(cuentas, cuentas_size);
	}

	int aux_id = id;

	if (ultimo_o_impar) {
		if ( I ) {
			printf("Proceso ID %d: \t Porción ordenada y contada | Es ultimo o impar\n" , id);
    }
    bytes_a_enviar = cuentas_size * sizeof(Counter);
		MPI_Send(&bytes_a_enviar, 1, MPI_INT, id - 1, 99, MPI_COMM_WORLD);
		MPI_Send(cuentas, bytes_a_enviar, MPI_BYTE, id - 1, 99, MPI_COMM_WORLD);
		free(cuentas);
    free(porcion_arreglo);
		return;
	}

	for(i = 1; i < procesos; i *= 2 ) {
		if( I )
			printf("Proceso ID %d [%d]: \t Recibiendo de ID %d\n", id, aux_id, id + i);

		MPI_Recv(&cantidad_recibida, 1, MPI_INT, id + i, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(recibe_cuentas, cantidad_recibida, MPI_BYTE, id + i, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		if( I )
			printf("Proceso ID %d: \t Sumando cuentas de ID %d\n", id, id + i);

    // Agregando a las cuentas de este worker
    for (j = 0; j < recibe_cuentas_size; j++) {
      cuentas_size = agregar_ordenado(cuentas, recibe_cuentas[j].num, recibe_cuentas[j].count, cuentas_size);
    }

    if (I) {
      printf("Proceso ID %d agregó cuentas\n", id);
      imprimir_cuentas(cuentas, cuentas_size);
    }

		aux_id = aux_id / 2;
		if( aux_id % 2 == 1) {
			if( I )
				printf("Proceso ID %d: \t Mando datos a ID %d\n", id, aux_id);

      bytes_a_enviar = cuentas_size * sizeof(Counter);
			MPI_Send(&bytes_a_enviar, 1, MPI_INT, id - 2 * i, 99, MPI_COMM_WORLD);
			MPI_Send(cuentas, bytes_a_enviar, MPI_BYTE, id - 2 * i, 99, MPI_COMM_WORLD);
			free(cuentas);
			free(recibe_cuentas);
      free(porcion_arreglo);
			return;
		}
	}
}
