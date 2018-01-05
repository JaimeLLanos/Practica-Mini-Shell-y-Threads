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
		if(fd == -1){
			fprintf(stderr,"%s: Error. No existe el fichero.\n",(*mandato).redirect_input);
			exit(1);
		}else{
			dup2(fd,0); //El 0 indica que es entrada de lectura
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
			dup2(fd,1); //El 1 indica que es salida de escritura
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
			dup2(fd,2); //El 2 indica que es salida de error
		}
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
	int i;
	tPipe p1;
	tPipe p2;
	for (i = 0; i < comandos-1; i++){
		pipe(pipes[i].p);
 	}// fin for creaccion de pipes
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	for(i = 0; i < comandos; i++){//creamos tantos hijos como comandos tenga la instrucción
		pids[i] = fork();	  
		if(pids[i] < 0){ //si no se puede crear el hijo
			fprintf(stderr,"Ha ocurrido un error al crear el proceso");
			exit(1);
		}// fin if de error

		if(pids[i] == 0){ //para el hijo i
			if(i == 0){ //primer mandato
				redireccionDeEntrada(mandatos);
				p1=pipes[i];
				dup2(p1.p[1],1); //se va a escribir en la entrada del pipe-...
				close(p1.p[0]);
				close(p1.p[1]);
				execvp(mandato[i].filename, mandato[i].argv); //...-la ejecucion del primer mandato
				exit(1);

			}else if((i != 0) && (i<comandos-1)){ //mandato intermedio (ni el primero ni el último)
				p1=pipes[i-1];
				p2=pipes[i];
				dup2(p1.p[0],0); //se va a leer de la salida del pipe anterior (i-1)
				close(p1.p[0]);
				close(p1.p[1]);
				dup2(p2.p[1],1); //se va a escribir en la entrada del siguiente pipe (i)-...
				close(p2.p[0]);
				close(p2.p[1]);
				execvp(mandato[i].filename, mandato[i].argv);//...-la ejecucion del mandato i
				exit(1);
			}
			else{ //último mandato
			     	redireccionDeSalida(mandatos);
				redireccionDeError(mandatos);
				p1=pipes[i-1];
				dup2(p1.p[0],0); //se va a leer de la salida del último pipe-...
				close(p1.p[0]);
				close(p1.p[1]);
				execvp(mandato[i].filename, mandato[i].argv);//...-y se va a ejecutar el último mandato
				//de esta forma, el último mandato recibirá las salidas de todos los mandatos anteriores para 
				//asi ejecutarse exactamente como queremos.
				exit(1);
			}
		}// fin del pid==0
	}//fin del for 		
	wait(NULL);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
}// fin del varios comandos

void cd(tline *mandatos){
	tcommand *mandato = (*mandatos).commands;
	if((*mandato).argv[1] == NULL){ //si no recibe un directorio
		chdir(getenv("HOME")); //el comando cd nos llevará al directorio home
	}else{
		chdir((*mandato).argv[1]); //en caso contrario cambiamos al directorio especificado
	}
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
