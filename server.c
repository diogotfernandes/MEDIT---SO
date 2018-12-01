#include "header.h"


PEDIDO p;

PIPES pipes;

pnoUsers usersList;		//Vai carregar os nomes na Base de Dados (medit.db ou outra...) para uma lista
settings defs; 			//Estrutura para armazenar as definições do MEDIT
pthread_t thread_users;	//

int main(int argc, char** argv){

	

	system("clear");

	int i, j;
	int opt, fflag = 0, hflag = 0, lflag = 0, cflag = 0, tflag = 0;
	defaultSettings(&defs);

	int aux;

	//aux = defs.maxUsers/nPipes;

	for(i = 0; i < 8; i++){				//INICIALIZAÇÃO DO ARRAY A ZERO
		for(j = 0; j < N_PIPES; j++)
			pipes.arr[i][j] = 0;
	}

	/*for(i = 0; i < 8; i++){
					for(j = 0; j < N_PIPES; j++){
						//if(pipes.arr[i][j] != 0)
							printf("[%d][%d] -> %d\n",i,j,pipes.arr[i][j]);
					}
				}*/

	/*
	__Pipe0___Pipe1__
	|		|		|
	|_______|_______|
	|		|		|
	|_______|_______|
	|		|		|
	|_______|_______|
	|		|		|
	|_______|_______|
	|		|		|
	|_______|_______|
	|		|		|
	|_______|_______|
	|		|		|
	|_______|_______|
	|		|		|
	|_______|_______|
	*/

	//pipes.arr[0][0] = 123;
	printf("%d\n",pipes.arr[0][0]);

	opterr = 0; //Desactiva Mensagem de erro do getopt()...

	mkfifo(defs.fifoServer,0600);	//CRIA FIFO PRINCIPAL, PARA COMUNICAÇÃO ENTRE CLIENTE->SERVIDOR; SERVER APENAS PARA A PRIMEIRA COMUNICAÇÃO

	pthread_create(&thread_users, NULL, &usersLoginT, NULL);	//THREAD QUE VAI VALIDAR O LOGIN DOS UTILIZADORES


	if(argc != 1){		//Argumentos introduzidos...
		while((opt = getopt(argc, argv, "f:l:c:t:")) != -1 ){
			switch(opt){
				case 'f':
					if(fflag != 0){		// Se a flag estiver a 1, é porque já foi utilizado o -f, então dá erro...
						printf("\n[ERRO]O comando -f só pode ser utilizado 1 vez!\n\n");
						return 1;
					}
					fflag++; 	//A flag passa a 1 quando é utilizado o -f.
					defs.db = optarg;
					break;
				case 'l':
					if(lflag != 0){
						printf("\n[ERRO]O comando -l só pode ser utilizado 1 vez!\n\n");
						return 1;
					}
					lflag++;
					defs.maxLines = atoi(optarg);
					break;
				case 'c':
					if(cflag != 0){
						printf("\n[ERRO]O comando -c só pode ser utilizado 1 vez!\n\n");
						return 1;
					}
					cflag++;
					defs.maxColumns = atoi(optarg);
					break;
				case 't':
					if(tflag != 0){
						printf("\n[ERRO]O comando -t só pode ser utilizado 1 vez!\n\n");
						return 1;
					}
					tflag++;
					defs.timeout = atoi(optarg);
					break;
				case '?':
					if (isprint(optopt)){
						fprintf(stderr, "\n[ERRO] Unknown option \"-%c\"\n\n", optopt);
					}else{
						fprintf(stderr, "\n[ERRO] Unknown option character %x\n\n", optopt);
					}
					return 1;
			}
		}
	}
	
		usersList = lerUsers(defs.db); //Vai carregar os nomes na Base de Dados (medit.db ou outra...) para uma lista
		


	char cmd[80];

	do{

		printf("\n>>> ");
		fgets(cmd,80,stdin);
		cmd[strlen(cmd)-1] = '\0'; 	//remove o /n 

		//printf("%s\n", cmd);

		if(!strcmp(cmd,"settings")){
			printf("Mostra Definições actuais do servidor...\n\n");
			showSettings();
		}else if(!strcmp(cmd,"test")){
			userValidation(usersList,"diogo");
		}
		else if(!strcmp(cmd,"load")){
			for(i = 0; i < 8; i++){
					for(j = 0; j < N_PIPES; j++){
						//if(pipes.arr[i][j] != 0)
							printf("[%d][%d] -> %d\n",i,j,pipes.arr[i][j]);
					}
				}
			printf("CMD>>> load...\n");
		}
		else if(!strcmp(cmd,"save")){
			printf("CMD>>> save\n");
		}
		else if(!strcmp(cmd,"free")){
			printf("CMD>>> free\n");
		}
		else if(!strcmp(cmd,"statistics")){
			printf("CMD>>> statistics\n");
		}
		else if(!strcmp(cmd,"users")){
			printf("CMD>>> users\n");
		}
		else if(!strcmp(cmd,"text")){
			printf("CMD>>> text\n");
		}

	}while(strcmp(cmd,"shutdown") != 0);

	printf("\nVAI DESLIGAR...\n\n");
	shutdownServer();
}

void showSettings(){

	printf("---------------SETTINGS---------------\n");
	printf("Database    ->	%s\n", defs.db);
	printf("Lines       ->	%d\n", defs.maxLines);
	printf("Columns     ->	%d\n", defs.maxColumns);
	printf("Main Pipe   ->	%s\n", defs.fifoServer);
	printf("Max Users   ->	%d\n", defs.maxUsers);
	printf("Timeout     ->	%d\n", defs.timeout);
	printf("---------------SETTINGS---------------\n");
}

//VAI CARREGAR AS VARIAVEIS DE AMBIENTE PARA A ESTRUTURA SETTINGS
settings defaultSettings(settings *s){
	

	if(getenv("MEDIT_DB") == NULL || getenv("MEDIT_MAIN_FIFO") == NULL || 
		getenv("MEDIT_MAXLINES") == NULL || ("MEDIT_MAXCOLUMNS") == NULL ||  
		getenv("MEDIT_TIMEOUT") == NULL || getenv("MEDIT_MAXUSERS") == NULL)
	{
		printf("\n[ERRO] NO ENVIRONMENT VARIABLES LOADED!!!!\n\n");
		exit(EXIT_FAILURE);
	}else{

		s->db = getenv("MEDIT_DB");
		s->fifoServer = getenv("MEDIT_MAIN_FIFO");
		s->maxLines = atoi(getenv("MEDIT_MAXLINES"));
		s->maxColumns = atoi(getenv("MEDIT_MAXCOLUMNS"));
		s->timeout = atoi(getenv("MEDIT_TIMEOUT"));
		s->maxUsers = atoi(getenv("MEDIT_MAXUSERS"));
	}

	printf("\nDEFAULTS SETTINGS LOADED...\n");

}

//FUNÇÃO PARA LER UTILIZADORES DA BASE DE DADOS PARA UMA LISTA
pnoUsers lerUsers(char *ficheiro){
    pnoUsers novo, anterior = NULL, lista = NULL;
    USERS userTemp;
    FILE *fr = fopen(ficheiro, "rt");

    if (!fr){
        printf("Erro a ler o ficheiro!\n");
        return NULL;
    }

    userTemp.next = NULL;

    while (fscanf(fr, "%s", userTemp.username) == 1){

        novo = (pnoUsers) malloc(sizeof (USERS));
        if (!novo){
            printf("Erro a reservar memória!\n");
            break;
        }

        *novo = userTemp;

        if (lista == NULL)
            lista = novo;
        else
            anterior->next = novo;

        anterior = novo;
    }
    fclose(fr);
    return lista;
    
}

//FUNÇÃO PARA PERCORRER A LISTA, À PROCURA DO UTILIZADOR INTRODUZIDO
int userValidation(pnoUsers lista, char *user) {

	int aux;

    while (lista) {
        if (strcmp(user, lista->username) == 0) {
        	printf("valido\n");
        	//printf("Existe na BD! [name-> %s]\n", lista->username);
            return 1;	//Válido. Utilizador está na Base de Dados
        }
        lista = lista->next;
    }
    //printf("Não existe! [name-> %s]\n", user);
    printf("nao valido\n");
    return 0;	//Não existe na Base de Dados
}

void *usersLoginT(){

	int fd, aux, ans, fd_ans, i, j, flag;
	char fifo_user[20];
	char str[12];

	while(1){
		fd = open(defs.fifoServer, O_RDONLY);
		aux = read(fd, &p, sizeof(p));

		if(aux == sizeof(p)){
			sprintf(fifo_user, USER_FIFO ,p.user.pid); //fifo_user = USER_%d + pid;		 (USER_FIFO = USER_%d)

			ans = userValidation(usersList,p.cmd);

			printf("VALIDATION -> [%d]\n",ans );
			
			printf("VALIDATION -> [%s]\n",p.cmd );

			//sprintf(Base, "%s %d", Base, Number);


			if(ans){
				for(i = 0; i < 8; i++){
					for(j = 0; j < N_PIPES; j++){
						if(pipes.arr[i][j] == 0){
							pipes.arr[i][j] = p.user.pid;
							sprintf(str, "%d-%d",ans,j);
							i = j = 1000;	//STOP... SAI DO CICLO;
							}
						}
					}


					/*for(i = 0; i < 8; i++){
					for(j = 0; j < N_PIPES; j++){
						if(pipes.arr[i][j] != 0){
							printf("USER_%d -> PIPE[%d]\n",pipes.arr[i][j],j);

							}
						}
					}*/
			}else{
				flag = 99;
				sprintf(str, "%d-%d",ans,flag);
			}

			printf("\nSTR->%s\n", str);

			fd_ans = open(fifo_user, O_WRONLY);
			write(fd_ans, &str, sizeof(ans));	//CASO
			close(fd_ans);

		}
		close(fd);
	}
	pthread_exit(0);
}

void shutdownServer(){

	unlink(defs.fifoServer);

}
