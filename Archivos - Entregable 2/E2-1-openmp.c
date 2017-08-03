#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>

double dwalltime();

int main(int argc,char*argv[]){
  //Controla los argumentos al programa
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
  uint32_t d, b, aux;
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
  uint64_t N, T, i, j, k;

  N = atoi(argv[1]);
  T = atoi(argv[2]);
  omp_set_num_threads(T);

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

  // Inicializo todas las matrices con 1s
  aux = 1;
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

  printf("\t * %lu : dimension matriz\n", N);
  printf("\t * %lu : cantidad de threads \n", T);

  timetick = dwalltime();

  //obtener promedios
  #pragma omp parallel for private(i,j)
    for (i=0; i<N; i++)
      for (j=0; j<N; j++) {
        #pragma omp critical (lock1)
        {
          sumD += D[i+N*j];
          sumB += B[i+N*j];
        }
      }

  // A B
  #pragma omp parallel for private(i,j,k)
    for (i=0; i<N; i++) {
      for (j=0; j<N; j++) {
        AB[i*N+j] = 0;
        for (k=0; k<N; k++) {
          AB[i*N+j] = AB[i*N+j] + A[i*N+k]*B[k+j*N];
        }
      }
    }

  // D E
  #pragma omp parallel for private(i,j,k)
    for (i=0; i<N; i++) {
      for (j=0; j<N; j++) {
        DE[i*N+j] = 0;
        for (k=0; k<N; k++) {
          DE[i*N+j] = DE[i*N+j] + D[i*N+k]*E[k+j*N];
        }
      }
    }

  d = (float) sumD / (N*N);
  b = (float) sumB / (N*N);

  // [AB]*[C]*d
  #pragma omp parallel for private(i,j,k)
    for (i=0; i<N; i++) {
      for (j=0; j<N; j++) {
        ABC[i*N+j] = 0;
        for (k=0; k<N; k++) {
          ABC[i*N+j] = ABC[i*N+j] + AB[i*N+k]*C[k+j*N];
        }
        ABC[i*N+j] = ABC[i*N+j]*d;
      }
    }

  // [DE]*[F]*d
  #pragma omp parallel for private(i,j,k)
    for (i=0; i<N; i++)
      for (j=0; j<N; j++) {
        DEF[i*N+j] = 0;
        for (k=0; k<N; k++)
          DEF[i*N+j] = DEF[i*N+j] + DE[i*N+k]*F[k+j*N];
        DEF[i*N+j] = DEF[i*N+j]*d;
      }

  #pragma omp parallel for collapse(2) private(i,j,k)
    for (i=0; i<N; i++)
      for (j=0; j<N; j++)
        TOT[i*N+j] = ABC[i*N+j] + DEF[i*N+j];

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
