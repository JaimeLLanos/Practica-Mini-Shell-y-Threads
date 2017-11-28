#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "parser.h"
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


void *redireccionDeEntrada(tline *mandato){
	int fd;	
	if((*mandato).redirect_input != NULL){
		fd = open((*mandato).redirect_input,O_RDONLY);
		if(fd == -1){
			 fprintf(stderr,"%s: Error. No existe el fichero.\n",(*mandato).redirect_input);
			exit(1);
		}else{
			dup2(fd,0); //El 0 indica que es de lectura
		}
	}
}

void *redireccionDeSalida(tline *mandato){
	int fd;
	if((*mandato).redirect_output != NULL){
		fd = open((*mandato).redirect_output,O_WRONLY | O_CREAT | O_TRUNC);
		if(fd == -1){
			fprintf(stderr,"%s: Error. Fallo al crear el fichero para escritura.",(*mandato).redirect_output);
			exit(1);
		}else{
			dup2(fd,1); //El 1 indica que es de escritura
		}
	}
	
}

void *redireccionDeError(tline *mandato){
	int fd;
	if((*mandato).redirect_error != NULL){
		fd = open((*mandato).redirect_error, O_WRONLY | O_CREAT | O_TRUNC);		
		if(fd == -1){
			fprintf(stderr,"%s: Error. Fallo al crear el fichero para su escritura",(*mandato).redirect_error);
			exit(1);
		}else{
			dup2(fd,2); //El 2 indica que es de error
		}
	}
}

void uncomando(tline *mandatos){
	pid_t pid;
	tcommand *mandato = (*mandatos).commands;
	pid=fork();
	if (pid < 0) {
		fprintf(stderr,"fallo en el fork.\n");
	} else if (pid == 0) {
		signal(SIGQUIT, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		redireccionDeEntrada(mandatos);
		redireccionDeSalida(mandatos);
		redireccionDeError(mandatos);
		if((*mandato).filename == NULL){
			fprintf(stderr,"%s: No se encuentra el mandato\n",(*mandato).filename);
			exit(1);
		}else{
			execvp((*mandato).filename,(*mandato).argv);
			fprintf(stderr,"error execvp\n");
			exit(1);		
		}
	} else { 
		wait(NULL);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		} //fin padre
}//fin unmandato

void varioscomandos(tline *mandatos){
	int p1[2];
	int p2[2];
	int comandos = (*mandatos).ncommands;
	int *pipes[comandos];
	int [comandos] pids;
	int i;
	for(i = 0; i < comandos; i++){//creamos tantos hijos como
		pids[i] = fork();	   //procesos haya
		if(pids[i] < 0){ //si no se puede crear el hijo
			fprintf(stderr,"Ha ocurrido un error al crear el proceso");
			exit(1);
		}
		if(pids[i] == 0){ //para el hijo i
			pipe(p1);
			pipe(p2);
			p1 = pipes + i;
			p2 = pipes + i - 1;
			if(i == (comandos - 1)){
				close(p1[0]);
				dup2(p1[1],1);
				close(p1[1]);
			}else if(i != 0){
				close(p1[1]);
				dup2(p1[0], 0);
				close(p1[0]);
				close(p2[0]);
				dup2(p2[1],1);
				close(p2[1]);
			}
			else{
				close(p1[1]);
				dup2(p1[0],0);
				close(p1[0]);
				execvp((*mandatos).filename, (*mandatos).argv);
				exit(1);
			}
		}

	}			
}

int main(int argc, char* argv[]){ //inicio main
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	tline *mandatos; //variable para hacerle el tokenize
	char buffer[1024];
	printf("msh> ");
	if (argc == 1) {
		while (fgets(buffer, 1024, stdin)) { //inicio de while
			//recogemos en buffer la variable de entrada de teclado
			printf("msh> ");			
			mandatos = tokenize(buffer);
			if ((*mandatos).ncommands == 1) {
				uncomando(mandatos);
			} else if ((*mandatos).ncommands > 1) {
				varioscomandos();
			}
		} //fin del while
	} else //fin del if
		fprintf(stderr,"Error, funcion sin argumentos.\n");
} //fin del main
