#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "parser.h"
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void uncomando(tcommand *mandato){
	
	pid_t pid;
	pid=fork();
	if (pid<0) {
		printf("fallo en el fork.\n");
	} else if (pid==0) {
		signal(SIGQUIT, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		printf("Soy el hijo, ejecuto el comando.\n");
		execvp((*mandato).filename,(*mandato).argv);
		printf("error execvp\n");
		exit(1);
	} else { 
		printf("padre\n");
		wait(NULL);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		} //fin padre
}//fin unmandato

void varioscomandos(){
}

int main(int argc, char* argv[]){ //inicio main
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	tline *mandatos; //variable de hacer tokenize
	char buffer[1024];
	if (argc==1) {
		while (1) { //inicio de while
			printf("msh> \n");
			fgets(buffer, 1024, stdin); //recogemos en buffer la variable de entrada de teclado
			mandatos=tokenize(buffer);
			if ((*mandatos).ncommands==1) {
				uncomando((*mandatos).commands);
			} else if ((*mandatos).ncommands>1) {
				varioscomandos();
			}
		} //fin del while
	} else //fin del if
		printf("Error, funcion sin argumentos.\n");
} //fin del main
