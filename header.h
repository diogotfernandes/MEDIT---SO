/*---------------INCLUDES---------------*/
#include <stdio.h> 		//standard buffered input/output
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ncurses.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>


/*---------------DEFINES---------------*/ //VALORES POR OMISSÃO
#define MEDIT_MAIN_FIFO "MAIN-FIFO"
#define MEDIT_MAXLINES 15 //Número máximo de linhas; 15 por omissão
#define MEDIT_MAXCOLUMNS 45 //Número máximo de colunas; 45 por omissão
#define MEDIT_TIMEOUT 10 //Tempo(em segundos) para edição de uma linha; por omissão 10 segundos
#define MEDIT_MAXUSERS 3 //Número máximo de utilizadores activos em simultâneo; por omissão 3
#define MEDIT_DB "medit.db" //Base de dados dos utilizadores; por omissão "medit.db"
#define N_PIPES 3
#define FIFO_COMUNICACAO "FIFO_%d"

/*---------------ESTRUTURAS---------------*/
typedef struct USERS {	//Lista dinâmica para guardar os usernames
	char username[9]; 	
	struct USERS *next;
}USERS, *pnoUsers;

typedef struct {			//Estrutura para guardar as definições do MEDIT (editor de texto)
	char *db;
	char *fifoServer;
	int maxLines;
	int maxColumns;
	int timeout;
	int maxUsers;
	int nPipes;
}settings;

typedef struct{
	char name[9];
	int login;
	int pipe;
	int pid;
}PIPES;



/*---------------PROTOTIPOS---------------*/
pnoUsers lerUsers(char *fich);
settings defaultSettings();
void showSettings();
int userValidation(pnoUsers lista, char *user);
// Função usada na thread para responder ao FIFO dos jogadores.
void *usersLoginT();
void shutdownServer();
void sair(int num);
void spellCheck (char * word);
void separaComando(char str[]);




/*---------------CLIENTE---------------*/

#define USER_FIFO "USER_%d"


/*---------------ESTRUTURAS---------------*/

typedef struct USER{
	char name[9];
	int login;
	int pid;
	int pipe;
	int pidS;
}USER;

typedef struct {
	char cmd[50];
	USER user;
	char fifo_user[20];
}PEDIDO;



/*---------------PROTOTIPOS---------------*/
void login (char * name);
void shutdownClient();
void logout(int s); // o processo é terminado
void desliga(int s);  // o servidor foi desligado
void trata_sinais(int sinal, siginfo_t *valor, void *n);
