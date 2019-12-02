typedef struct node_fila {
  struct node_fila * ant;
  int tid;
  char tipo;
} NodeFila;

typedef struct thread_fila {
  NodeFila * primeiro_da_fila;
  NodeFila * ultimo_da_fila;
} ThreadFila;

ThreadFila *CreateThreadFila() {
  ThreadFila *nova_fila = malloc(sizeof(ThreadFila));
  nova_fila->primeiro_da_fila = NULL;
  nova_fila->ultimo_da_fila = NULL;
  return nova_fila;
}

//Insere a thread na fila
int QueueThreadFila(ThreadFila * tf, int tid, char tipo) {

  NodeFila *novo_node;

  novo_node = malloc(sizeof(NodeFila));
  if(novo_node == NULL){
    printf("Erro no malloc da funcao QueueThreadFila. Novo node criado esta nulo.\n");
    exit(-1);  
  }

  novo_node->tid = tid;
  novo_node->tipo = tipo;
  novo_node->ant = NULL;

  if(tf->ultimo_da_fila != NULL)
    tf->ultimo_da_fila->ant = novo_node;
  tf->ultimo_da_fila = novo_node;

  if(tf->primeiro_da_fila == NULL)
    tf->primeiro_da_fila = novo_node;

  return 0;
}

//Remove a thread da fila
int DequeueThreadFila(ThreadFila * tf) {
  NodeFila *primeiro = tf->primeiro_da_fila;

  if(primeiro == NULL) {
    printf("Tentativa de remocao numa fila vazia.\n");
    return -1;
  }

  tf->primeiro_da_fila = tf->primeiro_da_fila->ant;
  if(primeiro == tf->ultimo_da_fila)
    tf->ultimo_da_fila = NULL;
  free(primeiro);

  return 0;
}

//Desaloca a memoria da fila
int DestroyThreadFila(ThreadFila * tf){

  NodeFila *nf = tf->primeiro_da_fila;
  while(nf != NULL){
    NodeFila * aux = tf->primeiro_da_fila->ant;
    free(nf);
    nf = aux;
  }
  free(tf);
  return 0;
}

//Retorna o id da thread no topo da fila
int GetIdTopoDaFila(ThreadFila * tf){
  if(tf->primeiro_da_fila == NULL)
    return -1;
  else
    return tf->primeiro_da_fila->tid;
}

//Retorna o tipo(leitora ou escritora) da thread no topo da fila
char GetTipoTopoDaFila(ThreadFila * tf){
  if(tf->primeiro_da_fila == NULL)
    return '\0';
  else
    return tf->primeiro_da_fila->tipo;
}
