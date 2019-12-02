#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "fila_thread.h"

int n_leitores, n_escritores;
int n_leituras, n_escritas;
char nome_log[20];
int variavel_compartilhada = -1;

//Arquivo de log geral do programa
FILE *arquivo_log;

//Arquivos de log individuais das threads leitoras
FILE **arquivos_leitores;

pthread_mutex_t mutex_global;
pthread_cond_t cond_fila_diminuir, cond_area_compartilhada_esvaziar;

// Threads escritoras ou leitoras que estao lendo ou escrevendo
int escritor_ativo = 0;
int leitores_ativos = 0;

ThreadFila *fila_de_threads;

void *Escrever(void *arg){
  int id = *(int*)arg;

  for(int i=0; i<n_escritas; i++) {
    
    //Thread escritora entra na fila
    pthread_mutex_lock(&mutex_global);
    QueueThreadFila(fila_de_threads, id, 'E');
    arquivo_log = fopen(nome_log, "a");
    fprintf(arquivo_log, "EntrouNaFila('E',%d,tf, &i);\n", id);
    fclose(arquivo_log);    
    
    //Thread escritora checa se esta no topo da fila.Caso nao, espera o topo da fila sair para checar novamente.
    while(GetIdTopoDaFila(fila_de_threads) != id || GetTipoTopoDaFila(fila_de_threads) != 'E')
        pthread_cond_wait(&cond_fila_diminuir, &mutex_global);
   
    //Thread escritora checa se ha alguma thread ativa, ou seja, se ha leitura ou escrita ocorrendo.Caso esteja, espera que todas se encerrem.
    while(leitores_ativos > 0 || escritor_ativo > 0)
        pthread_cond_wait(&cond_area_compartilhada_esvaziar, &mutex_global);

    //Thread escritora sai da fila, dando o broadcast para que as outras da fila chequem se agora estao no topo e torna-se ativa.
    escritor_ativo++;
    DequeueThreadFila(fila_de_threads);
    arquivo_log = fopen(nome_log, "a");
    fprintf(arquivo_log, "SaiuDaFila('E',%d,tf, &i, &tle, &tee);\n", id);
    fclose(arquivo_log);   
    pthread_cond_broadcast(&cond_fila_diminuir);

    pthread_mutex_unlock(&mutex_global);
     

    
    //Thread escritora escreve
  
    variavel_compartilhada = id;

    //Thread escritora torna-se inativa e da o signal para a thread topo da fila checar se pode se ativar.
    pthread_mutex_lock(&mutex_global);
    arquivo_log = fopen(nome_log, "a");
    fprintf(arquivo_log, "Executou('E',%d,tf, &i, &tle, &tee);\n", id);
    fclose(arquivo_log);   
    escritor_ativo--;
    pthread_cond_signal(&cond_area_compartilhada_esvaziar);
    pthread_mutex_unlock(&mutex_global);

  }

  printf("Escritor %d finalizou a execucao.\n", id);
  pthread_exit(NULL);
}

void *Ler(void *arg){
  int id = *(int*)arg;

  char *nome_do_arquivo;
  nome_do_arquivo = malloc(sizeof(char)*9);
  sprintf(nome_do_arquivo, "leitor_%d.txt", id);

  arquivos_leitores[id] = fopen(nome_do_arquivo, "w");
 
  for(int i=0; i<n_leituras; i++) {

    
    pthread_mutex_lock(&mutex_global);
    
    //Thread leitora entra na fila
    QueueThreadFila(fila_de_threads, id, 'L');
    arquivo_log = fopen(nome_log, "a");
    fprintf(arquivo_log, "EntrouNaFila('L',%d,tf, &i);\n", id);
    fclose(arquivo_log);
    
    //Thread leitora checa se esta no topo da fila.Caso nao, espera o topo da fila sair para checar novamente.
    while(GetIdTopoDaFila(fila_de_threads) != id || GetTipoTopoDaFila(fila_de_threads) != 'L')
      pthread_cond_wait(&cond_fila_diminuir, &mutex_global);
    
    //Thread leitora checa se ha alguma thread ativa, ou seja, se ha leitura ou escrita ocorrendo.Caso esteja, espera que todas se encerrem.
    while(escritor_ativo > 0)
      pthread_cond_wait(&cond_area_compartilhada_esvaziar, &mutex_global);
      
    //Thread leitora sai da fila, dando o broadcast para que as outras da fila chequem se agora estao no topo e torna-se ativa.
    leitores_ativos++;
    DequeueThreadFila(fila_de_threads);
    arquivo_log = fopen(nome_log, "a");
    fprintf(arquivo_log, "SaiuDaFila('L',%d,tf, &i, &tle, &tee);\n", id);
    fclose(arquivo_log);
    pthread_cond_broadcast(&cond_fila_diminuir);


    pthread_mutex_unlock(&mutex_global);


    //Thread leitora le
    if(variavel_compartilhada == -1)
        i--;
    else
        fprintf(arquivos_leitores[id], "Foi lido o ID %d\n", variavel_compartilhada);
    
    //Thread leitora torna-se inativa e da o signal para a thread topo da fila checar se pode se ativar.
    pthread_mutex_lock(&mutex_global);
    arquivo_log = fopen(nome_log, "a");
    fprintf(arquivo_log, "Executou('L',%d,tf, &i, &tle, &tee);\n", id);
    fclose(arquivo_log);
    leitores_ativos--;
    if(leitores_ativos == 0) pthread_cond_signal(&cond_area_compartilhada_esvaziar);
    pthread_mutex_unlock(&mutex_global);

  }

  printf("Leitor %d finalizou a execucao.\n", id);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]){

  n_leitores    = atoi(argv[1]);
  n_escritores  = atoi(argv[2]);
  n_leituras    = atoi(argv[3]);
  n_escritas    = atoi(argv[4]);
  strcpy(nome_log, argv[5]);

  pthread_t tid_leitores[n_leitores];
  pthread_t tid_escritores[n_escritores];
  int id_leitores[n_leitores];
  int id_escritores[n_escritores];
 
  arquivos_leitores = malloc(sizeof(FILE*)*n_leitores);

  //Importacao das bibliotecas utilizadas pelo log,que e um programa em C
  arquivo_log = fopen(nome_log, "w");
  fprintf(arquivo_log, "#include <stdio.h>\n");  
  fprintf(arquivo_log, "#include <stdlib.h>\n");
  fprintf(arquivo_log, "#include \"funcoes_log.h\"\n\n");
  fprintf(arquivo_log, "int i = 0, tle = 0, tee = 0;\n\n");
  fprintf(arquivo_log, "int main(int argc, char *argv[]) {\n");
  fprintf(arquivo_log, "ThreadFila *tf = CreateThreadFila();\n");
  fclose(arquivo_log);

  fila_de_threads = CreateThreadFila();
  
  
  //Criacao das threads escritoras e leitoras
  for(int i=0; i<n_escritores; i++){
      id_escritores[i] = i;
      pthread_create(&tid_escritores[i], NULL, Escrever,(void*)&id_escritores[i]);
  }
  for(int i=0; i<n_leitores; i++){
      id_leitores[i] = i;
      pthread_create(&tid_leitores[i], NULL, Ler, (void*)&id_leitores[i]);
  }
  //Ṕrograma espera threads terminarem para encerrar o log
  for(int i=0; i<n_escritores; i++)
      pthread_join(tid_escritores[i], NULL);
  for(int i=0; i<n_leitores; i++)
      pthread_join(tid_leitores[i], NULL);

  arquivo_log = fopen(nome_log, "a");
  fprintf(arquivo_log, "printf(\"Programa concorrente esta respeitando as condicoes do problema Leitor/Escritor e nao possui inanicao de threads!\\n\");");
  fprintf(arquivo_log, "}\n");
  fclose(arquivo_log);

  DestroyThreadFila(fila_de_threads);
  free(arquivo_log);

  printf("Main finalizou a execucao.\n");
  pthread_exit(NULL);
}
