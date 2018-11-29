#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include "util.h"

int checkuser(char *username){
  int fd_cli;
  char fifo_nome[20];
  PEDIDO p;
  int n;
  sprintf(fifo_nome,FIFO_CLI,getpid());
  p.remetente = getpid();
  p.tipo =1;
  p.username=username;
  mkfifo(fifo_nome,0600);
  fd_ser = open (FIFO_SER,O_WRONLY);
  n=write(fd_ser,&p,sizeof(PEDIDO));
  printf("Foi enviado %s",username);
  fd_cli=open(fifo_nome,O_RDONLY);
  n=read(fd_cli,&p,sizeof(PEDIDO));
  close(fd_cli);
  printf("Recebi valid =  %d",p.valid);
  return p.valid;

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
    char username[8]={0},*varAmb=NULL;

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
                username[8]='\0';
                puts(username);
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
    if ( strlen(username) <= 1){ // pede caso nao seja enviado pela linha de comandos
      printf("insira o utilizador: ");
      scanf("%8s",username );//fgets(username,9,stdin);
      fflush(stdin);
      printf("utilizador: %s ",username);
      int ret = checkuser(username);
      if( ret == 1){
        printf("valido\n");
      }
      if( ret == 2){
        printf("Ja em uso\n");
      }
      else{
        printf("invalido\n");
      }
	   }

     check = checauser(getenv("MEDIT_FICH"),username);

     printf("%d",check);


       initscr(); //INCICIALIZA toda a implementação de estrutura de data
       start_color(); //Inicia funcionalidade das cores
       clear(); //limpa o ecra
       cbreak();// desliga o buffering do input
       curs_set(2); //Trata da visibilidade do cursor (0-invisible,1- normal, 2-high visibilty)
       noecho(); // Não haver echo das teclas
       keypad(stdscr,true);
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
       move(posy,posx);
       wrefresh(uiWindow);
       do{
         ch = getch();
         oposx=posx;
         oposy=posy;
         switch (ch) {
           case 10: // enter key modo de navegação
             edicao= !edicao;
             wclear(notificacaoWindow);
             //wbkgd(notificacaoWindow, COLOR_PAIR(1));
             if(edicao == 0){
               wprintw(notificacaoWindow,"Modo de navegação no texto");
             }
             else{
               wprintw(notificacaoWindow,"Modo de edição da linha");
               scr_dump("bak");
             }
             move(posy,posx);
             wrefresh(notificacaoWindow);
             break;
           case KEY_BACKSPACE:
           case KEY_DC://faz delete dos chars no screen
             if(edicao==1){
               delch();
               }
             break;
           case 27: //ESC cancela a edicao no modo de edicao de linha
             if(edicao){
               edicao=0;
               scr_restore("bak");
               wclear(notificacaoWindow);
               wprintw(notificacaoWindow,"Modo de navegação no texto");
               move(posy,posx);
               wrefresh(notificacaoWindow);
               wrefresh(uiWindow);
             }
            break;
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
              posx = (posx<(ncol-2))?posx+1:posx;
           break;
           default :
           if(edicao == 1){
             mvaddch(posy,posx,ch);
             if(posx<(ncol-1)) posx++;
             else if(posy < (nrow-2)){
               posx = 0;
               posy++;
             }
             else{
               mvaddch(posy,posx,ch);
             }
           }
           break;
         }
         if(ch==KEY_UP || ch==KEY_DOWN|| ch==KEY_LEFT|| ch==KEY_RIGHT){
           move(posy,posx);
           mvwprintw(uiWindow,nrow - 5,(ncol/2)-3, "(%d,%d) ",posy,posx);
           wrefresh(uiWindow);
         }
         else{

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
