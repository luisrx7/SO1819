#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "util.h"


CLIENTE usersOnline[5];
int childpid;

void show_help(char *name) {
	fprintf(stderr, "\
	[uso] %s <opcoes>\n\
	-h         mostra essa tela e sai.\n\
	-f filename escolhe a base de dados do user\n", name) ;
	exit(-1) ;
}

void sair(int s){// manda sinal para os clientes online a avisar que vai encerrar
	int i;
	for(i=0;i<5;i++){
		if(usersOnline[i].userPid !=-1){
			kill(usersOnline[i].userPid,SIGUSR1);
			printf("Enviei o sinal SIGUSR1 ao pid %d [%s])\n",usersOnline[i].userPid,usersOnline[i].user);
		}
	}
	unlink(FIFO_SER);
	kill(childpid,SIGINT);
	exit(0);
}
int saveFicheiro(char *nomefich){
	FILE *fp;
	fp=fopen(nomefich,"a");
	if (fp == NULL)
	{
    printf("Error opening file!\n");
    exit(1);
	}

}
int broadcastficheiro(char *nomefich,int piduser,int ncol1){
	FILE *fp,*fg;
	char fifo_nome[20];
	fp=fopen(nomefich,"r");
	fg=fopen("guardaFicheiro.txt","w");
	char line[200];
	int i,n,fd_cli,nl=0,j;
	PEDIDO p;
	if(fp==NULL || fg==NULL) {
		printf("1:Erro ao abrir ficheiro %s\n",nomefich);
		return 0;
	}
	while (fgets(line,200, fp)!=NULL) {
		for(i=0;i<5;i++){
			if(usersOnline[i].userPid == piduser){
				strcpy(p.linha,line);
				sprintf(fifo_nome,FIFO_CLI,usersOnline[i].userPid);
				fd_cli = open(fifo_nome,O_WRONLY);
				for ( j = 0; j < strlen(p.linha); j++) {
					if(isalnum(p.linha[j])!=0 || 1==1){
						p.remetente = getpid();
						p.tipo = 5;
						p.carater = p.linha[j];
						p.linhaPoxy = nl;
						p.linhaPoxx = j;
						write(fd_cli, &p, sizeof(PEDIDO));
					}
				}
				printf("%s", p.linha);
				memset( p.linha,0,300);
			}
			close(fd_cli);
		}
		nl++;
		memset(line,0,200);
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
	int i=0;
	char line[46];
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
	if(ret == NULL && i<linhaPoxy){
		int j;
		for(j=0;j<(linhaPoxy - i);j++){
			fprintf(fp2,"\n");
		}
		fprintf(fp2,"%s",linha);
	}
	fclose (fp1);
	fclose (fp2);
	rename("bak2",nomefich);
	return 1;
}

int leficheiro(char *nomefich){ //1 se ler   0 se nao
	if(access( nomefich, F_OK ) != -1 ) {
		return 1;
	}
	else{
		printf("2:Erro ficheiro [%s] nao existe\n",nomefich);
		return 0;
	}
/*
char * line = NULL;
size_t len = 8;
int read;
while ((read = getline(&line, &len, fp)) != -1) {
	//  printf("%s", line);
}
*/
}

int load(char *nomeficheiro,int nlinhas){
	FILE *fp;
	char fifo_nome[20];
	fp=fopen(nomeficheiro,"r");
	char line[45];
	int nl=0,n,fd_cli,j,i=0,x=0;
	int read;
	PEDIDO p;
	if(fp==NULL) {
		printf("3:Erro ao abrir ficheiro %s\n",nomeficheiro);
		return 0;
	}
	printf("%d",nlinhas);
	while(x<=nlinhas){
		for(i=0;i<5;i++){
			if(usersOnline[i].userPid !=-1){
				strcpy(line,"                                            ");
				strcpy(p.linha,line);
				p.linhaPoxy = x+3;
				kill(usersOnline[i].userPid,SIGUSR2);
				sprintf(fifo_nome,FIFO_CLI,usersOnline[i].userPid);
				fd_cli = open(fifo_nome,O_WRONLY);
				n = write(fd_cli, &p, sizeof(PEDIDO));
				close(fd_cli);
				usleep(50000);
			}
		}
		x++;
	}
	while (fgets(line,46, fp)!=NULL && nl<=nlinhas) {
		for(i=0;i<5;i++){
			if(usersOnline[i].userPid !=-1){
				strcpy(p.linha,line);
				p.linhaPoxy = nl+3;
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
	fclose(fp);
}

int checauser(char *nomefich,char *username){ //1 se tiver o username no ficheiro 0 se nao
	int check=0;
	FILE *fp;
	fp=fopen(nomefich,"r");
	if(fp==NULL) {
		printf("4:Erro ao abrir ficheiro %s\n",nomefich);
		return 0;
	}
char line[100];
printf("\nusername[%s]\n",username);

int ret;

do{
	ret = fscanf(fp,"%s",line);
	int i;
	/*for(i=0;i<9;i++){
		if(line[i]=='\n'){
				line[i]='';
		}
	}*/
	//printf("\nlinha[%s]\n",line );
	if(strcmp(line,username)==0)
	{
		//printf("User registado na base de dados\n");
		check=1;
		break;
	}
}while(feof(fp) == 0);
fclose(fp);
return check;
}

int main(int argc, char *argv[],char *envp[]) {
	char comando[20],nomefich[30],ficheiroEdit[30];
	int n,opt,done=0,limpa=0;
	int fd_lixo,fd_ser, res,fd_cli,maxusers=5;
	char fifo_nome[20],str[20];
	fd_set fontes;
	PEDIDO p;
	int canal[2],canal2[2];
	char limpabuffer[200],readbuffer[300],*token;
	const char s[2] = " ";
	int nbytes,j=0;
	char *palavrasRecebidas[20];
	char *palavrasErradas[20];
	signal(SIGINT,sair);
	pipe(canal);
	pipe(canal2);
	//fcntl(canal2[0], F_SETFL, O_NONBLOCK);
	if((childpid = fork()) == -1){
		perror("fork");
		exit(1);
	}
	if(childpid  == 0){//fecha canal de stdin e faz copia de stdin para canal[0]
		fprintf(stderr,"filho %d com pai %d\n", getpid(), getppid());
		dup2(canal2[1],1);//trocar stdout do aspell para o canal2
		dup2(canal[0],0); //trocar stdin do aspell para o canal
		execlp("aspell","aspell","--dont-suggest","-a",NULL);//--dont-suggest
		exit(0);
	}
	else{
		sleep(1);
		  //char *smaxusers = getenv("MEDIT_MAXUSERS");
			int nrow = 15;
			int ncol = 45;
		char *nrows = getenv("MEDIT_MAXLINES");
		char *ncols = getenv("MEDIT_MAXCOLUMNS");
		char *nomeficheiro = getenv("MEDIT_FICH");
		strcpy(nomeficheiro, "medit.db");
		if(nomeficheiro == NULL || nrows == NULL || ncols == NULL){
			puts("algumas variaveis de ambiente nao definidas\n executar . ./script.sh");
			//exit(3);
		}
		 nrow = atoi(nrows); //Para fazer a conversao
		 ncol = atoi(ncols);
		 char *ficheiro[nrow];
		//maxusers = atoi(smaxusers);
		int i,a=0;
		for(i=0;i<maxusers;i++){
			usersOnline[i].userPid= -1;
			usersOnline[i].editinglineN = -1;
			strcpy(usersOnline[i].user,"NULL");
		}
		strcpy(nomefich,nomeficheiro);
		puts(nomefich);
		done=leficheiro(nomefich);
		if(access(FIFO_SER,F_OK)==0){
			fprintf(stderr, "[ERRO] Ja ha um SERVIDOR\n");
			exit(1);
		}
		while((opt=getopt(argc,argv,"hdf:")) >0 ){
			switch ( opt ) {
				case 'h': /* help */
				show_help(argv[0]) ;
				break ;
				case 'd': /* opção -d */
				strcpy(nomefich,optarg);
				printf("leficheiro: %s\n", nomefich);
				done=leficheiro(nomefich);
				break ;
				case 'f': /* opção -f */
				strcpy(ficheiroEdit,optarg);
				//printf("leficheiro: %s\n", nomefich);
				do{
					if(leficheiro(ficheiroEdit)!=1){
						printf("5:erro ao abrir ficheiro %s\n nome do ficheiro:",ficheiroEdit);
						scanf("%s",ficheiroEdit);
					}
					else{
						a=1;
					}
				}while(a!=1);
				break ;
				default:
				fprintf(stderr, "Opcao invalida ou falta argumento: `%c'\n", optopt);
				return -1 ;
			}
		}
		do{
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
		fd_ser = open(FIFO_SER,O_RDONLY | O_NONBLOCK);
		fd_lixo = open(FIFO_SER,O_WRONLY);//impedir que fique em espera de um cliente
		printf("Comando:" ); fflush(stdout);
		do{
			FD_ZERO(&fontes); // FD_ZERO() clears a set.
			FD_SET(0,&fontes); // add a given file descriptor from a set.
			FD_SET(fd_ser,&fontes);
			res = select(fd_ser+1,&fontes,NULL,NULL,NULL);

			if(res>0 && FD_ISSET(0,&fontes)){ // FD_ISSET() tests to see if a file descriptor is part of the set
				scanf("%s", str);//teclado
				if(strcmp("users",str)==0){
					for(int i=0;i<maxusers;i++)
					printf("cliente[%d]=%d\t%s\t%d\n",i,usersOnline[i].userPid,usersOnline[i].user,usersOnline[i].editinglineN);
				}
				if(strcmp("shutdown",str)==0){
					//desliga os clientes e sai
					sair(SIGUSR1);
				}
				if(strcmp("settings",str)==0){ // Mostra no excran os settings actuais
					printf("numero de linhas:%d\t numero de colunas:%d\t nome da base de dados:%s \n numero do pipe de iteracao: RESOLVER\t nome do pipe principal:%s\n",nrow,ncol,nomeficheiro,FIFO_SER);
				}
				if(strcmp("load",str)==0){
					printf("Insira nome do ficheiro: \n");
					scanf("%s",nomefich);
					load(nomefich,nrow);
				}
				if(strcmp("save",str)==0){
					printf("Insira nome do ficheiro a guardar: \n");
					scanf("%s",nomefich);
					//save(nomefich);
				}
				if(strcmp("free",str)==0){

					printf("free");
				}
				if(strcmp("statistics",str)==0){

					printf("statistics");
				}
				if(strcmp("users",str)==0){

					printf("users");
				}

				printf("Comando:" ); fflush(stdout);
			}
			if(res>0 && FD_ISSET(fd_ser,&fontes)){ //pelo pipe
				n =  read( fd_ser,&p,sizeof(PEDIDO));
				int ret = 0,x=0;
				int i = 0;
				int pidtosend = 0;
				//char usernameLOCK[9];

				switch (p.tipo) {
					case 1:
					//login
					//executa a resposta
					//checka o user e responde se é valido ou nao ou se ja esta a ser usado
					ret = checauser(nomefich,p.username);
					if(ret == 1){
						for(i=0;i<maxusers;i++){
							if(strcmp(usersOnline[i].user,p.username)==0){// se tiver ja logado na tabela de clientes
								ret = 2;
								break;
							}
						}
						if(ret == 1){ //senao tiver logado adiciona a lista de users online
							for(i=0;i<maxusers;i++){
								if(strcmp(usersOnline[i].user,"NULL")==0){
									strcpy(usersOnline[i].user,p.username);
									printf("O user: %s - %d fez login.\n",usersOnline[i].user,p.remetente);
									usersOnline[i].userPid = p.remetente;
									pidtosend = p.remetente;
									break;
								}
							}
						}
					}
					sprintf(fifo_nome,FIFO_CLI,p.remetente);
					p.remetente = getpid();
					p.tipo =1;
					p.valid = ret;
					p.linhaPoxx = ncol;
					p.linhaPoxy = nrow;
					mkfifo(fifo_nome,0600);
					fd_cli = open (fifo_nome,O_WRONLY);
					n=write(fd_cli,&p,sizeof(PEDIDO));
					close(fd_cli);
					printf("Foi enviado %d bytes em resposta ao user \n",n);
					if(ret == 1){
						sleep(1);
						broadcastficheiro(ficheiroEdit,pidtosend,ncol);
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
							//strcpy(p.username,usersOnline[i].user);
							break;
						}
					}
					if(ret == 1){ //senao tiver locked bloqueia e responde ao user a dizer que pode editare
						for(i=0;i<maxusers;i++){
							if(strcmp(usersOnline[i].user,p.username)==0){ //encontra user na tabela
								usersOnline[i].editinglineN = p.linhaPoxy;
								printf("O user: %s bloqueou a linha %d.\n",usersOnline[i].user,p.linhaPoxy);
								//strcpy(usernameLOCK,usersOnline[i].user);
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
					/*	 manda para todos os users o novo bloqueio de linha	*/
					if(ret == 1){
						for(i=0;i<maxusers;i++){
								sprintf(fifo_nome,FIFO_CLI,usersOnline[i].userPid);
								fd_cli = open(fifo_nome,O_WRONLY);
								p.remetente = getpid();
								p.tipo = 3;
								//strcpy(p.username,usernameLOCK);
								write(fd_cli, &p, sizeof(PEDIDO));
								close(fd_cli);
						}
					}
					//printf("Foi enviado %d bytes em resposta ao pedido de lockline \n",n);
					break;


					case 4: //unlockline
					printf("Foi recebido %d bytes do pedido de unlockline da linha %d  \n",n,p.linhaPoxy);

					pidtosend = p.remetente;


					printf("Received string //teste: [%s]: \n",p.linha);
					p.linha[ncol]='\0';
					token = strtok(p.linha, s);
					while( token != NULL ) {
						//printf("Enviei [%s]: \n",token);
						//token[strlen(token)]='\n';
						palavrasRecebidas[j]=malloc(strlen(token)+1);
						strcpy(palavrasRecebidas[j],token);
						token = strtok(NULL, s);
						printf("Vou enviar ao aspell [%s]: \n",palavrasRecebidas[j]);
						j++;
					}
					for(i=0;i<j;i++){
						write(canal[1],palavrasRecebidas[i],strlen(palavrasRecebidas[i]));//write canal[1]
						write(canal[1],"\n",1);
						memset( readbuffer,0,sizeof(readbuffer));
						if(limpa==0){
							nbytes = read(canal2[0], readbuffer, sizeof(readbuffer));
							limpa=1;
							memset( readbuffer,0,sizeof(readbuffer));
						}
						nbytes = read(canal2[0], readbuffer, sizeof(readbuffer));
						if(nbytes==1){
							memset( readbuffer,0,sizeof(readbuffer));
							write(canal2[1],s,1);
							nbytes = read(canal2[0], readbuffer, sizeof(readbuffer));
						}
						//readbuffer[strlen(readbuffer)-1] = '\0';
						//printf("Received string: %d [%s]: \n", nbytes,readbuffer);
						if(readbuffer[0]=='#'){
							strcpy(p.palavrasErradas[x],palavrasRecebidas[i]);
							printf("Palavras Erradas,recebidas do aspell [%s]: \n",p.palavrasErradas[x]);
							x++;
						}
					}




					if(p.nPalavrasErradas == 0){
						for(i=0;i<maxusers;i++){
							if(usersOnline[i].editinglineN==p.linhaPoxy){// se a linha ja tiver a ser editada
								printf("O user: %s desbloqueou a linha %d.\n",usersOnline[i].user,p.linhaPoxy);
								usersOnline[i].editinglineN = -1;  //desbloqueia a linha

								break;
							}
						}

						sprintf(fifo_nome,FIFO_CLI,pidtosend);
						p.remetente = getpid();
						p.tipo =1;
						p.valid = 1;
						mkfifo(fifo_nome,0600);
						fd_cli = open (fifo_nome,O_WRONLY);
						write(fd_cli, &p, sizeof(PEDIDO));

						for(i=0;i<maxusers;i++){
							if(usersOnline[i].userPid!= -1){
								sprintf(fifo_nome,FIFO_CLI,usersOnline[i].userPid);
								fd_cli = open(fifo_nome,O_WRONLY);
								p.remetente = getpid();
								p.tipo = 4;
								//strcpy(p.username,usernameLOCK);
								write(fd_cli, &p, sizeof(PEDIDO));
								close(fd_cli);
							}
						}
					}
					else{
						sprintf(fifo_nome,FIFO_CLI,pidtosend);
						p.remetente = getpid();
						p.tipo =1;
						p.valid = 0;
						mkfifo(fifo_nome,0600);
						fd_cli = open (fifo_nome,O_WRONLY);
						write(fd_cli, &p, sizeof(PEDIDO));
					}

					sprintf(fifo_nome,FIFO_CLI,pidtosend);
					p.remetente = getpid();
					p.tipo = 6;
					p.nPalavrasErradas=x;
					fd_cli = open (fifo_nome,O_WRONLY);
					n=write(fd_cli,&p,sizeof(PEDIDO));
					close(fd_cli);
				//	for(i=0;i<20;i++){
					//	memset( p.palavrasErradas,0,20*20);
						memset(p.palavrasErradas, 0, sizeof(p.palavrasErradas[0][0]) * 20 * 20);
				//	}




					break;
					case 5: //recebe char e manda para todos
					printf("Foi recebido %d bytes do pedido de broadcast char \n",n);
					printf("[broadcast] **char:%d[%d,%d]\t", p.carater,p.linhaPoxx,p.linhaPoxy);
					for(i=0;i<5;i++){
						if(usersOnline[i].userPid !=-1){
							sprintf(fifo_nome,FIFO_CLI,usersOnline[i].userPid);
							fd_cli = open(fifo_nome,O_WRONLY);
							n = write(fd_cli, &p, sizeof(PEDIDO));
							close(fd_cli);
							printf("[%d].",i);
						}
					}
					printf("\n");
					printf("[broadcast] **fim\n");
					break;
					default:
					fprintf(stderr, "[ERRO] tipo de pedido desconhecido\n");
					break;
				}
				//printf("Recebi %d bytes...[]\n",n);
				printf("Comando:" ); fflush(stdout);
			}
		}while(1);
		close(fd_ser);
		unlink(FIFO_SER);
		exit(0);
}
}
