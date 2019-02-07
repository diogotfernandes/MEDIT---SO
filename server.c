#include "header.h"

PEDIDO p;	//ESTRUTURA PEDIDO
PIPES pipes[15];	//ESTRUTURA PIPES
pnoUsers usersList;		//Vai carregar os nomes na Base de Dados (medit.db ou outra...) para uma lista
settings defs;			//Estrutura para armazenar as definições do MEDIT
char comando[2][25];

pthread_t thread_login, thread_controlaMedit, thread_editores[MEDIT_MAXUSERS];	//id da thread

char fifoComunicacao[25] = FIFO_COMUNICACAO;

int main(int argc, char** argv){

	system("clear");

	//Caso não exista Servidor a correr, o cliente fecha
	if (access(MEDIT_MAIN_FIFO, F_OK) == 0) {
        printf("[ERRO] Já existe um Servidor ativo!!\n");
        return EXIT_FAILURE;
    }

	int i, j, opt, fflag = 0, nflag = 0, pflag = 0;
	defaultSettings(&defs);

	/* Inicia O tratamento de sinaiss*/
    struct sigaction sinal;
    sigemptyset(&sinal.sa_mask);
    sinal.sa_sigaction = trata_sinais;
    sinal.sa_flags = SA_SIGINFO; /* Important. */
    union sigval SignalInt;

    sigaction(SIGUSR1, &sinal, NULL);
	signal(SIGINT, sair);

	opterr = 0; //Desactiva Mensagem de erro do getopt()...

	pthread_create(&thread_login, NULL, &usersLoginT, NULL);	//THREAD QUE VAI VALIDAR O LOGIN DOS UTILIZADORES

	if(argc != 1){		//Argumentos introduzidos...
		while((opt = getopt(argc, argv, "f:n:p:")) != -1 ){
			switch(opt){
				case 'f':
					if(fflag != 0){		// Se a flag estiver a 1, é porque já foi utilizado o -f, então dá erro...
						printf("\n[ERRO]O comando -f só pode ser utilizado 1 vez!\n\n");
						return 1;
					}
					fflag++; 	//A flag passa a 1 quando é utilizado o -f.
					defs.db = optarg;
					break;
				case 'p':
					if(pflag != 0){		// Se a flag estiver a 1, é porque já foi utilizado o -f, então dá erro...
						printf("\n[ERRO]O comando -p só pode ser utilizado 1 vez!\n\n");
						return 1;
					}
					pflag++; 	//A flag passa a 1 quando é utilizado o -f.
					defs.fifoServer = optarg;
					break;	
				case 'n':
					if(nflag != 0){		// Se a flag estiver a 1, é porque já foi utilizado o -f, então dá erro...
						printf("\n[ERRO]O comando -n só pode ser utilizado 1 vez!\n\n");
						return 1;
					}
					nflag++; 	//A flag passa a 1 quando é utilizado o -f.
					defs.nPipes = atoi(optarg);
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

	int counter = 0;

	for(i = 0; i < defs.maxUsers; i++){				//INICIALIZAÇÃO DA ESTUTURA DE ARRAYS A ZERO
		for(j = 0; j < defs.nPipes; j++){
			pipes[i].pipe = counter;
			pipes[i].pid = 0;
			strcpy(pipes[i].name, "");
			pipes[i].login = 0;
			counter++;
			if(counter == defs.nPipes)
				counter = 0;
			break;
		}
				
	}
	
	usersList = lerUsers(defs.db); //Vai carregar os nomes na Base de Dados (medit.db ou outra...) para uma lista
	mkfifo(defs.fifoServer,0600);	//CRIA FIFO PRINCIPAL, PARA COMUNICAÇÃO ENTRE CLIENTE->SERVIDOR; SERVER APENAS PARA A PRIMEIRA COMUNICAÇÃO

    //int sprintf(char *str, const char *format, ...)
    //sprintf(str, "Value of Pi = %f", M_PI);
    //copia para o fifo_user a USER_PID
    //USER_PID -> Identificação do FIFO de cada utilizador


	

	for (i = 0; i < defs.nPipes; i++){
		sprintf(fifoComunicacao,FIFO_COMUNICACAO,i);
		mkfifo(fifoComunicacao,0600);
	}

	char cmd[80];

	//pthread_create(&thread_controlaMedit, NULL, &controlaMedit, NULL);

	do{

		printf("\n>>> ");
		fgets(cmd,80,stdin);
		separaComando(cmd);
		//printf("[%s]-[%s]", comando_separado[0],comando_separado[1]);
		cmd[strlen(cmd)-1] = '\0'; 	//remove o /n 

		

		if(!strcmp(cmd,"settings")){
			printf("Mostra Definições actuais do servidor...\n\n");
			showSettings();
		}else if(!strcmp(comando[0],"load")){
			printf("[CMD -> %s] - [ARGS -> %s]\n", comando[0], comando[1]);
		}
		else if(!strcmp(comando[0],"save")){
			printf("[CMD -> %s] - [ARGS -> %s]\n", comando[0], comando[1]);
		}
		else if(!strcmp(cmd,"free")){
			printf("[CMD -> %s] - [ARGS -> %s]\n", comando[0], comando[1]);
		}
		else if(!strcmp(cmd,"statistics")){
			printf("CMD>>> statistics\n");
		}
		else if(!strcmp(cmd,"users")){
			for(i = 0; i < defs.maxUsers; i++){
				if(pipes[i].pid != 0){
					printf("[Nome] -> %s\n",pipes[i].name);
					printf("[PID] -> %d\n",pipes[i].pid);
					printf("[Pipe] -> %d\n\n",pipes[i].pipe);
				}
			}
		}
		else if(!strcmp(cmd,"text")){
			printf("CMD>>> text\n");
		}
		else if(!strcmp(cmd,"valid")){
			char w[50];
			printf("Palavra-> ");
			scanf(" %s",w);
			printf("%s\n", w);
			spellCheck(&w);
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
	printf("N Pipes     ->	%d\n", defs.nPipes);
	printf("---------------SETTINGS---------------\n");
}

//VAI CARREGAR AS VARIAVEIS DE AMBIENTE PARA A ESTRUTURA SETTINGS
settings defaultSettings(settings *s){

	s-> db = MEDIT_DB;
	s-> fifoServer = MEDIT_MAIN_FIFO;
	s-> nPipes = N_PIPES;
	s-> maxLines = MEDIT_MAXLINES;
	s-> maxColumns = MEDIT_MAXCOLUMNS;
	s-> maxUsers = MEDIT_MAXUSERS;
	s-> timeout = MEDIT_TIMEOUT;

//VERIFICAR SE VALORES ESTÃO DENTRO DOS PARAMETROS

	if(getenv("MEDIT_MAXLINES") != NULL)
		s->maxLines = atoi(getenv("MEDIT_MAXLINES"));
	if(getenv("MEDIT_MAXCOLUMNS") != NULL)
		s->maxColumns = atoi(getenv("MEDIT_MAXCOLUMNS"));
	if(getenv("MEDIT_TIMEOUT") != NULL)
		s->timeout = atoi(getenv("MEDIT_TIMEOUT"));
	if(getenv("MEDIT_MAXUSERS") != NULL)
		s->maxUsers = atoi(getenv("MEDIT_MAXUSERS"));
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

    while (lista) {
        if (strcmp(user, lista->username) == 0) {
        	//printf("valido\n");
        	//printf("Existe na BD! [name-> %s]\n", lista->username);
            return 1;	//Válido. Utilizador está na Base de Dados
        }
        lista = lista->next;
    }
    //
    //printf("Não existe! [name-> %s]\n", user);
    //printf("nao valido\n");  
    return 0;	//Não existe 
}

void *usersLoginT(){

	int fd, aux, ans, fd_ans, i, j, flag, pidServ;
	char fifo_user[20];
	char str[20];

	while(1){
		fd = open(defs.fifoServer, O_RDONLY);
		aux = read(fd, &p, sizeof(p));

		if(aux == sizeof(p)){
			sprintf(fifo_user, USER_FIFO ,p.user.pid); //fifo_user = USER_%d + pid;		 (USER_FIFO = USER_%d)


			ans = userValidation(usersList,p.cmd);

			//printf("\nVALIDATION -> [%d]\n",ans );
			//printf("NAME -> [%s]\n",p.cmd );

			pidServ = getpid();

			if(ans){
				for(i = 0; i < defs.maxUsers; i++){
					if(!strcmp(p.cmd,pipes[i].name)){
						flag = 99;
						ans = 2;
						sprintf(str, "%d-%d-%d",ans,flag,pidServ);
						break;
					}else if(pipes[i].pid == 0){
						printf("[LOGIN] -> [%s]\n",p.cmd );
						pipes[i].pid = p.user.pid;
						strcpy(pipes[i].name, p.cmd);
						pipes[i].login = 1;
						sprintf(str, "%d-%d-%d",ans,pipes[i].pipe,pidServ);
						break;
					}
				}
			}else{
				flag = 99;
				sprintf(str, "%d-%d-%d",ans,flag,pidServ);
			}

			//printf("%s\n", str);

			fd_ans = open(fifo_user, O_WRONLY);
			write(fd_ans, &str, sizeof(str));	//CASO
			close(fd_ans);

				
		}
		close(fd);
	}
	pthread_exit(0);
}

void sair(int num){
 	return;
 }

 void  desligaCliente(int num){
 	printf("lalala\n");

 	return;
 }


 void shutdownServer(){

	union sigval sinal;
    sinal.sival_int = -1;
    int i,j;

    for(i = 0; i < defs.maxUsers; i++){
    	if(pipes[i].pid != 0){
    		sigqueue(pipes[i].pid, SIGUSR1, sinal);
    	}
    }
	unlink(defs.fifoServer);
	for (i = 0; i < defs.nPipes; i++){
		sprintf(fifoComunicacao,FIFO_COMUNICACAO,i);
		unlink(fifoComunicacao);
	}
}

void spellCheck (char * word){

	printf("%s\n", word);

	int pipen[2];
    pipe(pipen);  //criação do pipe

    int pipe2[2];
    pipe(pipe2);

    pid_t child_pid;
    char ans[100];

    child_pid = fork();

    if(child_pid < 0 ){
    	    perror("fork failure");
    		exit(EXIT_FAILURE);
        }

    if(child_pid == 0){
    	close(pipen[1]);
    	close(pipe2[0]);
        dup2(pipen[0],0);
        
        dup2(pipe2[1],1);
        //printf("filho\n");
        close(pipen[0]);
        close(pipe2[1]);
        execl("/usr/bin/aspell","aspell","pipe",NULL);
    }else{
		close(pipen[0]);
	  	close(pipe2[1]);
	  	write(pipen[1],word,sizeof(word));
	  	close(pipen[1]);
	  	read(pipe2[0],&ans,sizeof(ans));
	  	read(pipe2[0],&ans,sizeof(ans));


	  	close(pipe2[1]);
  		if(ans[0] == '*'){
  			printf("\n\nvalid\n\n");
  			printf("%s\n", ans);
  		}
    		

  		else{
  			printf("%s\n", ans);
    		printf("Not valid\n");
  		}
  			
  	//printf("\n[%s]\n",ans);

  }
}

void separaComando(char str[])
{
    char *token;
    const char s[] = " ";
    int i = 0;
    token = strtok(str, s);
    for (i = 0; i < 2; i++)
    {
        if (token == NULL)
        {
            strcpy(comando[i], "");
        }
        else
        {
            strcpy(comando[i], token);
        }
        token = strtok(NULL, s);
    }
}


/*
void *controlaMedit(){


for(i = 0; i < MEDIT_MAXUSERS; i++){
    pthread_create(&thread_editores[i], NULL, &editar, (void *) &jogador[i]);
  }
	//thread p/ editores
        for(i = 0; i < defs.maxUsers; i++)
        {
            if(Dadosjogo.jogadores[i].pid != 0)
            {
                pthread_create(&thread_jogadores[i], NULL, &ControlaJogador, (void *) &medit.editores[i]);

            }
        }
}
*/

void trata_sinais(int sinal, siginfo_t *valor, void *n)
{
//função de tratamento de sinais.
    union sigval valor_sinal_trata_sinal;
    int i;
    switch (sinal)
    {
    case SIGUSR1:
        for(i = 0 ; i< MEDIT_MAXUSERS; i++)
            {
                if(valor->si_pid == pipes[i].pid)
                {
                    printf("\n Saiu o cliente [%s]\n", pipes[i].name);
                    pipes[i].pid = 0;
                    strcpy(pipes[i].name, "");
                    break;
                }
            }

        break;
    }
}