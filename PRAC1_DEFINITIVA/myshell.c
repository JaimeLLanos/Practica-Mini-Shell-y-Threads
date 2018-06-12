#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "parser.h"
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


//Estructura de tipo de dato tPipe, para facilitar el trabajo con el array de pipes en el método "variosComandos"
typedef struct {
	int p[2];
} tPipe;


void *redireccionDeEntrada(tline *mandato){
	int fd;	
	if((*mandato).redirect_input != NULL){
		fd = open((*mandato).redirect_input,O_RDONLY); //cogemos el descriptor de fichero asociado a nuestro mandato
		if(fd<0){
			fprintf(stderr,"%s: Error. No existe el fichero.\n",(*mandato).redirect_input);
			exit(1);
		}
		dup2(fd,0); //El 0 indica que es entrada de lectura
		close(fd);
	}
}

void *redireccionDeSalida(tline *mandato){
	int fd;
	if((*mandato).redirect_output != NULL){
		fd = open((*mandato).redirect_output,O_WRONLY | O_CREAT | O_TRUNC);
		if(fd<0){
			fprintf(stderr,"%s: Error. Fallo al crear el fichero para escritura.",(*mandato).redirect_output);
			exit(1);
		}
		dup2(fd,1); //El 1 indica que es salida de escritura
		close(fd);
	}
	
}

void *redireccionDeError(tline *mandato){
	int fd;
	if((*mandato).redirect_error != NULL){
		fd = open((*mandato).redirect_error, O_WRONLY | O_CREAT | O_TRUNC);		
		if(fd<0){
			fprintf(stderr,"%s: Error. Fallo al crear el fichero para su escritura",(*mandato).redirect_error);
			exit(1);
		}
		dup2(fd,2); //El 2 indica que es salida de error
		close(fd);
	}
}

void unComando(tline *mandatos){ //un solo comando o mandato por instruccion
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

void variosComandos(tline *mandatos){ //n mandatos con el uso de pipes
	tcommand *mandato = (*mandatos).commands;
	int comandos = (*mandatos).ncommands;
	tPipe pipes[comandos-1];
	pid_t pids [comandos];
	pid_t pid;
	int i, status;
	tPipe p1;
	tPipe p2;
	for (i = 0; i < comandos-1; i++){
		pipe(pipes[i].p);
 	}// fin for creaccion de pipes
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	for(i = 0; i < comandos; i++){//creamos tantos hijos como comandos tenga la instrucción
		pid = fork();	  
		if(pid < 0){ //si no se puede crear el hijo
			fprintf(stderr,"Ha ocurrido un error al crear el proceso");
			exit(1);
		}// fin if de error

		if(pid == 0){ //para el hijo i
			p1=pipes[i-1];
			p2=pipes[i];
			if(i == 0){ //primer mandato
				dup2(p2.p[1],1); //se va a escribir en la entrada del pipe-...
				redireccionDeEntrada(mandatos);	
			}
				
			if((i != 0) && (i<comandos-1)){ //mandato intermedio (ni el primero ni el último)				
				dup2(p1.p[0],0); //se va a leer de la salida del pipe anterior (i-1)

				dup2(p2.p[1],1); //se va a escribir en la entrada del siguiente pipe (i)-...			
			}else if(i == comandos-1){ //último mandato
				dup2(p1.p[0],0); //se va a leer de la salida del último pipe-...
				redireccionDeSalida(mandatos);
				redireccionDeError(mandatos);
			}
			
			//cerramos todos los pipes del proceso hijo por ambos extremos
			for(i=0; i<comandos-1; i++){
				close(pipes[i].p[0]);
				close(pipes[i].p[1]);
			}

			
			execvp(mandato[i].filename, mandato[i].argv);
			exit(1);
		}// fin del pid hijo
		
		else if(pid>0){
			pids[i] = pid;
		}
		signal(SIGQUIT, SIG_IGN);
		signal(SIGINT, SIG_IGN);
	}//fin del for 	
	
	//cerramos todos los pipes de los procesos padre por ambos extremos
	for(i=0; i<comandos-1; i++){
			close(pipes[i].p[0]);
			close(pipes[i].p[1]);
	}
	
	for(i = 0; i<comandos; i++){
		waitpid(pids[i],&status,0);
	}
}// fin del varios comandos

void cd(tline *mandatos){
	tcommand *mandato = (*mandatos).commands;
	if((*mandato).argv[1] == NULL){ //si no recibe un directorio
		chdir(getenv("HOME")); //el comando cd nos llevará al directorio home
	}else{
		chdir((*mandato).argv[1]); //en caso contrario cambiamos al directorio especificado
	}
}

void jobs(){
//no implementada
}

void fg(){
//no implementada
}

int main(int argc, char* argv[]){ //inicio main
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	tline *mandatos; //variable para hacerle el tokenize	
	char buffer[1024];
	printf("msh> "); //prompt
	if (argc == 1) {
		while (fgets(buffer, 1024, stdin)) { //inicio de while
			//recogemos en buffer la entrada de teclado
			printf("msh> ");			
			mandatos = tokenize(buffer); //con tokenize, transformamos dicha entrada en un tline
			if ((*mandatos).ncommands == 1) { //si la instruccion es de un solo comando
				tcommand *mandato = (*mandatos).commands; //se crea esta variable para la comprobacion de argv[0]
				if(strcmp((*mandato).argv[0],"cd") == 0){ //comprobamos si dicho comando es el comando cd
					cd(mandatos);
				}else if(strcmp((*mandato).argv[0],"jobs") == 0){
					jobs();
				}else if(strcmp((*mandato).argv[0],"fg") == 0){
					fg();
				}else{
					unComando(mandatos);
				}

			} else if ((*mandatos).ncommands > 1) { //si la instruccion leida contiene más de un mandato (usa pipes)
				variosComandos(mandatos);
			}
		} //fin del while
	} else //fin del if
		fprintf(stderr,"Error, funcion sin argumentos.\n");
} //fin del main