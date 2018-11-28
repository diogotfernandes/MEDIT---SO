#include "header.h"
#define CH '*'

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


    /*******************NCURSES******************************/


    int nrow = 16, ncol = 55, posx = 3 , posy = 1;
	int ch;

	int i, j, height, width, starty, startx, statsW, statsH, statsY, statsX;
	height = 17; 	//linhas
	width = 6;		//colunas
	starty = startx = 2;
	//widthNames = 15; //colunas
	statsH = 5;
	statsW = 76;
	statsY = 18;
	statsX = 2;

	initscr();
	clear();
	noecho();
	cbreak();

	WINDOW *numbers = newwin(height, width, 		starty, startx);
	WINDOW *names 	= newwin(height, width + 9, 	starty, startx + 6);
	WINDOW *text 	= newwin(height, width + 50,	starty, startx + 20); //altura, largura, starty, startx;
	WINDOW *stats 	= newwin(statsH, statsW, statsY, statsX);

	//box(numbers, 0, 0);
	wborder(numbers, '|', ' ', '-', '-', '+', '+', '+', '-');
	//box(names,0,0);
	wborder(names, ' ', '|', '-', '-', '-', '+', '-', '+');
	//box(text, 0,0);
	wborder(text, '|', '|', '-', '-', '+', '+', '+', '+');
	//box(stats,0,0);	
	wborder(stats, '|', '|', '-', '-', '+', '+', '+', '+');


	for(i = 1; i < 16; i++){
		mvwprintw(numbers, i,1," [%d]", i);
	}
	for(i = 1; i < 16; i++){
		mvwprintw(names,i,1,"-> some guy");
	}

	mvwprintw(stats,1,2,"STATS");
	mvwprintw(stats,2,2,"User -> %s", p.user.name);
	mvwprintw(stats,3,2,"Pipe -> PIPE_%d",p.user.pipe);

	for(i = 1; i < 16; i++){
		mvwprintw(text,i,3,"gtyh gtyh gtyh gtyh gtyh gtyh gtyh gtyh ");
	}


	wrefresh(names);
	wrefresh(numbers);
	wrefresh(text);
	wrefresh(stats);

	keypad(text, TRUE);
	wmove(text, posy, posx);

	do{
		ch = wgetch(text);
		switch(ch){
			case KEY_UP:
				posy = (posy>1)?posy-1:posy;
				break;
			case KEY_DOWN:
				posy = (posy<(nrow-1))?posy+1:posy;
				break;
			case KEY_LEFT:
				posx = (posx>1)?posx-1:posx;
				break;
			case KEY_RIGHT:
				posx = (posx<(ncol-1))?posx+1:posx;
				break;
			case 10:
				mvwprintw(stats,1, 1, "(%d,%d) ", posy, posx);
				wrefresh(stats);
				break;
		}
		
		wmove(text, posy, posx);

	}while(1);

	endwin();

    /***********************************************************/


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