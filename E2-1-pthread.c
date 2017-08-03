#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

double dwalltime();
void *calcular(void* index);
void imprimeMatriz(uint32_t *S,uint32_t N,int fil);
uint32_t promediar(uint32_t *M);
void multiplicaMatriz(int index, uint32_t *X, uint32_t *Y, uint32_t* M, uint32_t coef);
void sumarMatriz(int index, uint32_t *X, uint32_t *Y, uint32_t *R);

uint32_t N, T;
uint32_t *A;
uint32_t *B;
uint32_t *C;
uint32_t *D;
uint32_t *E;
uint32_t *F;
uint32_t *AB;
uint32_t *DE;
uint32_t *ABC;
uint32_t *DEF;
uint32_t *TOT;
uint32_t d, b;
pthread_barrier_t barrera_1;
pthread_barrier_t barrera_2;

int main(int argc,char*argv[]){
  if (argc < 2){
    printf("\n Falta un argumento : \n");
    printf("\t * N dimensión matriz\n");
    printf("\t * T cantidad de threads \n");
    return 1;
  }

  int sumD = 0;
  int sumB = 0;
  int check = 1;
  double timetick;
  uint64_t i, j, k;

  N = atoi(argv[1]);
  T = atoi(argv[2]);
  int threadIds[T];

  A = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  B = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  C = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  D = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  E = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  F = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  AB = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  DE = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  ABC = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  DEF = (uint32_t*)malloc(sizeof(uint32_t)*N*N);
  TOT = (uint32_t*)malloc(sizeof(uint32_t)*N*N);

  for (i=0; i<N; i++) {
    for (j=0; j<N; j++) {
      A[i*N+j] = 1;
      B[i+N*j] = 1;
      C[i+N*j] = 1;
      D[i*N+j] = 1;
      E[i+N*j] = 1;
      F[i+N*j] = 1;
    }
  }

  printf("\t * %u : dimension matriz\n", N);
  printf("\t * %u : cantidad de threads \n", T);

  pthread_t threads[T];

  pthread_barrier_init(&barrera_1, NULL, T);
  pthread_barrier_init(&barrera_2, NULL, T);

  timetick = dwalltime();

  for (i=0; i<T; i++) {
    threadIds[i] = i;
    pthread_create(&threads[i], NULL, calcular, (void*)&threadIds[i]);
  }

  for (i=0; i<T; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("Tiempo en segundos %f \n", dwalltime() - timetick);

  for (i=0; i<N; i++) {
    for (j=0; j<N; j++) {
      check=check&&(TOT[i*N+j]==(N*N+N*N));
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
  free(AB);
  free(ABC);
  free(DE);
  free(DEF);
  free(TOT);
  pthread_barrier_destroy(&barrera_1);
  pthread_barrier_destroy(&barrera_2);
  return 0;
}

/*****************************************************************/

#include <sys/time.h>

double dwalltime()
{
  double sec;
  struct timeval tv;

  gettimeofday(&tv,NULL);
  sec = tv.tv_sec + tv.tv_usec/1000000.0;
  return sec;
}

void* calcular(void* ind) {
  int index = *((int *) ind);
  uint64_t i,j,k, sumB, sumD;
  if (T > 1) {
    if (index == 0)
      b = promediar(B);
    if (index == 1)
      d = promediar(D);
  } else {
    b = promediar(B);
    d = promediar(D);
  }
  multiplicaMatriz(index, A, B, AB, 1);
  multiplicaMatriz(index, D, E, DE, 1);
  pthread_barrier_wait(&barrera_1);
  multiplicaMatriz(index, AB, C, ABC, d);
  multiplicaMatriz(index, DE, F, DEF, b);
  pthread_barrier_wait(&barrera_2);
  sumarMatriz(index, ABC, DEF, TOT);
  pthread_exit(NULL);
}

uint32_t promediar(uint32_t *M) {
  uint32_t i, j;
  uint32_t sum = 0;
  for(i=0; i<N*N; i++){
    sum = sum + M[i];
  }
  return sum / (N*N);
}

void multiplicaMatriz(int index, uint32_t *X, uint32_t *Y, uint32_t* M, uint32_t coef) {
  double aux;
  int i, j, k;
  int maxFila = index*(N/T) + ((N/T)-1);
  for(i=index*(N/T); i<=maxFila; i++){
    for(j=0;j<N;j++){
      aux =0;
      for(k=0;k<N;k++){
        aux =  aux + X[i*N+k]*Y[k+j*N];
      }
      M[i*N+j] = aux * coef;
    }
  }
}

void sumarMatriz(int index, uint32_t *X, uint32_t *Y, uint32_t *R) {
  double aux;
  int i, j, k;
  int maxFila = index*(N/T) + ((N/T)-1);
  for(i=index*(N/T); i<=maxFila; i++){
    for(j=0; j<=N; j++){
      R[i*N+j] = X[i*N+j] + Y[i*N+j];
    }
  }
}

void imprimeMatriz(uint32_t *S,uint32_t N,int fil){
  int i,j;
  printf("\nImprimiendo matriz\n");
  for(i=0;i<N;i++){
    for(j=0;j<N;j++){
      if (fil == 1)
        printf("%u\t", S[i*N+j]);
      else
        printf("%u\t", S[i+j*N]);
    }
    printf("\n");
  }
  printf("\n");
}
