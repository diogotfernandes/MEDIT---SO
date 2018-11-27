/*---------------INCLUDES---------------*/
#include <stdio.h> 		//standard buffered input/output
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*---------------DEFINES---------------*/  /* SUBSTITUIDO POR VARIÁVEIS DE AMBIENTE
#define MEDIT_FIFO_SERVER "fifoServer"
#define MEDIT_MAXLINES 15 //Número máximo de linhas; 15 por omissão
#define MEDIT_MAXCOLUMNS 45 //Número máximo de colunas; 45 por omissão
#define MEDIT_TIMEOUT 10 //Tempo(em segundos) para edição de uma linha; por omissão 10 segundos
#define MEDIT_MAXUSERS 3 //Número máximo de utilizadores activos em simultâneo; por omissão 3
#define MEDIT_DB "medit.db" //Base de dados dos utilizadores; por omissão "medit.db"
*/

/*---------------ESTRUTURAS---------------*/
typedef struct USERS {	//Lista dinâmica para guardar os usernames
	char username[8]; 	
	struct USERS *next;
}USERS, *pnoUsers;

typedef struct {			//Estrutura para guardar as definições do MEDIT (editor de texto)
	char *db;
	char *fifoServer;
	int maxLines;
	int maxColumns;
	int timeout;
	int maxUsers;
}settings;


/*---------------PROTOTIPOS---------------*/
pnoUsers lerUsers(char *fich);
settings defaultSettings();
void showSettings();
int userValidation(pnoUsers lista, char *user);
// Função usada na thread para responder ao FIFO dos jogadores.
void *usersLoginT();
void shutdownServer();




/*---------------CLIENTE---------------*/

#define MEDIT_MAIN_FIFO "MAIN-FIFO"
#define USER_FIFO "USER_%d"
#define N_PIPES 2


/*---------------ESTRUTURAS---------------*/
typedef struct USER{
	char name[8];
	int login;
	int pid;
	int pipe;
}USER;

typedef struct {
	char cmd[50];
	USER user;
}PEDIDO;

typedef struct{
	int arr[8][N_PIPES];
}PIPES;

/*---------------PROTOTIPOS---------------*/
void login (char * name);
void shutdownClient();