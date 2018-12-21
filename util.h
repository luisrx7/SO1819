#include <signal.h>

#define FIFO_SER "sss"
#define FIFO_CLI "ccc%d" //fifo de cliente


typedef struct{
int tipo; //=1 é login   =2 logout    =3 lockline    =4 unlockline
          // =5 é char
char username[9];
int valid;//0 nao é valido    1 é valido    2 ja em uso

int remetente;
char linha[45];
int linhaPoxy;
int linhaPoxx;
int carater;
}PEDIDO;

typedef struct{
int userPid;
char user[9];
int editinglineN;
}CLIENTE;
