#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fila_thread.h"

//As threads serao identificadas por seu ID e tipo juntos

//Insere sequencialmente a thread na fila
int EntrouNaFila(char tipo, int id, ThreadFila *tf, int *i) {

  *i = *i+1;
    
  QueueThreadFila(tf, id, tipo);
    
  return 0;
}


//Checa se as threads estao saindo da fila corretamente, em relacao a ordem em que elas entraram e as normas de leitor/escritor
//Se a thread estiver saindo de forma errada o programa encerra com uma mensagem explicando o erro
//Se estiver saindo de forma correta, ela e removida da fila, o numero de threads ativas sera incrementado
// e uma mensagem de sucesso sera impressa
int SaiuDaFila(char tipo, int id, ThreadFila *tf, int* i, int *threads_leitoras_executando, int *thread_escritora_executando) {

  *i = *i+1;
 
  if(GetIdTopoDaFila(tf) != id || GetTipoTopoDaFila(tf) != tipo){
     printf("Erro na linha %d. Thread %d do tipo %c pulou fila!\n", *i, id, tipo);
     exit(-1);
  } else {
     DequeueThreadFila(tf);
     printf("Thread %d do tipo %c saiu na ordem correta, garantindo ausencia de inanicao.\n", id, tipo);
  }

  if(*thread_escritora_executando > 0) {
     printf("Erro na linha %d. Thread %d do tipo %c saiu da fila para executar quando ja tinha um escritor escrevendo.\n", *i, id, tipo);
     exit(-1);
  } else
  if(*threads_leitoras_executando > 0 && tipo == 'E') {
     printf("Erro na linha %d. Thread escritora %d saiu da fila para escrever quando havia outras threads executando.\n", *i, id);
     exit(-1);
  }

  if(tipo == 'L')
     *threads_leitoras_executando++;
  else
     *thread_escritora_executando++;
  
  return 0;
}

//Notifica que a thread terminou sua iteraçao, decrementando o numero de threads ativas
int Executou(char tipo, int id, ThreadFila *tf, int* i, int *threads_leitoras_executando, int *thread_escritora_executando) {

  *i = *i+1;

  if(tipo == 'L')
     *threads_leitoras_executando--;
  else
     *thread_escritora_executando--;

  return 0;
}
