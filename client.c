#include "header.h"

PEDIDO p;

char fifo_user[20], cmd[80];


int main(int argc, char **argv[])
{

	int fd,resp_login;
	
	//Caso não exista Servidor a correr, o cliente fecha
	if (access(MEDIT_MAIN_FIFO, F_OK) != 0) {
        printf("[ERRO] O servidor nao esta a correr!\n");
        return EXIT_FAILURE;
    }

    p.user.pid = getpid();
    p.user.login = 0;


    //int sprintf(char *str, const char *format, ...)
    //sprintf(str, "Value of Pi = %f", M_PI);
    //copia para o fifo_user a USER_PID
    //USER_PID -> Identificação do FIFO de cada utilizador
    sprintf(fifo_user, USER_FIFO, p.user.pid);

    //Criar FIFO do utilizador para comunicação SERVIDOR->CLIENTE
    mkfifo(fifo_user, 0600);

    do{
    	printf("\n>>> ");
    	fgets(p.cmd,80,stdin);
		p.cmd[strlen(p.cmd)-1] = '\0'; 	//remove o /n 

		//printf("%s\n", p.cmd);

		login(p.cmd);

		//printf("%d\n", p.user.login);

		

    }while(p.user.login == 0);


    if(p.user.login == 1)
    	printf("login successful!  [%s]\n",p.user.name );

    shutdownClient();

}



void login (char * name){

	int fd, res,aux;
	char resp_login[4];

	fd = open("MAIN-FIFO", O_WRONLY);
	res = write(fd, &p, sizeof(p));
	close(fd);

	fd = open(fifo_user, O_RDONLY);
	aux = read(fd, &resp_login, sizeof(resp_login));

   	//printf("%s\n",resp_login);
   	/***SPLIT resp_login [v-p] v-> válido?; p->pipe a utilizar***/
   	int var,i = 0;
   	int input[2];

	//char str[] = "1-1";
	char * token;
	token = strtok (resp_login,"-");
	while (token != NULL)
	{
	  	sscanf (token, "%d", &var);
    	input[i++] = var;

    	token = strtok (NULL, "-");
	}

	//printf("VALIDO -> [%d]\n",input[0]);
	//printf("PIPE[%d]\n",input[1]);
   	/*************************************************************/

	if(aux == sizeof(resp_login)){
		switch(input[0]){
			case 0:
				printf("Utilizador não encontrado!\n");
				break;
			case 1:
				//printf("Utilizador encontrado na base de dados!\n");
				p.user.login = 1;
				p.user.pipe = input[1];
				printf("Vou utilizar o PIPE[%d]\n",p.user.pipe);
				strcpy(p.user.name,name);
				//p.user.name = name;
				break;
		}
	}
	close(fd);
}

void shutdownClient(){


	unlink(fifo_user);
}