#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

double dwalltime();
double promedio_matriz(uint32_t *A, uint32_t N);
void imprime_matriz(uint32_t *S, uint32_t F, uint32_t C, int fil);
void imprime_vector(uint32_t *S, uint32_t size_vector);
void multiplica_matriz(uint32_t *A, uint32_t *B, uint32_t *R, uint32_t F, uint32_t C, double coeficiente);
void suma_matrices(uint32_t *A, uint32_t *B, uint32_t *R, uint32_t F, uint32_t C);
void root(uint32_t N, uint32_t cantidad_procesos);
void worker(uint32_t ID, uint32_t N, uint32_t cantidad_procesos);

int main(int argc, char** argv) {
  if (argc < 2){
    printf("\n Falta un argumento : \n");
    printf("\t * N dimensión matriz\n");
    return 1;
  }

  uint32_t N = atoi(argv[1]);
  int ID, cantidad_procesos;

  MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &ID);
	MPI_Comm_size(MPI_COMM_WORLD, &cantidad_procesos);

	if (ID == 0)
		root(N, cantidad_procesos);
  else if (ID > 0)
		worker(ID, N, cantidad_procesos);

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

void imprime_matriz(uint32_t *S, uint32_t F, uint32_t C, int fil){
  int i,j;
  printf("\nImprimiendo matriz\n");
  for (i=0;i<C;i++){
    for (j=0;j<F;j++){
      if (fil == 1)
        printf("%u\t", S[i*F+j]);
      else
        printf("%u\t", S[i+j*C]);
    }
    printf("\n");
  }
  printf("\n");
}


void imprime_vector(uint32_t *S, uint32_t size_vector){
  int i;
  printf(" - VECTOR - \n");
  for (i = 0 ; i < size_vector; i++)
	  printf(" %u |", S[i]);

  printf("\n\n ");
}

void multiplica_matriz(uint32_t *A, uint32_t *B, uint32_t *R, uint32_t F, uint32_t C, double coeficiente) {
  int i, j, k;
  for (i = 0; i < F; i++){
    for (j = 0; j < C; j++){
      R[i*C+j] = 0;
      for (k = 0; k < C; k++){
        R[i*C+j] += A[i*C+k]*B[k+j*C];
      }
      R[i*C+j] *= coeficiente;
    }
  }
}

void suma_matrices(uint32_t *A, uint32_t *B, uint32_t *R, uint32_t F, uint32_t C) {
  int i, j, aux;
  for (i = 0; i < F; i++) {
    for (j = 0; j < C; j++) {
      aux = i*C+j;
      R[aux] = A[aux] + B[aux];
    }
  }
}

double promedio_matriz(uint32_t *A, uint32_t N) {
  int i;
  double sum = 0;
  for (i = 0; i < N*N; i++) {
    sum += A[i];
  }
  return sum/(N*N);
}

void root(uint32_t N, uint32_t cantidad_procesos) {
  int check = 1;
  int i,j,k;
  int filas = N/cantidad_procesos;
  double avg_b, avg_d, timetick;
  printf(" ==== VARIABLES \n \t* Cantidad de procesos: %d\n\t* Filas por proceso: %d\n\n", cantidad_procesos, filas);

  uint32_t *A = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  uint32_t *B = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  uint32_t *C = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  uint32_t *D = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  uint32_t *E = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  uint32_t *F = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  uint32_t *a	= (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *d = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *ab = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *abc = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *de = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *def = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *r = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *R = (uint32_t*)malloc(sizeof(uint32_t)*N*N);

  for(i = 0; i < N; i++){
    for(j = 0; j < N; j++){
      A[i*N+j] = 1;
      B[i*N+j] = 1;
      C[i*N+j] = 1;
      D[i*N+j] = 1;
      E[i*N+j] = 1;
      F[i*N+j] = 1;
    }
  }

  for(i = 0; i < N*filas; i++){
    ab[i] = 0;
    abc[i] = 0;
    de[i] = 0;
    def[i] = 0;
    r[i] = 0;
  }

  double tiempo_calculo = dwalltime();
  double overhead_1 = dwalltime();

  MPI_Bcast(B, N*N, MPI_UINT32_T, 0, MPI_COMM_WORLD);
  MPI_Bcast(C, N*N, MPI_UINT32_T, 0, MPI_COMM_WORLD);
  MPI_Scatter(A, N*filas, MPI_UINT32_T, a, N*filas, MPI_UINT32_T, 0, MPI_COMM_WORLD);

  MPI_Bcast(E, N*N, MPI_UINT32_T, 0, MPI_COMM_WORLD);
  MPI_Bcast(F, N*N, MPI_UINT32_T, 0, MPI_COMM_WORLD);
  MPI_Scatter(D, N*filas, MPI_UINT32_T, d, N*filas, MPI_DOUBLE, 0, MPI_COMM_WORLD );

  printf(" --------- Overhead 1: %f\n", dwalltime() - overhead_1);

  timetick = dwalltime();

  // El root se encarga del promedio
  avg_b = promedio_matriz(B, N);
  MPI_Bcast(&avg_b, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  avg_d = promedio_matriz(D, N);
  MPI_Bcast(&avg_d, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  multiplica_matriz(a, B, ab, filas, N, 1);
  multiplica_matriz(ab, C, abc, filas, N, avg_d);
  multiplica_matriz(d, E, de, filas, N, 1);
  multiplica_matriz(de, F, def, filas, N, avg_b);
  suma_matrices(abc, def, r, filas, N);

  printf(" --------- ID %d terminó en: %f segundos\n", 0 , dwalltime() - timetick);

  double overhead_2 = dwalltime();

  MPI_Gather(r, N*filas, MPI_UINT32_T, R, N*filas, MPI_UINT32_T, 0, MPI_COMM_WORLD);

  printf(" --------- Overhead 2: %f\n", dwalltime() - overhead_2);
  printf(" --------- Tiempo de calculo: %f segundos\n", dwalltime() - tiempo_calculo);
  // imprime_matriz(R, N, N, 0);

  // Verificación
  for(i=0;i<N;i++){
    for(j=0;j<N;j++){
      check=check&&(R[i*N+j]==(N*N+N*N));
    }
  }
  if(check){
    printf("Multiplicacion de matrices resultado correcto\n");
  } else{
    printf("Multiplicacion de matrices resultado erroneo\n");
  }

  free(A);
  free(B);
  free(C);
  free(D);
  free(E);
  free(F);
  free(R);
  free(a);
  free(d);
  free(ab);
  free(abc);
  free(de);
  free(def);
}

void worker(uint32_t ID, uint32_t N, uint32_t cantidad_procesos) {
  int check = 1;
  int i,j,k;
  int filas = N/cantidad_procesos;
  double avg_b, avg_d, timetick;

  uint32_t *A;
  uint32_t *B = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  uint32_t *C = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  uint32_t *D;
  uint32_t *E = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  uint32_t *F = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  uint32_t *a	= (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *d = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *ab = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *abc = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *de = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *def = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *r = (uint32_t*)malloc(sizeof(uint32_t)*filas*N);
  uint32_t *R;

  for(i = 0; i < N*filas; i++){
    ab[i] = 0;
    abc[i] = 0;
    de[i] = 0;
    def[i] = 0;
    r[i] = 0;
  }

  MPI_Bcast(B, N*N, MPI_UINT32_T, 0, MPI_COMM_WORLD);
  MPI_Bcast(C, N*N, MPI_UINT32_T, 0, MPI_COMM_WORLD);
  MPI_Scatter(A, N*filas, MPI_UINT32_T, a, N*filas, MPI_UINT32_T, 0, MPI_COMM_WORLD);

  MPI_Bcast(E, N*N, MPI_UINT32_T, 0, MPI_COMM_WORLD);
  MPI_Bcast(F, N*N, MPI_UINT32_T, 0, MPI_COMM_WORLD);
  MPI_Scatter(D, N*filas, MPI_UINT32_T, d, N*filas, MPI_DOUBLE, 0, MPI_COMM_WORLD );

  MPI_Bcast(&avg_b, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&avg_d, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  timetick = dwalltime();

  multiplica_matriz(a, B, ab, filas, N, 1);
  multiplica_matriz(ab, C, abc, filas, N, avg_d);
  multiplica_matriz(d, E, de, filas, N, 1);
  multiplica_matriz(de, F, def, filas, N, avg_b);
  suma_matrices(abc, def, r, filas, N);

  printf(" --------- ID %d terminó en: %f segundos\n", ID , dwalltime() - timetick);

  MPI_Gather(r, N*filas, MPI_UINT32_T, R, N*filas, MPI_UINT32_T, 0, MPI_COMM_WORLD);

  //Libera memoria
  free(B);
  free(C);
  free(E);
  free(F);
  free(a);
  free(d);
  free(ab);
  free(abc);
  free(de);
  free(def);
}
