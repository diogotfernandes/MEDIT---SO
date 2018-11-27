//INCLUDES
#include <stdio.h> 		//standard buffered input/output
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define MEDIT_MAIN_FIFO "main-fifo"
#define USER_FIFO "USER_%d"


typedef struct USER{
	char name[8];
	int login;
	int pid;
}USER;

typedef struct {
	char cmd[50];
	USER user;
}PEDIDO;


void login (char * name);