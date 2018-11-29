#include <signal.h>

#define FIFO_SER "sss"
#define FIFO_CLI "ccc%d" //fifo de cliente


typedef struct{
int tipo; //=1 é login   =2 é normal
  char username[8];
  int valid;//0 nao é valido    1 é valido    2 ja em uso

int remetente;
char linha[45];
int linhaPoxy;
}PEDIDO;
