// MergeSort V1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//--------------------------------------------
// VARIABLES GLOBALES
//--------------------------------------------

int *data;	// Datos a ordenar
int *L;		// Estructura adicional para hacer merge

int N;		// Cantidad total de elementos a ordenar
int M;		// Valor maximo que puede haber en los datos
int T;		// Cantidad de threads
int I;		// Ver información detallada en consola

pthread_t *threads;

// Estructura con información referente al hilo
typedef struct {
	int id;
	int extremoIni;
	int extremoFin;
	pthread_mutex_t lockDatos;
} infoThread;

infoThread *infoThreads;





//--------------------------------------------
// FUNCIONES
//--------------------------------------------

double dwalltime()
{
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

	printf("[");
	for ( i = 0; i<cant; i++) {
		if (i == cant - 1)
			printf("%u]\n", v[i]);
		else
			printf("%u, ", v[i]);
	}
}

char IsSorted(int data[], int size)
{
	int i;

	for (i = 0; i<(size - 1); i++)
	{
		if (data[i] > data[i + 1])
			return 0;
	}
	return 1;
}

void Merge(int datos[], int posIni1, int posFin1, int posIni2 , int posFin2)
{
	int indiceL, indiceR;
	int i;

	// Verifica que no se desborde el maximo
	if (posFin1 >= N){
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


void* MergeSort(void* idThread) {
	int id = *((int *) idThread);
	int i, j;

	// ======================================
	// Determina porción a ordenar
	// ======================================
	
	// Garantiza que los datos se repartan en partes entera entre los hilos
	int proporcionEntera = N - N % T;	
	int ini = id*(proporcionEntera/T);
	int fin;

	// Para que cada hilo ordene una parte entera de elementos se omite N % T: el último hilo lo ordena
	if( id == (T-1))
		fin = (id+1)*(proporcionEntera/T) - 1 + N % T;
	else
		fin = (id+1)*(proporcionEntera/T) - 1;

	infoThreads[id].extremoIni = ini;
	infoThreads[id].extremoFin = fin;

	int largo = fin - ini + 1;



	// ======================================
	// Ordena su porción de datos utilizando MergeSort
	// ======================================
 	
	for ( i = 1; i < largo; i *= 2) {
		for ( j = 0; j < largo; j += (2 * i)) {
			Merge(data, ini + j, ini + j + i - 1, ini + min(j + i, largo - 1), ini + min((j + 2 * i - 1), largo - 1));
		}
	}

	if( I ){
		printf("Thread %d: completo ordenacion para intervalo [%d ; %d]\n" , id, ini, fin);
		imprimeVector(&data[ini] , fin - ini + 1);
	}


	// ======================================
	// Hace un merge de subarreglos ordenados, según le corresponda al hilo
	// ======================================
	
	int idRelativo = id;

	// Excepto el hilo main: si soy el último thread, o un número impar cuando termine de ordenar finalizo
	if( id > 0 ){
		if( (id % 2 == 1) || (id +  1) == T ){
			if( I )
				printf("Thread %d: mi vida simplemente no tiene sentido\n" , id);
			pthread_mutex_unlock( &infoThreads[id].lockDatos );
			pthread_exit(NULL);
		}
	}

	// Se hace merge entre los segmentos ordenados por los hilos
	for( i = 1 ; i < T ; i *= 2 ){

		// Mutex garantiza que los datos con los que se desea hacer merge ya estén ordenados
		pthread_mutex_lock( &infoThreads[id+i].lockDatos );
		pthread_join(threads[id+i], NULL);
		pthread_mutex_destroy( &infoThreads[id+i].lockDatos );
		if( I )
			printf("Thread %d: hago un merge con %d. Rango [%d ; %d] y [%d ; %d]\n" , id, id+i, ini, infoThreads[id].extremoFin,  infoThreads[id+i].extremoIni , infoThreads[id+i].extremoFin );
		
		// Se efectua el merge
		Merge(data, ini , infoThreads[id].extremoFin , infoThreads[id+i].extremoIni, infoThreads[id+i].extremoFin );
		infoThreads[id].extremoFin = infoThreads[id+i].extremoFin;

		// Se actualiza el id: cuando mi posición relativa a la cantidad de hilos activos es impar, termino
		idRelativo /= 2;
		if( idRelativo % 2 == 1){
			if( I ) 
				printf("Thread %d: Mi vida ya no tiene sentido\n" , id);
			pthread_mutex_unlock( &infoThreads[id].lockDatos );
			pthread_exit(NULL);
		}
	}	
}





//--------------------------------------------
// PROGRAMA PRINCIPAL
//--------------------------------------------

int main(int argc,char*argv[])
{
	int i, j;

	// Controla los argumentos recibidos
	if (argc < 4){
	    printf("\n Falta un argumento : \n");
	    printf("\t * N cantidad de elementos del vector\n");
	    printf("\t * M numero maximo de cada elemento del vector\n");
	    printf("\t * T cantidad de threads \n");
	    printf("\t * I imprime?\n");
    	return 1;
  	}	

  	N = atoi(argv[1]);
  	M = atoi(argv[2]);
  	T = atoi(argv[3]);
  	I = atoi(argv[4]);
	
	// Reserva de memoria asociada a los datos
	data = 			(int *) malloc(sizeof(int)*N);
	L = 			(int *) malloc(sizeof(int)*N);

	// Reserva de memoria asociada a los threads
	infoThreads = 	(infoThread *) malloc(sizeof(infoThread)*T);
	threads = 		(pthread_t *) malloc(sizeof(pthread_t)*T);


	if (data == NULL || L == NULL ) {
		printf("No hay memoria disponible\n");
	}

	for ( i = 0; i < N; i++)
		data[i] = rand() % M;

	if( I )
		imprimeVector(data,N);


	// ========================================================================
  	// PARTE PARALELA
  	// ========================================================================
  	
  	double timetick = dwalltime();
 
  	// Se crea T-1 threads y el hilo main también trabaja
  	for (i=1; i<T; i++) { 

  		// Garantiza que ningun otro hilo intente acceder a los datos hasta que no estén ordenados
  		pthread_mutex_init(&infoThreads[i].lockDatos,NULL);
  		pthread_mutex_lock(&infoThreads[i].lockDatos);

  		// Crea el hilo con su correspondiente id
    	infoThreads[i].id = i;
    	pthread_create(&threads[i], NULL, MergeSort, (void*)&infoThreads[i].id);
  	}
  	
  	infoThreads[0].id = 0;
	MergeSort( &infoThreads[0].id );


	printf("Tiempo en segundos %f \n", dwalltime() - timetick);

   	// ========================================================================
  	// FIN PARTE PARALELA
  	// ========================================================================

	
	if( I )
		imprimeVector(data,N);

	if (IsSorted(data, N))
		printf("Ordenado correctamente :)\n");
	else
		printf("Vector desordenado :(\n");

	

	free(threads);
	free(infoThreads);
	free(data);
	free(L);
	
	return 0;
}