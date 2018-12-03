#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include "util.h"


char username[9]={0};
char linharecebida[45];
int posNL=-1;
char linhaeditada[45];

int checkuser(){
  int fd_cli,fd_ser;
  char fifo_nome[20];
  PEDIDO p;
  int n;
  p.remetente = getpid();
  p.tipo =1;
  strcpy(p.username,username);
  fd_ser = open (FIFO_SER,O_WRONLY);
  n=write(fd_ser,&p,sizeof(PEDIDO));
  printf("Foi enviado %s\t",username);

  //ler resposta do servidor
  sprintf(fifo_nome,FIFO_CLI,getpid());
  mkfifo(fifo_nome,0600);
  fd_cli=open(fifo_nome,O_RDONLY);
  n=read(fd_cli,&p,sizeof(PEDIDO));
  close(fd_cli);
  printf("Recebi valid =  %d\n\n",p.valid);
  return p.valid;

}

int lockline(int linenumber){ //1 se puder editar linha 0 se nao
  int fd_cli,fd_ser;
  char fifo_nome[20];
  PEDIDO p;
  int n;
  p.remetente = getpid();
  p.tipo =3;
  strcpy(p.username,username);
  p.linhaPoxy = linenumber;
  sprintf(fifo_nome,FIFO_CLI,getpid());
  mkfifo(fifo_nome,0600);
  fd_ser = open (FIFO_SER,O_WRONLY);
  n=write(fd_ser,&p,sizeof(PEDIDO));
  printf("Foi enviado ao serv %s\t",username);

  //ler resposta do servidor
  fd_cli=open(fifo_nome,O_RDONLY);
  n=read(fd_cli,&p,sizeof(PEDIDO));
  close(fd_cli);
  printf("Recebi valid =  %d\n\n",p.valid);
  return p.valid;

}

void unlockline(int linenumber,char *linhatxt){
  int fd_cli,fd_ser;
  char fifo_nome[20];
  PEDIDO p;
  int n;
  p.remetente = getpid();
  p.tipo =4;
  strcpy(p.linha,linhatxt);
  strcpy(p.username,username);
  p.linhaPoxy = linenumber;
  sprintf(fifo_nome,FIFO_CLI,getpid());
  mkfifo(fifo_nome,0600);
  fd_ser = open (FIFO_SER,O_WRONLY);
  n=write(fd_ser,&p,sizeof(PEDIDO));
  printf("Foi enviado ao serv %s\t",username);

}

int logout(){
  int fd_cli,fd_ser;
  char fifo_nome[20];
  PEDIDO p;
  int n;
  p.remetente = getpid();
  p.tipo =2;
  fd_ser = open (FIFO_SER,O_WRONLY);
  n=write(fd_ser,&p,sizeof(PEDIDO));
  printf("Foi enviado logout\n");
  exit(1);

}

void recebe(int s){
  char fifo_nome[20];
  if(s == SIGUSR1){
    printf("o servidor vai encerrar em 1 segundos\n" );
    sleep(1);
    endwin();
    sprintf(fifo_nome,FIFO_CLI,getpid());
    unlink(fifo_nome);
    exit(0);
  }
}

void recebe1(int s){
  char fifo_nome[20];
    endwin();
    logout();
    exit(0);
  }

void recebelinha(int s){
  int fd_cli,n;
  char fifo_nome[20];
  PEDIDO p;
  //ler resposta do servidor
  sprintf(fifo_nome,FIFO_CLI,getpid());
  mkfifo(fifo_nome,0600);
  fd_cli=open(fifo_nome,O_RDONLY);
  n=read(fd_cli,&p,sizeof(PEDIDO));
  close(fd_cli);

  int j;
  for(j=0;j<45;j++){
    if(p.linha[j] =='\n' || p.linha[j] =='\0'){
      p.linha[j] = ' ';
    }
  }
  p.linha[45] = '\0';

  strcpy(linharecebida,"                                             ");
  strcpy(linharecebida,p.linha);
  posNL = p.linhaPoxy;
  //printf("recebi a linha %s\t",p.linha);
}

void show_help(char *name) {
    fprintf(stderr, "\
            [uso] %s <opcoes>\n\
            -h         mostra essa tela e sai.\n\
            -u username \n\", name) ;",name);
    exit(-1) ;
}
int main(int argc, char **argv) {
    int opt ,check=0;
    int posy,posx,oposy,oposx,nrow,ncol,edicao=0,ch;
    char *varAmb=NULL;
    int skip = 0;
    int uFARg = 0;


  signal(SIGUSR1,recebe);
  signal(SIGUSR2,recebelinha);
  signal(SIGINT,recebe1);
  //verifica se o server está a correr
    if (access(FIFO_SER,F_OK)!=0){
            printf("O servidor não está a correr\n");
            return 3;
            }

    WINDOW * uiWindow;
    WINDOW * notificacaoWindow;
    WINDOW * shortcutsWindow;

    char *nrows = getenv("MEDIT_MAXLINES");
    char *ncols = getenv("MEDIT_MAXCOLUMNS");
    if(nrows == NULL || ncols == NULL){
      puts("variaveis de ambiente nao definidas\n executar . ./script.sh");
      exit(3);
    }

    nrow = atoi(nrows);
    ncol = atoi(ncols);
    while( (opt = getopt(argc, argv, "hu:V:")) > 0 ) {
        switch ( opt ) {
            case 'h':
                show_help(argv[0]) ;
                break ;
            case 'u':
                strncpy(username,optarg,8);
                username[8]='\0'; //poe \0 no ultimo elemento
                puts(username);
                uFARg = 1;

                break ;
            case 'v': //ler variavel ambiente
		            varAmb=optarg;
                printf("%s : %s\n", varAmb ,getenv(varAmb));
                break ;
            default:
                fprintf(stderr, "Opcao invalida ou faltando argumento: `%c'\n", optopt) ;
                return -1 ;
        }
    }


// pede caso nao seja enviado pela linha de comandos

      do{
        do{
          if(uFARg == 0){
            printf("insira o utilizador: ");
            scanf("%s",username );//fgets(username,9,stdin);
            fflush(stdin);
          }
        }while(strlen(username)>8);
        printf("utilizador: %s ",username);
        int ret = checkuser(username);
        printf("return de checkuser %d\n",ret );
        if( ret == 1){
          printf("\nvalido\n");
          skip = 1;
        }
        else if( ret == 2){
          printf("\nJa em uso\n");
          uFARg = 0;
        }
        else{
          printf("\ninvalido\n");
          strcpy(username,"");
          uFARg = 0;
        }
      }
      while(skip != 1);


     //check = checauser(getenv("MEDIT_FICH"),username);

     printf("%d",check);

        char linha[44];
        char linhaoriginal[45];
       initscr(); //INCICIALIZA toda a implementação de estrutura de data
       start_color(); //Inicia funcionalidade das cores
       clear(); //limpa o ecra
       cbreak();// desliga o buffering do input
       curs_set(2); //Trata da visibilidade do cursor (0-invisible,1- normal, 2-high visibilty)
       noecho(); // Não haver echo das teclas
       keypad(stdscr,true);
       nodelay(stdscr,true);
       init_pair(1,COLOR_BLUE, COLOR_BLACK); //numero do par , cor texto , cor fundo
       init_pair(2,COLOR_WHITE, COLOR_BLACK);



       notificacaoWindow = newwin(3,ncol,0,0);
       wbkgd(notificacaoWindow, COLOR_PAIR(1));

       uiWindow = newwin(nrow-3,ncol,3,0);//linhas cols y x
       wbkgd(uiWindow, COLOR_PAIR(2));
       scrollok(uiWindow,true);

       /*shortcutsWindow = newwin(4,75,18,0);
       wprintw(shortcutsWindow,"\n\t\t\tnotificacao");
       wbkgd(shortcutsWindow, COLOR_PAIR(1));*/

       wrefresh(uiWindow);
       wrefresh(notificacaoWindow);
       //wrefresh(shortcutsWindow);

       //getmaxyx(stdscr,nrow,ncol);
       posx = ncol/2;
       posy = nrow/2;
       wclear(notificacaoWindow);
       wclear(uiWindow);
       mvwprintw(notificacaoWindow,0,0,"Modo de navegação no texto");
       move(posy,posx);
       mvwprintw(uiWindow,nrow - 5,(ncol/2)-3, "(%d,%d) ",posy,posx);
       wrefresh(uiWindow);
       wrefresh(notificacaoWindow);
       do{
         do{
           ch = getch();
           oposx=posx;
           oposy=posy;

           wrefresh(stdscr);
           if(posNL!= -1){
             int k;
             for (k = 0; k < 45; k++) {
                  mvaddch(posNL,k,linharecebida[k]);
             }
             wrefresh(uiWindow);
             move(posy,posx);
             posNL = -1;
             //printf("printou esta merda%s\n", linharecebida);
             strcpy(linharecebida,"                                             ");
           }
         }
         while(ch == ERR);


         switch (ch) {
           case 10: // enter key modo de navegação

             edicao= !edicao;
             wclear(notificacaoWindow);
             //wbkgd(notificacaoWindow, COLOR_PAIR(1));
             if(edicao == 0){ // se esta a 0 é porque antes estava a 1 ou seja tem informacao para enviar para o servidor
               wprintw(notificacaoWindow,"Modo de navegação no texto");
               int k;
               for (k = 0; k < 45; k++) {
                 int c = (mvinch(posy, k) & A_CHARTEXT);
                 linha[k] = c;
               }
               unlockline(posy,linha);
               move(posy,posx);
             }
             else if(edicao == 1){  //edicao a 1      manda info para o serv para bloquear a linha
               if(lockline(posy)==1){
                 wprintw(notificacaoWindow,"Modo de edição da linha");
                 //scr_dump("bak");
                 strcpy(linha,""); // lim
                 int k;
                 for (k = 0; k < 45; k++) {
                   int c = (mvinch(posy, k) & A_CHARTEXT);
                   linha[k] = c;
                 }


               }
               else{
                 edicao= !edicao;
                 wprintw(notificacaoWindow,"esta linha ja esta a ser editada");
               }
             }
             move(posy,posx);
             wrefresh(notificacaoWindow);
             break;
           case KEY_BACKSPACE:
             if(edicao==1){
               delch();
               if(posx >1)posx--;
               move(posy,posx);
               }
             break;
           case KEY_DC://faz delete dos chars no screen
             if(edicao==1){
               delch();

               }
             break;
           case 27: //ESC cancela a edicao no modo de edicao de linha
             if(edicao == 1){
               edicao=0;
               //scr_restore("bak");
               strcpy(linhaoriginal,linha);
               int k;
               for (k = 0; k < 45; k++) {
                   mvaddch(posy,k,linhaoriginal[k]);
               }
               unlockline(posy,linhaoriginal);
               move(posy,posx);
               wclear(notificacaoWindow);
               wprintw(notificacaoWindow,"Modo de navegação no texto");
               move(posy,posx);
               wrefresh(notificacaoWindow);
               wrefresh(uiWindow);
             }
             else if(edicao==0){
               endwin();
               logout();
             }
            break;
            //condition ? consequence : alternative
           case KEY_UP:
              posy = (posy>3 && !edicao)?posy-1:posy;
           break;
           case KEY_DOWN:
              posy = (posy<(nrow-1) && !edicao)?posy+1:posy;
           break;
           case KEY_LEFT:
              posx = (posx>0)?posx-1:posx;
           break;
           case KEY_RIGHT:
              posx = (posx < (ncol-1))?posx+1:posx;
           break;
           default :
           if(edicao == 1){
             mvaddch(posy,posx,ch); //move carater
             if(posx<(ncol-1) && posx != (ncol-1)){
               posx++;
             }  //
             else{
               posx = ncol-1;
               //posy++;
             }
             move(posy,posx);
           }
           break;
         }
         if(ch==KEY_UP || ch==KEY_DOWN|| ch==KEY_LEFT|| ch==KEY_RIGHT){
           move(posy,posx);
           mvwprintw(uiWindow,nrow - 5,(ncol/2)-3, "(%d,%d) ",posy,posx);
           wrefresh(uiWindow);
         }
       }
       while (posy!=(nrow-1) || posx!=(ncol-1));
         wgetch(uiWindow);
         endwin();

    if ( argv[optind] != NULL ) {
        int i ;
        puts("* Argumentos em excesso *") ;
        for(i=optind; argv[i] != NULL; i++) {
            printf("-- %s\n", argv[i]) ;
        }
    }
    return 0 ;
}
