#include "header.h"

PEDIDO p;

char cmd[80] , main_fifo[20] = MEDIT_MAIN_FIFO;
char comando[3][25];

int pidServer;

int main(int argc, char** argv)
{
	system("clear");

	int fd;

	opterr = 0; //Desactiva Mensagem de erro do getopt()...

    p.user.pid = getpid(); 	//GET PID do utilizador
    p.user.login = 0;		//utilizador não está logado ainda

	signal(SIGUSR1, desliga); 
    
    //int sprintf(char *str, const char *format, ...)
    //sprintf(str, "Value of Pi = %f", M_PI);
    //copia para o fifo_user o USER_PID
    //USER_PID -> Identificação do FIFO de cada utilizador
    sprintf(p.fifo_user, USER_FIFO, p.user.pid);

    //Criar FIFO do utilizador para comunicação SERVIDOR->CLIENTE
    mkfifo(p.fifo_user, 0600);

    int uflag = 0, pflag = 0, opt;

    if(argc != 1){  //user introduzido com [-u user]
    	while((opt = getopt(argc, argv, "u:p:")) != -1 ){
    		switch(opt){
    			case 'p':
					if(pflag != 0){		// Se a flag estiver a 1, é porque já foi utilizado o -f, então dá erro...
						printf("\n[ERRO]O comando -p só pode ser utilizado 1 vez!\n\n");
						return 1;
					}
					pflag++; 	//A flag passa a 1 quando é utilizado o -f.
					strcpy(main_fifo,optarg);
					break;
    			case 'u':
    				if(uflag != 0){
						printf("\n[ERRO]O comando -u só pode ser utilizado 1 vez!\n\n");
						return 1;
					}
						uflag++;
						strcpy(p.cmd,optarg);
						login(p.cmd);
						if(p.user.login == 0){
							shutdownClient();
							return EXIT_FAILURE;
						}
						break;
				case '?':
					if (isprint(optopt))
					{
						fprintf(stderr, "\n[ERRO] Unknown option \"-%c\"\n\n", optopt);
					}else{
						fprintf(stderr, "\n[ERRO] Unknown option character %x\n\n", optopt);
					}
					return 1;
    					
    		}
    	}
    }

    //Caso não exista Servidor a correr, o cliente fecha
	if (access(main_fifo, F_OK) != 0) {
        printf("[ERRO] O servidor nao esta a correr!\n");
        return EXIT_FAILURE;
    }

    if(p.user.login == 0){
    	do{
    	printf("\nUsename >>> ");
    	scanf("%8s",p.cmd);
		login(p.cmd);
    	}while(p.user.login == 0);
    }

    /*******************NCURSES******************************/
    int nrow = 16, ncol = 46, posx = 1 , posy = 1, highlight = 1, ch;

	int i, j, height, width, starty, startx, statsW, statsH, statsY, statsX;
	height = 17; 	//linhas
	width = 6;		//colunas
	starty = startx = 2;
	statsH = 5;
	statsW = 66;
	statsY = 18;
	statsX = 2;

	initscr();
	clear();
	noecho();
	cbreak();
	curs_set(0);

	WINDOW *numbers = newwin(height, width, 		starty, startx);		//altura, largura, starty, startx;
	WINDOW *names 	= newwin(height, width + 9, 	starty, startx + 6);	//altura, largura, starty, startx;
	WINDOW *text 	= newwin(height, width + 45,	starty, startx + 20); 	//altura, largura, starty, startx;
	WINDOW *info 	= newwin(statsH, statsW, statsY, statsX);				//altura, largura, starty, startx;

	wborder(numbers, '|', ' ', '-', '-', '+', '+', '+', '-');
	wborder(names, ' ', '|', '-', '-', '-', '+', '-', '+');
	wborder(text, '|', '|', '-', '-', '+', '+', '+', '+');
	wborder(info, '|', '|', '-', '-', '+', '+', '+', '+');


	for(i = 1; i < 16; i++){
		if(i<=9){
			mvwprintw(numbers, i,1," [0%d]", i);
		}
		if(i>=10){
			mvwprintw(numbers, i,1," [%d]", i);
		}
	}

	mvwprintw(info,1,2,"User -> %s", p.user.name);
	mvwprintw(info,2,2,"Pipe -> PIPE_%d",p.user.pipe);


	wrefresh(names);
	wrefresh(numbers);
	wrefresh(text);
	wrefresh(info);

	keypad(text, TRUE);
	wmove(text, posy, posx);

	int stop = 1, edit;

	char editText[16][46];

	for(i = 0; i < 15; i++){
          strcpy(editText[i], "INICIO some some some tre some some Some FIM "); 
      }

    strcpy(editText[5], "                                             ");
    strcpy(editText[10], "                                             ");
    strcpy(editText[1], "       d             f                       ");
    strcpy(editText[13], "                                             ");
	

	do{
		for(i = 1; i < 16; i++){
			if(i == highlight)
				wattron(text, A_REVERSE);
			mvwprintw(text,i,1,editText[i-1]);
			wattroff(text,A_REVERSE);
		}

		ch = wgetch(text);
		switch(ch){
			case KEY_UP:
				highlight--;
				if(highlight == 0)
					highlight = 1;
				posy = (posy>1)?posy-1:posy;
				break;
			case KEY_DOWN:
				highlight++;
				if(highlight == 16)
					highlight = 15;
				posy = (posy<(nrow-1))?posy+1:posy;
				break;
			case 113:
					mvwprintw(info,1, 30, "(y%d,x%d) ", posy, posx);
					wrefresh(info);
					break;
			case 10:
				mvwprintw(names,posy,1,"-> %s",p.user.name);
				wrefresh(names);
				mvwprintw(info,3, 2, "EDIT MODE!");
				edit = 1;
				curs_set(1);
				do{
					int chEdit = wgetch(text);

					switch(chEdit){

					case KEY_LEFT:
						posx = (posx>1)?posx-1:posx;
						break;

					case KEY_RIGHT:
						posx = (posx<(ncol-1))?posx+1:posx;
						break;

					case 330: //DELETE
						highlight = posy;
						wattron(text, A_REVERSE);

						editText[posy-1][posx-1] = ' ';

						for(i=posx-1; i<46; i++){
					        /* Move each array element to its left */
					        editText[posy-1][i] = editText[posy-1][i + 1];
					        //editText[posy-1][last] = ' ';
					    }
						mvwprintw(text,posy,1,editText[posy-1]);
						
					break;	

					case KEY_BACKSPACE:
						highlight = posy;
						wattron(text, A_REVERSE);
						editText[posy-1][posx-1] = ' ';

						int i;
					    
					    for(i=posx-1; i<46; i++)
					    {
					        /* Move each array element to its left */
					        editText[posy-1][i] = editText[posy-1][i + 1];
					        //editText[posy-1][last] = ' ';
					    }
						mvwprintw(text,posy,1,editText[posy-1]);
						posx = (posx>1)?posx-1:posx;
						wrefresh(text);
						break;

					case 10:
						mvwprintw(info,3, 2, "          ");
						mvwprintw(names,posy,1,"           ");
						wrefresh(names);
						wrefresh(info);
						wattroff(text,A_REVERSE);
						wrefresh(text);
						curs_set(0);
						edit = 0;
						break;
					case KEY_UP:
						break;

					case KEY_DOWN:
						break;

					default:
						if(editText[posy-1][posx-1] == ' '){
							editText[posy-1][posx-1] = chEdit;
						}else{
							//
						}
						mvwprintw(text,posy,1,editText[posy-1]);
						posx = (posx<(ncol-1))?posx+1:posx;
						wrefresh(text);

					break;
					}
					wmove(text, posy, posx);
					mvwprintw(info,1, 30, "(Y%d,X%d) ", posy, posx);
					wrefresh(info);
					wrefresh(text);

				}while(edit);
				break;

			case 103:
				stop = 0;
				break;
		}
		
		wmove(text, posy, posx);


	}while(stop);

	endwin();

    /***********************************************************/


    shutdownClient();

}



void login (char * name){

	int fd, res,aux;
	char resp_login[20];

	fd = open(main_fifo, O_WRONLY);
	res = write(fd, &p, sizeof(p));
	close(fd);

	fd = open(p.fifo_user, O_RDONLY);
	aux = read(fd, &resp_login, sizeof(resp_login));

   	//printf("%s\n",resp_login);
   	/***SPLIT resp_login [v-p] v-> válido?; p->pipe a utilizar***/
   	int k = 0;

	char *token;
    const char s[] = "-";
    token = strtok(resp_login, s);
    for (k = 0; k < 4; k++)
    {
        if (token == NULL)
        {
            strcpy(comando[k], "");
        }
        else
        {
            strcpy(comando[k], token);
        }
        token = strtok(NULL, s);
    }

   	int valLogin = atoi(comando[0]);
   	int pipe = atoi(comando[1]);
   	int pidServer = atoi(comando[2]);


	if(aux == sizeof(resp_login)){
		switch(valLogin){
			case 0:
				printf("Utilizador não encontrado!\n");
				break;
			case 1:
				//printf("Utilizador encontrado na base de dados!\n");
				p.user.login = 1;
				p.user.pipe = pipe;
				p.user.pidS = pidServer;
				strcpy(p.user.name,name);
				//printf("%d\n",p.user.pidS );
				break;
			case 2:
				printf("Utilizador já entrou!\n");
				break;
		}
	}
	close(fd);
}

void shutdownClient(){

	union sigval sinal;
    sinal.sival_int = -1;

    sigqueue(p.user.pidS, SIGUSR1, sinal);  // envia qual o pid desejado, o sinal enviado, e a estrutura dos sinais

	unlink(p.fifo_user);
	system("clear");
	printf("[AVISO] Saiu do Servidor!\n");
	sleep(3);
	endwin();
	exit(0);
}
 
 // 
void desliga(int s)  // o servidor foi desligado
{
	system("clear");
	echo();
	refresh();
	clear();
	if(s == SIGUSR1)
		printf("[AVISO] O Servidor foi desligado!\n");
	shutdownClient();
	sleep(3);
	endwin();
	exit(0);
}
