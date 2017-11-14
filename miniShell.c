#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "parser.h"
#include <string.h>
#include <signal.h>

int main(int argc, char *argv[]){
	if(argc == 1){
		while(1){
			signal(SIGINT, SIG_IGN); //ignorar Ctrol+C
			int pipeP-H[2];
			char buf[1024];
			pipe(pipeP-H);
			pid_t pid;
			pid = fork();
			if(pid == 0){
				close(pipeP-H[1]); //EL hijo va a recibir un mandato del padre
				read(pipeP-H[0], buf, 1024);
				signal(SIGINT, SIG_DFL);
				//execvp(buf);
				//exit(1);
			}	
			}else{
				close(pipeP-H[0]); //El padre va a enviar un mandato	
				//recoger la entrada de teclado
				
				write(pipeP-H[1],
			}
		}
	}
	else{
		printf("El programa no necesita argumentos adicionales");
		exit(1);
	}
}
