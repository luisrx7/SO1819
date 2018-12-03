#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "util.h"


CLIENTE usersOnline[5];


void show_help(char *name) {
    fprintf(stderr, "\
            [uso] %s <opcoes>\n\
            -h         mostra essa tela e sai.\n\
            -f filename escolhe a base de dados do user\n", name) ;
    exit(-1) ;
}


void sair(int s){//guarda dados e manda sinal para os clientes online a avisar que vai encerrar
  int i;
  for(i=0;i<5;i++){
    if(usersOnline[i].userPid !=-1){
      kill(usersOnline[i].userPid,SIGUSR1);
      printf("Enviei o sinal SIGUSR1 ao pid %d [%s])\n",usersOnline[i].userPid,usersOnline[i].user);
    }
  }
unlink(FIFO_SER);
exit(0);
}

int broadcastficheiro(char *nomefich,int piduser){
    FILE *fp;
    char fifo_nome[20];
    fp=fopen(nomefich,"r");
    char line[45];
    int i,n,fd_cli,nl=3,j;
    PEDIDO p;
    if(fp==NULL) {
      printf("Erro ao abrir ficheiro %s\n",nomefich);
      return 0;
    }
    while (fgets(line,46, fp)!=NULL) {
    /*  for(j=0;j<45;j++){
        if(line[j] =='\n'){
          line[j] = ' ';
        }
      }
      line[45] = '\0';*/

      for(i=0;i<5;i++){
        if(usersOnline[i].userPid == piduser){
          strcpy(p.linha,line);
          p.linhaPoxy = nl;
          kill(usersOnline[i].userPid,SIGUSR2);
          sprintf(fifo_nome,FIFO_CLI,usersOnline[i].userPid);
          fd_cli = open(fifo_nome,O_WRONLY);
          n = write(fd_cli, &p, sizeof(PEDIDO));
          close(fd_cli);
          usleep(50000);
          printf("%s\n", p.linha);
          strcpy(p.linha,"                                             ");
          printf("[broadcast]Enviei o sinal SIGUSR2 ao pid %d [%s])\n",usersOnline[i].userPid,usersOnline[i].user);
        }
      }
      nl++;
    }

}


int gravanoficheiro(char *nomefich,char *linha, int linhaPoxy){
  FILE *fp1;
  FILE *fp2;
  char fifo_nome[20];
  fpos_t position;
  char *ret;
  linhaPoxy -=3;
  fp1=fopen(nomefich,"r");
  fp2=fopen("bak2","w");
  //fgetpos(fp, &position);
  int i=0;
  char line[46];
/*  while ((c=fgetc(fp))!=EOF) {
    if(c==EOF && (i < linhaPoxy)){
      fprintf(fp,"\n");
    }


    if(c == '\n'){
      i++;
      fsetpos(fp, &position);
    }
    if(i == linhaPoxy){
      fgetpos(fp, &position);
      fprintf(fp,"%s\n",linha);
    }
  }*/

while ((ret = fgets(line,46, fp1))!=NULL) {

  if(i!=linhaPoxy){ //caso normal copia simples
      fprintf(fp2,"%s",line);
  }
  else{ //é a linha que queremos
      fprintf(fp2,"%s",linha);
  }
  i++;
  fgetpos(fp1, &position);
}
/*
9
10awdawdawdaw
11
12knvkrdnlg
*/

if(ret == NULL && i<linhaPoxy){
  int j;
//  i--;
  for(j=0;j<(linhaPoxy - i);j++){
    fprintf(fp2,"\n");

    printf("foudeu\n");
  }
  fprintf(fp2,"%s",linha);
  printf("foudeu2\n");
}


 /*fseek(fp, 45*linhaPoxy, 1);
 fprintf(fp,"%s\n",linha);*/


  fclose (fp1);
  fclose (fp2);
  rename("bak2","texto.txt");
  return 1;
}


int leficheiro(char *nomefich){ //1 se ler   0 se nao
    FILE *fp;
    fp=fopen(nomefich,"r");
    if(fp==NULL) {puts ("Erro ao abrir ficheiro");
   return 0;
  }
    char * line = NULL;
    size_t len = 8;
    int read;
    while ((read = getline(&line, &len, fp)) != -1) {
      //  printf("%s", line);
    }

    fclose(fp);
    return 1;
}

int checauser(char *nomefich,char *username){ //1 se tiver o username no ficheiro 0 se nao
     int check=0;
     FILE *fp;
    fp=fopen(nomefich,"r");
    if(fp==NULL) {puts ("Erro ao abrir ficheiro");
   return 0;
  }
    char line[10];

    printf("\nusername[%s]\n",username);

    while (fgets(line,10, fp)!=NULL) {

      int i;
      for(i=0;i<9;i++){
        if(line[i]=='\n'){
          line[i]='\0';
        }
      }

        printf("\nlinha[%s]",line );


        if(strcmp(line,username)==0)
          {
            printf("User ja registado na base de dados\n");
            check=1;
            break;
          }

    }
    fclose(fp);
    return check;
}

int main(int argc, char *argv[],char *envp[]) {
  //char usersOnline[9][9];//max de 9 users  cada um com 8+1 caracteres
  //char pidUsersOnline[9];
  char comando[20],nomefich[30];
  int n,opt,done=0;
  int fd_lixo,fd_ser, res,fd_cli,maxusers=5;
  char fifo_nome[20],str[20];
  fd_set fontes;
  PEDIDO p;

  signal(SIGINT,sair);

  //char *smaxusers = getenv("MEDIT_MAXUSERS");
  char *nomeficheiro = getenv("MEDIT_FICH");
  if(nomeficheiro == NULL){
    puts("variaveis de ambiente nao definidas\n executar . ./script.sh");
    exit(3);
  }

  //maxusers = atoi(smaxusers);

  int i;
  for(i=0;i<maxusers;i++){
    usersOnline[i].userPid= -1;
    usersOnline[i].editinglineN = -1;
    strcpy(usersOnline[i].user,"NULL");
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
              done=leficheiro(nomefich);
              break ;
          default:
              fprintf(stderr, "Opcao invalida ou falta argumento: `%c'\n", optopt) ;
              return -1 ;
      }
    }
    if ( (strlen(nomefich) < 1) || done ==0){
      printf("Insira o nome do ficheiro de usernames: ");
      scanf("%s",nomefich );
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
          for(int i=0;i<maxusers;i++)
            printf("cliente[%d]=%d\t%s\n",i,usersOnline[i].userPid,usersOnline[i].user);
        }
        if(strcmp("sair",str)==0){
          //desliga os clientes e sai
          sair(SIGUSR1);

        }
    printf("Comando:" ); fflush(stdout);
    }


    if(res>0 && FD_ISSET(fd_ser,&fontes)){ //pelo pipe
      n =  read( fd_ser,&p,sizeof(PEDIDO));
      int ret = 0;
      int i = 0;
      int pidtosend = 0;
      switch (p.tipo) {
        case 1:
          //login
          //executa a resposta
          //checka o user e responde se é valido ou nao ou se ja esta a ser usado
          ret = checauser(nomefich,p.username);
          if(ret == 1){
            for(i=0;i<maxusers;i++){
              if(strcmp(usersOnline[i].user,p.username)==0){// se tiver ja logado na tabela de clientes
                printf("O user: %d fez login.\n",usersOnline[i].user);
                ret = 2;
                break;
              }
            }
            if(ret == 1){ //senao tiver logado adiciona a lista de users online
              for(i=0;i<maxusers;i++){
                if(strcmp(usersOnline[i].user,"NULL")==0){// se tiver ja logado
                  strcpy(usersOnline[i].user,p.username);
                  usersOnline[i].userPid = p.remetente;
                  pidtosend = p.remetente;
                  break;
                }
              }
            }
          }
          //checkar na tabela de users ligados
          //se tiver o user tiver desligado p.valid = 1 e adicionamos o user e o pid so cliente que pediu a resposta á tabela de users ligados
          //se tiver ligado p.valid = 2
          sprintf(fifo_nome,FIFO_CLI,p.remetente);
          p.remetente = getpid();
          p.tipo =1;
          p.valid = ret;
          mkfifo(fifo_nome,0600);
          fd_cli = open (fifo_nome,O_WRONLY);
          n=write(fd_cli,&p,sizeof(PEDIDO));
          close(fd_cli);
          printf("Foi enviado %d bytes em resposta ao user \n",n);

          if(ret == 1){
            usleep(700000);
            broadcastficheiro("texto.txt",pidtosend);

          }




        break;

        case 2://logout
        //receber

        for(i=0;i<maxusers;i++){
          if(usersOnline[i].userPid==p.remetente){// se tiver ja logado faz o logout
            printf("O user: %s - %d fez logout.\n",usersOnline[i].user,usersOnline[i].userPid);
            strcpy(usersOnline[i].user,"NULL");
            usersOnline[i].userPid= -1;
            break;
          }

        }

        break;

        case 3: //lockline

          ret = 1;
          for(i=0;i<maxusers;i++){
            if(usersOnline[i].editinglineN==p.linhaPoxy){// se a linha ja tiver a ser editada
              printf("O user: %s tem  a linha %d bloqueada.\n",usersOnline[i].user,p.linhaPoxy);
              ret = 0;
              strcpy(p.username,usersOnline[i].user);
              break;
            }
          }
          if(ret == 1){ //senao tiver locked bloqueia e responde ao user a dizer que pode editar
            for(i=0;i<maxusers;i++){
              if(strcmp(usersOnline[i].user,p.username)==0){ //encontra user na tabela
                usersOnline[i].editinglineN = p.linhaPoxy;
                printf("O user: %s bloqueou a linha %d.\n",usersOnline[i].user,p.linhaPoxy);
              }
            }
          }
        sprintf(fifo_nome,FIFO_CLI,p.remetente);
        p.remetente = getpid();
        p.tipo =1;
        p.valid = ret;
        mkfifo(fifo_nome,0600);
        fd_cli = open (fifo_nome,O_WRONLY);
        n=write(fd_cli,&p,sizeof(PEDIDO));
        close(fd_cli);
        printf("Foi enviado %d bytes em resposta ao pedido de lockline \n",n);


        break;



        case 4: //unlockline
          printf("Foi recebido %d bytes do pedido de unlockline \n",n);
          ret = 1;
          for(i=0;i<maxusers;i++){
            if(usersOnline[i].editinglineN==p.linhaPoxy){// se a linha ja tiver a ser editada
              printf("O user: %s desbloqueou a linha %d.\n",usersOnline[i].user,p.linhaPoxy);
              usersOnline[i].editinglineN = -1;  //desbloqueia a linha
              break;
            }
          }

          //gravar a nova linha no ficheiro
          if(gravanoficheiro("texto.txt",p.linha,p.linhaPoxy)==1){
            printf("alteracao da linha %d efetuada com exito\n",p.linhaPoxy);
          }
          else{
            printf("alteracao da linha %d efetuada sem exito\n",p.linhaPoxy);
          }

          printf("broadcast da nova linha\n");
          for(i=0;i<5;i++){
            if(usersOnline[i].userPid !=-1 && usersOnline[i].userPid != p.remetente){
              kill(usersOnline[i].userPid,SIGUSR2);
              sprintf(fifo_nome,FIFO_CLI,usersOnline[i].userPid);
              fd_cli = open(fifo_nome,O_WRONLY);
              n = write(fd_cli, &p, sizeof(PEDIDO));
              close(fd_cli);
              printf("Enviei o sinal SIGUSR2 ao pid %d [%s])\n",usersOnline[i].userPid,usersOnline[i].user);
            }
          }
          printf("%s\n", p.linha);


        /*sprintf(fifo_nome,FIFO_CLI,p.remetente);
        p.remetente = getpid();
        p.tipo =1;
        p.valid = ret;
        mkfifo(fifo_nome,0600);
        fd_cli = open (fifo_nome,O_WRONLY);
        n=write(fd_cli,&p,sizeof(PEDIDO));
        close(fd_cli);
        printf("Foi enviado %d bytes em resposta ao pedido de lockline \n",n);*/

        break;














        default:
          fprintf(stderr, "[ERRO] tipo de pedido desconhecido\n");
        break;
      }
      printf("Recebi %d bytes...[]\n",n);
/*
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
      }*/
    }
    }while(1);
    close(fd_ser);
    unlink(FIFO_SER);


    exit(0);
  }
