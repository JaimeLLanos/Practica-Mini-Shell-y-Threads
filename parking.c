#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#define COCHES 8
#define CAMIONES 3
#define PLAZAS 5


//pthread_mutex_t plazas[PLAZAS]; (quitar esta linea que no se usa)
int parking[PLAZAS];
pthread_mutex_t park; //mutex para el parking cuando este lleno
int ocupacion;
pthread_cond_t nolleno; //condiciones de espera

void *fcamion(void *args){
	int numero= *((int*) args);
	int plibres;
	int i;
	while (1) {
		i=0;
		sleep((rand()%10)+2);
		pthread_mutex_lock(&park);
		while ((ocupacion==PLAZAS) || (ocupacion==PLAZAS-1)) {
			pthread_cond_wait(&nolleno, &park);
			}
		while (parking[i]!=0)
			i++;
		if ((i<PLAZAS-1) && (parking[i+1]==0)){
		parking[i]=numero;
		parking[i+1]=numero;
		ocupacion+=2;
		plibres=PLAZAS-ocupacion;
		printf("ENTRADA: Camion %d aparcando en plaza %d. Plazas libres: %d\n", numero, i+1, plibres);
		printf("Parking: ");
		for (i=0; i<PLAZAS; i++) {
			printf("[%d] ", parking[i]); }
		printf("\n"); 
		
		pthread_mutex_unlock(&park);
		sleep((rand()%10)+2);
		pthread_mutex_lock(&park);
		i=0;
		while (parking[i]!=numero)
			i++;
		parking[i+1]=0;
		parking[i]=0;
		ocupacion-=2;
		plibres=PLAZAS-ocupacion;
		printf("SALIDA: Camion %d saliendo. Plazas libres: %d\n", numero, plibres);
		printf("Parking: ");
		for (i=0; i<PLAZAS; i++) {
			printf("[%d] ", parking[i]); }
		printf("\n");
		pthread_cond_signal(&nolleno);
		} //fin del if parking i+1
		pthread_mutex_unlock(&park); 
	}  //fin de while
}//fin camion

void *fcoche(void *args){
	int numero= *((int*) args);
	int plibres;
	int i;
	while (1) {
		i=0;
		sleep((rand()%10)+2);
		pthread_mutex_lock(&park);
		while (ocupacion==PLAZAS) {
			pthread_cond_wait(&nolleno, &park);
			}
		while (parking[i]!=0)
			i++;
		parking[i]=numero;
		ocupacion+=1;
		plibres=PLAZAS-ocupacion;
		printf("ENTRADA: Coche %d aparcando en plaza %d. Plazas libres: %d\n", numero, i+1, plibres);
		printf("Parking: ");
		for (i=0; i<PLAZAS; i++) {
			printf("[%d] ", parking[i]); }
		printf("\n");
		pthread_mutex_unlock(&park);
		sleep((rand()%10)+2);
		pthread_mutex_lock(&park);
		i=0;
		while (parking[i]!=numero)
			i++;
		parking[i]=0;
		ocupacion-=1;
		plibres=PLAZAS-ocupacion;
		printf("SALIDA: Coche %d saliendo. Plazas libres: %d\n", numero, plibres);
		printf("Parking: ");
		for (i=0; i<PLAZAS; i++) {
			printf("[%d] ", parking[i]); }
		printf("\n");
		pthread_cond_signal(&nolleno);
		pthread_mutex_unlock(&park);
	} //fin del while
}//fin coche

int main (int argc, char* argv[]){ //inicio de main
	int i;
	int ncoches[COCHES];
	int ncamiones[CAMIONES];
	ocupacion=0;
	pthread_t coches[COCHES]; //creo que esto no hace falta ponerlo (probar quitandolo)
	pthread_mutex_init(&park, NULL);
	pthread_cond_init(&nolleno, NULL);
	pthread_t camiones[CAMIONES];
	for (i=0; i<PLAZAS; i++) {
		parking[i]=0; }
	for (i=0; i<COCHES; i++){
		ncoches[i]=i+1;}
	for (i=0; i<CAMIONES; i++) {
		ncamiones[i]=101+i; }
	for (i=0; i<COCHES; i++) {
		pthread_create(&coches[i], NULL, fcoche, (void*) &ncoches[i]); }
	for (i=0; i<CAMIONES; i++) {
		pthread_create(&camiones[i], NULL, fcamion, (void*) &ncamiones[i]); }
	while (1);
} // fin del main
