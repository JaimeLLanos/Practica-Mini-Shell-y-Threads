#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, char *argv[]){
	int pipeP-H[2];
	//int pipeH-P[2];
	char buf[1024];
	pipe(pipeP-H);
	pipe(pireH-P);
	pid_t pid;
	pid = fork();
	if(pid == 0){
		close(pipeP-H[1]); //EL hijo va a recibir un mandato del padre
		read(pipeP-H[0], buf, 1024);
		
	}	
}
