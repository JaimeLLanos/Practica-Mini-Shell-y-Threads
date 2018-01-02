#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "parser.h"
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef struct {
	int p[2];
} tPipe;


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
	tcommand *mandato = (*mandatos).commands;
	int comandos = (*mandatos).ncommands;
	tPipe pipes[comandos-1];
	int pids [comandos];
	int i;
	tPipe p1;
	tPipe p2;
	for (i=0; i<comandos-1; i++){
		pipe(pipes[i].p);
 	}// fin for creaccion de pipes
		for(i = 0; i < comandos; i++){//creamos tantos hijos como
		pids[i] = fork();	  
		if(pids[i] < 0){ //si no se puede crear el hijo
			fprintf(stderr,"Ha ocurrido un error al crear el proceso");
			exit(1);
		}// fin if de error

		if(pids[i] == 0){ //para el hijo i
			if(i == 0){
				redireccionDeEntrada(mandatos);
				p1=pipes[i];
				dup2(p1.p[1],1);
				close(p1.p[0]);
				close(p1.p[1]);
				execvp(mandato[i].filename, mandato[i].argv);
				exit(1);

			}else if((i != 0) && (i<comandos-1)){
				p1=pipes[i-1];
				p2=pipes[i];
				dup2(p1.p[0],0);
				close(p1.p[0]);
				close(p1.p[1]);
				dup2(p2.p[1],1);
				close(p2.p[0]);
				close(p2.p[1]);
				execvp(mandato[i].filename, mandato[i].argv);
				exit(1);
			}
			else{
			     redireccionDeSalida(mandatos);
				p1=pipes[i-1];
				dup2(p1.p[0],0);
				close(p1.p[0]);
				close(p1.p[1]);
				execvp(mandato[i].filename, mandato[i].argv);
				exit(1);
			}
			redireccionDeError(mandatos);
		}// fin del pid==0
	}//fin del for 		
	wait(NULL);
}// fin del varios comandos

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
				varioscomandos(mandatos);
			}
		} //fin del while
	} else //fin del if
		fprintf(stderr,"Error, funcion sin argumentos.\n");
} //fin del main
