#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/mftplib.h"
#include "main.h"
void print_help();
int main(int argc, char **argv){
	if (argc == 1){
		print_help();
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < argc; i++){
		if ((strcmp(argv[i],"-s") == 0) || (strcmp(argv[i],"--server") == 0))
			return server_main(argc,argv);
		if ((strcmp(argv[i],"-h") == 0) || (strcmp(argv[i],"--help") == 0)){
			print_help();
			exit(EXIT_SUCCESS);
		}
	}
	return client_main(argc,argv);
}
void print_help(){
	printf(
		"usage:\n"
		"	-s [--server] : Start in server mode\n"
		"	-h [--help] : show help text \n"
	);
}
