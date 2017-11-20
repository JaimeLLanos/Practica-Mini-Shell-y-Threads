#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "parser.h"
#include <string.h>

int main(int argc, char *argv[]){
	if(argc == 1){
		char buf[1024];
		tline *entrada;
		printf("msh> ");
		while(fgets(buf,1024,stdin)){
			pid_t pid;
			int status;
			pid = fork();
			entrada = tokenize(buf);
			if(entrada.ncommands == 1){
				if(pid < 0){
					printf("Falló el fork()");
				}
				else if(pid == 0){
					if(entrada.commands[0].filename == NULL)
						printf("El mandato especificado no existe");
					else{
						execvp(entrada.commands[0].argv[0], argv + 1);
						printf("Error al ejecutar el comando");
						exit(1);
					}
				} else{
					wait(&status);
					if(WIFEXITED(status) != 0)
						if(WEXITSTATUS(status) != 0)
							printf("El comando no se ejecutó correctamente");
					exit(0);
				}
				
			}
			printf("msh> ");
		}
	}else{
		printf("La mini Shell no tiene que recibir ningun argumento");
	}	
}

}
