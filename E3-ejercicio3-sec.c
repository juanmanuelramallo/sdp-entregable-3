#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/time.h"

double dwalltime(){
  double sec;
  struct timeval tv;

  gettimeofday(&tv,NULL);
  sec = tv.tv_sec + tv.tv_usec/1000000.0;
  return sec;
}

typedef struct pair {
    int num;
    int count;
} Pair;
int N;

int agregar_ordenado(Pair* hash, int num, int tamano, int count){
	int i=0 j=0;;
  if (tamano == 0){
    hash[0].num = num;
    hash[0].count = count;
    tamano++;
  } else {
 		while ((i < tamano) && (hash[i].num != num)){
  		i++;
	   }

  	if (hash[i].num == num){
  		hash[i].count += veces;
  	} else {
    	if (tamano < N){
    		hash[tamano].num = num;
    		hash[tamano].count = count;
    		tamano++;
    	}	else {
      	printf("Vector lleno.\n");
    	}
  	}
  }

  Pair aux;

  for (j = i; j > 0; j--){

  	if(array[j].count>array[j-1].count){

      	aux=array[j-1];
      	array[j-1]=array[j];
      	array[j]=aux;
    	}
  }
  return cantidad;
}

void printList(node *head,int c){

    for(int i=0;i<c;i++){
    	printf("Numero: %d  Cantidad: %d\n",head[i].data,head[i].count );
    }
}

int main(int argc, char *argv[]){

	int *A;
	node *B;

	if (argc < 3){
  		printf("Error, faltan opciones.\n");
  		printf("N: Cantidad de Elementos\n");
  		printf("M: Mostrar Arreglo: 1 si o 0 no\n");
  		return 0;
  	}

	N = atoi(argv[1]);
	int mostrar = atoi(argv[2]);

	A = (int*)malloc(sizeof(int)*N);
 	B = (node*)malloc(sizeof(node)*N);

 	double timetick;

	for (int i=0;i<N;i++){
		A[i] = rand()%10000;
		B[i].data=-1;
	}

	if (mostrar == 1){
		for (int i = 0; i < N; ++i){
			printf("El arreglo completo es: A[%d] = %d\n",i,A[i]);
		}
	}

	timetick = dwalltime();

	int cantidad = 0;

	for (int i = 0; i < N; i++){
	    cantidad=sortedInsert(B,A[i],cantidad,1);
	}

	printf("El tiempo en completar todo el programa en segundos es: %f \n", dwalltime() - timetick);

	if (mostrar == 1){

		printf("Después del merge, tengo este vector:\n");
    	//ver si se puede hacer que queden los n elementos con sus cantidades o si deben ser si o si 100
        for(int i=0;i<cantidad;i++){
			printf("Numero= %d Cantidad= %d \n",B[i].data,B[i].count);
		}
	}

	return 0;
}
