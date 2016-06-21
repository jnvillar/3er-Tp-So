#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[]) {
	int status;
	pid_t child;

	if (argc <= 1) {
		fprintf(stderr, "Uso: %s commando [argumentos ...]\n", argv[0]);
		exit(1);
	}

	/* Fork en dos procesos */
	child = fork();
	if (child == -1) { perror("ERROR fork"); return 1; }
	if (child == 0) {
		/* S'olo se ejecuta en el Hijo */
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execvp(argv[1], argv+1);
		/* Si vuelve de exec() hubo un error */
		perror("ERROR child exec(...)"); exit(1);
	} else {
		/* S'olo se ejecuta en el Padre */
		while(1) {
			if (wait(&status) < 0) { perror("waitpid"); break; }			
			if (WIFEXITED(status)) break; /* Proceso terminado */
			ptrace(PTRACE_SYSCALL, child, NULL, NULL);

			int aux = ptrace(PTRACE_PEEKUSER, child, 4*ORIG_RAX, NULL);
			printf("%d\n", aux);
			if (aux == 374){
				ptrace(PTRACE_KILL, child, NULL, NULL);
				write(1,"Se ha hecho justicia!\n", 40);
				return 0;
			}
			//write(1,"Se ha hecho justicia!\n", 40);
		}
	}
	return 0;
}
