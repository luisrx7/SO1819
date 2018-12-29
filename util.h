#include <signal.h>
#include <time.h>

#define FIFO_SER "sss"
#define FIFO_CLI "ccc%d" //fifo de cliente


typedef struct{
int tipo; //=1 é login   =2 logout    =3 lockline    =4 unlockline
          // =5 é char  =6 palavrasErradas  =7 libertarlinha
char username[9];
int valid;//0 nao é valido    1 é valido    2 ja em uso

int remetente;
char linha[50];
int linhaPoxy;
int linhaPoxx;
int carater;
char palavrasErradas[20][45];
int nPalavrasErradas;
}PEDIDO;

typedef struct{
int userPid;
char user[9];
int editinglineN;
time_t seg;
}CLIENTE;
