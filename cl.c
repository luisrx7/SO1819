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
char useraeditarlinha[9];
int posNL=-1;
char linhaeditada[45];

int checkuser(char *username){
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
    for(j=0;j<9;j++){
        if(p.username[j] =='\n' || p.username[j] =='\0'){
            p.username[j] = ' ';
        }
    }
    p.username[8] = '\0';


    for(j=0;j<46;j++){
        if(p.linha[j] =='\n' || p.linha[j] =='\0'){
            p.linha[j] = ' ';
        }
    }
    p.linha[44] = '\0';

    strcpy(linharecebida,"                                            ");
    strcpy(linharecebida,p.linha);

    strcpy(useraeditarlinha,"        ");
    strcpy(useraeditarlinha,p.username);
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
    int posy,posx,fd_cli, res,nrow,ncol,edicao=0,ch;
    char *varAmb=NULL;
    int skip = 0;
    int uFARg = 0;
    fd_set fontes;



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
    WINDOW * lnWindow;

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
    curs_set(1); //Trata da visibilidade do cursor (0-invisible,1- normal, 2-high visibilty)
    noecho(); // Não haver echo das teclas
    keypad(stdscr,true);
    init_pair(1,COLOR_BLUE, COLOR_BLACK); //numero do par , cor texto , cor fundo
    init_pair(2,COLOR_WHITE, COLOR_BLACK);
    init_pair(3,COLOR_RED, COLOR_WHITE);


    notificacaoWindow = newwin(3,ncol,0,0);
    wbkgd(notificacaoWindow, COLOR_PAIR(1));

    uiWindow = newwin(nrow+1,ncol,3,3);//linhas cols y x
    wbkgd(uiWindow, COLOR_PAIR(2));
    scrollok(uiWindow,true);

    lnWindow = newwin(nrow+1,3,3,0);//linhas cols y x
    wbkgd(lnWindow, COLOR_PAIR(1));
    scrollok(lnWindow,true);

    wrefresh(lnWindow);
    /*shortcutsWindow = newwin(4,75,18,0);
    wprintw(shortcutsWindow,"\n\t\t\tnotificacao");
    wbkgd(shortcutsWindow, COLOR_PAIR(1));*/

    wrefresh(uiWindow);



    //nodelay(stdscr,true);
    //getmaxyx(stdscr,nrow,ncol);
    wclear(notificacaoWindow);
    wclear(uiWindow);
    move(0,0);
    wprintw(notificacaoWindow,"Modo de navegação no texto");
    posx = ncol/2;
    posy = nrow/2;
    move(posy,posx);
    // mvwprintw(uiWindow,3,(ncol/2)-3, "(%d,%d) ",posy-3,posx-3);
  /*  wrefresh(uiWindow);
    wrefresh(notificacaoWindow);*/

    int k=0,j=0;

// imprime numero d linhas
  for (j = 0; j < nrow; j++) {
    for (k = 0; k < 3; k++) {
        char SposNL[3];
        sprintf(SposNL, "%d ", j);
        SposNL[2] = '|';
        mvwaddch(lnWindow,j,k,SposNL[k]);
    }
  }



  refresh();
  wrefresh(lnWindow);
  wrefresh(notificacaoWindow);
  wrefresh(uiWindow);

    wrefresh(stdscr);
    int fd_ser,fd_lixo,n;
    char fifo_nome[20];
    PEDIDO p;
    sprintf(fifo_nome,FIFO_CLI,getpid());
    mkfifo(fifo_nome,0600);
    fd_cli = open(fifo_nome,O_RDONLY | O_NONBLOCK);
    fd_lixo = open(fifo_nome,O_WRONLY);//impedir que fique em espera
    fd_ser = open(FIFO_SER,O_WRONLY);


    do{

        move(posy,posx);
        wrefresh(stdscr);
        FD_ZERO(&fontes); // FD_ZERO() clears a set.
        FD_SET(0,&fontes); // add a given file descriptor from a set.
        FD_SET(fd_cli,&fontes);
        res = select(fd_cli+1,&fontes,NULL,NULL,NULL);



        if(res>0 && FD_ISSET(fd_cli,&fontes) ){ //pelo pipe


            /************lê do seu pipe************/
            n=read(fd_cli,&p,sizeof(PEDIDO));

            if(p.carater == 8){ //backspace
              move(p.linhaPoxy,p.linhaPoxx);
              delch();
              move(posy,posx);
              //mvaddch(p.linhaPoxy,p.linhaPoxx,'|');
            }
            if(p.carater == 9){//delete
              move(p.linhaPoxy,p.linhaPoxx);
              delch();
              move(posy,posx);
              //mvaddch(p.linhaPoxy,p.linhaPoxx,'|');
            }

            else{
              mvaddch(p.linhaPoxy,p.linhaPoxx,p.carater);
              move(posy,posx);
            }
/*
            //ler resposta do servidor

            //fd_cli=open(fifo_nome,O_RDONLY);
//            close(fd_cli);


            //if(p.linhaPoxx!= -1){

                 //move carater
                //}
                if(p.linhaPoxy!= -1){


                    int k;
                    for (k = 3; k < (45+3); k++) { //escrever linha
                        mvaddch(posNL,k,linharecebida[k-3]);
                    }

                    //escrever n linha


                    /*
                    for (k = 0; k < 9; k++) { //escrever user
                        mvaddch(posNL,k+ncol+3+2,useraeditarlinha[k]);
                    }

                    //mvwprintw(lnWindow,posNL,0,c);
                    wrefresh(uiWindow);
                    wrefresh(lnWindow);
                    move(posy,posx);
                    posNL = -1;
                    //printf("printou esta merda%s\n", linharecebida);
                    strcpy(linharecebida,"                                            ");
                }*/
        }

        if(res>0 && FD_ISSET(0,&fontes)){   //teclado

                ch = getch();

                switch (ch) {
                    case 10: // enter key modo de navegação

                    edicao= !edicao;
                    wclear(notificacaoWindow);
                    //wbkgd(notificacaoWindow, COLOR_PAIR(1));
                    if(edicao == 0){ // se esta a 0 é porque antes estava a 1 ou seja tem informacao para enviar para o servidor
                        wprintw(notificacaoWindow,"Modo de navegação no texto");
                        int k;
                        for (k = 3; k < 48; k++) {
                            int c = (mvinch(posy, k) & A_CHARTEXT);
                            linha[k-3] = c;
                        }
                        unlockline(posy,linha);
                        move(posy,posx);
                    }
                    else if(edicao == 1){  //edicao a 1      manda info para o serv para bloquear a linha
                        if(lockline(posy)==1){
                            wprintw(notificacaoWindow,"Modo de edição da linha");
                            //scr_dump("bak");
                            strcpy(linha,""); // lim

                            /*   backup da linha   */

                            int k;
                            for (k = 3; k < 48; k++) {
                                int c = (mvinch(posy, k) & A_CHARTEXT);
                                linha[k-3] = c;
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
                      if(posx >1)posx--;
                  /*    move(posy,posx);
                        delch();*/


                        p.remetente = getpid();
                        p.tipo = 5;
                        p.linhaPoxx = posx;
                        p.linhaPoxy = posy;
                        p.carater = 8;

                        n=write(fd_ser,&p,sizeof(PEDIDO));
                    }
                    break;
                    case KEY_DC://faz delete dos chars no screen
                    if(edicao==1){
                      //  delch();
                      p.remetente = getpid();
                      p.tipo = 5;
                      p.linhaPoxx = posx;
                      p.linhaPoxy = posy;
                      p.carater = 8;

                      n=write(fd_ser,&p,sizeof(PEDIDO));
                    }
                    break;
                    case 27: //ESC cancela a edicao no modo de edicao de linha
                    if(edicao == 1){
                        edicao=0;
                        //scr_restore("bak");
                        strcpy(linhaoriginal,linha);
                        int k;
                        for (k = 0; k < 45; k++) {
                          p.remetente = getpid();
                          p.tipo = 5;
                          p.linhaPoxx = k;
                          p.linhaPoxy = posy;
                          p.carater = linhaoriginal[k];

                          n=write(fd_ser,&p,sizeof(PEDIDO));
                            //mvwaddch(uiWindow,posy,k,linhaoriginal[k-3]);
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
                    move(posy,posx);
                    break;
                    case KEY_DOWN:
                    posy = (posy<((nrow+3)-1) && !edicao)?posy+1:posy;
                    move(posy,posx);
                    break;
                    case KEY_LEFT:
                    posx = (posx>3)?posx-1:posx;
                    move(posy,posx);
                    break;
                    case KEY_RIGHT:
                    posx = (posx < ((ncol+2)))?posx+1:posx;
                    move(posy,posx);
                    break;


                    default :
                    if(edicao == 1){
                        //envia o carater para o SERVIDOR
                        //nao imprime para o ecra
                        //recebe do servidor e imprime

                        //strcpy(p.linha,linhatxt);
                        //strcpy(p.username,username);
                        p.remetente = getpid();
                        p.tipo = 5;
                        p.linhaPoxx = posx;
                        p.linhaPoxy = posy;
                        p.carater = ch;


                        n=write(fd_ser,&p,sizeof(PEDIDO));
                        //printf("Foi enviado ao serv o char:%d[%d,%d] %s\t", ch,posx,posy);

                        //mvaddch(posy,posx,ch); //move carater


                        if(posx<(ncol+2) && posx != (ncol+2)){
                            posx++;
                        }  //
                        else{
                            posx = ncol+2;

                            //posy++;
                        }
                        move(posy,posx);
                    }
                    break;
                }
                if(ch==KEY_UP || ch==KEY_DOWN|| ch==KEY_LEFT|| ch==KEY_RIGHT){
                    move(posy,posx);
                    mvwprintw(notificacaoWindow,2,(ncol/2)-3, "(%d,%d) ",posy-3,posx-3);
                    wrefresh(stdscr);
                }
            //}
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
