#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#define COCHES 8
#define CAMIONES 3
#define PLAZAS 5


int parking[PLAZAS];
pthread_mutex_t park; //mutex para el parking cuando este lleno
int ocupacion;
pthread_cond_t nolleno; //condiciones de espera

void *fcamion(void *args){
	int numero= *((int*) args); //matricula del camion
	int plibres;
	int i;
	while (1) {
		i=0;
		sleep((rand()%10)+2);
		pthread_mutex_lock(&park); //un camion entra al parking
		while ((ocupacion==PLAZAS) || (ocupacion==PLAZAS-1)) {
			pthread_cond_wait(&nolleno, &park); //en caso de que estuviera lleno o no cupiese nuestro camion, esperamos
		}
		while (parking[i]!=0)
			i++; //recorremos las posiciones del parking hasta encontrar una libre
		if ((i<PLAZAS-1) && (parking[i+1]==0)){ //si hubiera dos plazas libres, y además fueran consecutivas
		parking[i]=numero; //ponemos la posicion actual y la siguiente como "ocupada" por nuestro camion
		parking[i+1]=numero;
		ocupacion+=2;
		plibres=PLAZAS-ocupacion; //hallamos el numero de plazas libres
		printf("ENTRADA: Camion %d aparcando en plaza %d. Plazas libres: %d\n", numero, i+1, plibres);
		printf("Parking: ");
		for (i=0; i<PLAZAS; i++) {
			printf("[%d] ", parking[i]); //imprimos el estado actual del parking 
		} 
		printf("\n"); 
		
		pthread_mutex_unlock(&park); //una vez aparcado se libera el mutex
		sleep((rand()%10)+2);
		pthread_mutex_lock(&park); //cerramos el mutex para que salga nuestro camion
		i=0;
		while (parking[i]!=numero) //buscamos el camion en el parking
			i++;
		parking[i+1]=0; //ponemos como vacias las plazas que ocupaba
		parking[i]=0;
		ocupacion-=2; //actualizamos la ocupacion actual
		plibres=PLAZAS-ocupacion; //y el numero de plazas libres que quedan
		printf("SALIDA: Camion %d saliendo. Plazas libres: %d\n", numero, plibres);
		printf("Parking: ");
		for (i=0; i<PLAZAS; i++) {
			printf("[%d] ", parking[i]); //imprimos el estado actual del parking
		}
		printf("\n");
		pthread_cond_signal(&nolleno); //al haber vaciado 2 posiciones del parking, señalizamos que el parking no esta lleno
		} //fin del if parking i+1
		pthread_mutex_unlock(&park); //liberamos el mutex
	}  //fin de while
}//fin camion

void *fcoche(void *args){
	int numero= *((int*) args); //matricula del coche
	int plibres;
	int i;
	while (1) {
		i=0;
		sleep((rand()%10)+2);
		pthread_mutex_lock(&park); //un coche entra en el parking
		while (ocupacion==PLAZAS) {
			pthread_cond_wait(&nolleno, &park);//en caso de que estuviera lleno o no cupiese nuestro coche esperamos
		}
		while (parking[i]!=0)
			i++; //recorremos las posiciones del parking hasta encontrar una libre
		parking[i]=numero; //y aparcamos el coche en la posicion libre
		ocupacion+=1; //aumentando tambien el contador que marca la ocupacion
		plibres=PLAZAS-ocupacion;
		printf("ENTRADA: Coche %d aparcando en plaza %d. Plazas libres: %d\n", numero, i+1, plibres);
		printf("Parking: ");
		for (i=0; i<PLAZAS; i++) {
			printf("[%d] ", parking[i]); //estado actual del parking
		}
		printf("\n");
		pthread_mutex_unlock(&park); //liberamos el mutex
		sleep((rand()%10)+2);
		pthread_mutex_lock(&park); //bloqueamos el mutex, porque un coche va a salir
		i=0;
		while (parking[i]!=numero)
			i++; //buscamos el coche en el parking
		parking[i]=0; //una vez encontrado, ponemos como libre la plaza que ocupaba
		ocupacion-=1; 
		plibres=PLAZAS-ocupacion; //actualizamos la ocupacion y las plazas libres que quedan
		printf("SALIDA: Coche %d saliendo. Plazas libres: %d\n", numero, plibres);
		printf("Parking: ");
		for (i=0; i<PLAZAS; i++) {
			printf("[%d] ", parking[i]); //estado actual del parking
		}
		printf("\n");
		pthread_cond_signal(&nolleno); //al haber vaciado 1 posicion del parking, señalizamos que no esta lleno
		pthread_mutex_unlock(&park); //liberamos el mutex
	} //fin del while
}//fin coche

int main (int argc, char* argv[]){ //inicio de main
	int i;
	int ncoches[COCHES]; //array con el numero maximo de coches predefinido
	int ncamiones[CAMIONES]; //array con el numero maximo de camiones predefinido
	ocupacion=0;
	pthread_t coches[COCHES]; //array de threads, cada thread corresponde a un coche
	pthread_mutex_init(&park, NULL); //mutex que limitará la entrada de vehiculos al parking de uno en uno
	pthread_cond_init(&nolleno, NULL); //condicion que bloqueará la accion si el parking estuviera lleno
	pthread_t camiones[CAMIONES]; //array de threads, cada thread corresponde a un camion
	for (i=0; i<PLAZAS; i++) {
		parking[i]=0; //se inicializan las plazas de aparcamiento a 0 (vacias)
	}
	for (i=0; i<COCHES; i++){
		ncoches[i]=i+1; //se da nombre a los coches (0,1...n)
	}
	for (i=0; i<CAMIONES; i++) {
		ncamiones[i]=101+i; //se da nombre a los camiones (101, 102... n)
	}
	for (i=0; i<COCHES; i++) {
		pthread_create(&coches[i], NULL, fcoche, (void*) &ncoches[i]); //se crean los threads de los coches
	}
	for (i=0; i<CAMIONES; i++) {
		pthread_create(&camiones[i], NULL, fcamion, (void*) &ncamiones[i]); //se crean los threads de los camiones
	}
	while (1);
} // fin del main
