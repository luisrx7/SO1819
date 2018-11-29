#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "util.h"

void show_help(char *name) {
    fprintf(stderr, "\
            [uso] %s <opcoes>\n\
            -h         mostra essa tela e sai.\n\
            -f filename escolhe a base de dados do user\n", name) ;
    exit(-1) ;
}

int leficheiro(char *nomefich){
     FILE *fp;
     char str[50];
    fp=fopen(nomefich,"r");
    if(fp==NULL) {puts ("Erro ao abrir ficheiro");
   return 0;
  }
    int done =0;
    char c;
    char * line = NULL;
    char * user = NULL;
    size_t len = 8;
    int read;
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("%s", line);
    }

    fclose(fp);
    return 1;
}
int checauser(char *nomefich,char *username){
     FILE *fp;
     int check=0;
     char str[50];
    fp=fopen(nomefich,"r");
    if(fp==NULL) {puts ("Erro ao abrir ficheiro");
   return 0;
  }
    int done =0;
    char c;
    char * line = NULL;
    char * user = NULL;
    size_t len = 8;
    int read;
    username[strlen(username)]='\n';
    while ((read = getline(&line, &len, fp)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        printf("%s",line );
        if(strcmp(line,username)==0)
          {
            printf("User ja registado na base de dados\n");
            check=1;
          }
    }
    fclose(fp);
    return check;
}

int main(int argc, char** argv) {
  char usersOnline[9][9];//max de 9 users  cada um com 8+1 caracteres

  char comando[20],nomefich[30]={0};
  int sair=0,opt,done=0;
  //verifica se o servidor já está aberto

  char *nomeficheiro = getenv("MEDIT_FICH");
  if(nomeficheiro == NULL){
    puts("variaveis de ambiente nao definidas\n executar . ./script.sh");
    exit(3);
  }
  //strcpy(nomefich,nomeficheiro);
  //puts(nomefich);


  if(access(FIFO_SER,F_OK)==0){
    fprintf(stderr, "[ERRO] Ja ha um SERVIDOR\n");
    exit(1);
  }


  do{
    while((opt=getopt(argc,argv,"hf:")) >0 ){
      switch ( opt ) {
          case 'h': /* help */
              show_help(argv[0]) ;
              break ;
          case 'f': /* opção -f */
              strcpy(nomefich,optarg);
              printf("leficheiro: %s\n", nomefich);
              leficheiro(nomefich);
              break ;
          default:
              fprintf(stderr, "Opcao invalida ou falta argumento: `%c'\n", optopt) ;
              return -1 ;
      }
    }

    if ( strlen(nomefich) <= 1 || done ==0){
      printf("Insira o nome do ficheiro de usernames: ");
      scanf("%s",nomefich );//fgets(username,9,stdin);
      fflush(stdin);
      printf("nomefich: %s \n",nomefich);
      done=leficheiro(nomefich);

    }
  }while(done!=1);

  //shell do servidor
  mkfifo(FIFO_SER,0600);
  fd_ser = open(FIFO_SER,O_RDONLY);
  fd_lixo = open(FIFO_SER,O_WRONLY);//impedir que fique em espera de um cliente
  printf("Comando:" ); fflush(stdout);
  do{
    FD_ZERO(&fontes); // FD_ZERO() clears a set.
    FD_SET(0,&fontes); // add a given file descriptor from a set.
    FD_SET(fd_ser,&fontes);
    res = select(fd_ser+1,&fontes,NULL,NULL,NULL);

    if(res>0 && FD_ISSET(0,&fontes)){ // FD_ISSET() tests to see if a file descriptor is part of the set
        scanf("%s", str);//teclado
        if(strcmp("clientes",str)==0){
          for(int i=0;i<5;i++)
            printf("cliente[%d]=%d\n",i,cliente[i]);
        }
    printf("Comando:" ); fflush(stdout);
    }
    if(res>0 && FD_ISSET(fd_ser,&fontes)){ //pelo pipe
      n =  read( fd_ser,&p,sizeof(PEDIDO));
      switch (p.tipo) {
        case 1://login
        //executa a resposta
          //checka o user e responde se é valido ou nao ou se ja esta a ser usado
          int ret = 0;
          ret = checauser(nomefich,p.username);
          if(ret == 1){
            int i = 0;
            for(i=0;i<9;i++){
              if(strcmp(usersOnline[i],p.username)==0){
                ret = 2;
                break;
              }
            }
            //checkar na tabela de users ligados
            //se tiver o user tiver desligado p.valid = 1 e adicionamos o user e o pid so cliente que pediu a resposta á tabela de users ligados
            //se tiver ligado p.valid = 2
          }
          sprintf(fifo_nome,FIFO_CLI,p.remetente);
          p.remetente = getpid();
          p.tipo =1;
          p.username=username;
          p.valid = ret
          mkfifo(fifo_nome,0600);
          fd_ser = open (FIFO_SER,O_WRONLY);
          n=write(fd_ser,&p,sizeof(PEDIDO));
          printf("Foi enviado %s",username);


        break;
        case 2://normal


        break;
        default:
          fprintf(stderr, "[ERRO] tipo de pedido desconhecido\n");
        break;
      }
      printf("Recebi %d bytes...[%d %c %d =]\n",n,p.num1,p.op,p.num2);

      //verifica se existe ... acrescenta
      int i,pos_cliente=-1,pos_livre=-1;
      for(i=0;i<5;i++){
        if(cliente[i]==p.remetente)
        pos_cliente=i;
        if(cliente[i]==-1 && pos_livre==-1)
        pos_livre=i;
      }
      if(pos_cliente==-1 && pos_livre!=-1)
      cliente[pos_livre]=p.remetente;

      switch (p.op) {
        case '+':p.res=p.num1+p.num2;break;
        case '-':p.res=p.num1-p.num2;break;
        case '*':p.res=p.num1*p.num2;break;
        case '/':p.res=p.num1/p.num2;break;
      }
      sprintf(fifo_nome,FIFO_CLI,p.remetente);
      fd_cli = open(fifo_nome,O_WRONLY);
      n = write(fd_cli, &p, sizeof(PEDIDO));
      close(fd_cli);
      printf("Enviei %d db[%d %c %d = %d])\n",n,p.num1,p.op,p.num2,p.res);

      for(i=0;i<5;i++){
        if(cliente[i]!=-1 && cliente[i]!=p.remetente){
          kill(cliente[i],SIGUSR1);
          sprintf(fifo_nome,FIFO_CLI,cliente[i]);
          fd_cli = open(fifo_nome,O_WRONLY);
          n = write(fd_cli, &p, sizeof(PEDIDO));
          close(fd_cli);
          printf("Enviei %d db[%d %c %d = %d])\n",n,p.num1,p.op,p.num2,p.res);
        }
      }
    }
    }while(1);
    close(fd_ser);
    unlink(FIFO_SER);


    exit(0);
  }



    return (EXIT_SUCCESS);
}
