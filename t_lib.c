#include "t_lib.h"

TCB_Queue readyQueue;
TCB_Queue runningQueue;

void t_yield()
{
  if (readyQueue.head != NULL){
    tcb *tmp = runningQueue.head;
    addThread_ToReadyQueue(tmp);

    runningQueue.head = readyQueue.head;

    readyQueue.head = readyQueue.head->next;

    swapcontext(tmp->thread_context, runningQueue.head->thread_context);
  }
}

void t_init()
{
  tcb *tmp = (tcb *) malloc(sizeof(tcb));
  tmp->thread_id = -1;
  tmp->thread_priority = 1;
  tmp->thread_context = (ucontext_t *) malloc(sizeof(ucontext_t));
  tmp->next = NULL;

  getcontext(tmp->thread_context);

  addThread_ToRunningQueue(tmp);
}

int t_create(void (*fct)(int), int id, int pri)
{
  size_t sz = 0x10000;

  ucontext_t *uc;
  uc = (ucontext_t *) malloc(sizeof(ucontext_t));

  getcontext(uc);
/***
  uc->uc_stack.ss_sp = mmap(0, sz,
       PROT_READ | PROT_WRITE | PROT_EXEC,
       MAP_PRIVATE | MAP_ANON, -1, 0);
***/
  uc->uc_stack.ss_sp = malloc(sz);  /* new statement */
  uc->uc_stack.ss_size = sz;
  uc->uc_stack.ss_flags = 0;
  uc->uc_link = runningQueue.tail->thread_context;
  makecontext(uc, (void (*)(void)) fct, 1, id);

  tcb *tmp = (tcb *) malloc(sizeof(tcb));
  tmp->thread_id = id;
  tmp->thread_priority = 1;
  tmp->thread_context = uc;
  tmp->next = NULL;

  addThread_ToReadyQueue(tmp);
}

void t_shutdown(){
  if (runningQueue.head != NULL){
    free(runningQueue.head->thread_context->uc_stack.ss_sp);
    free(runningQueue.head->thread_context);
    free(runningQueue.head);
  }
  while (readyQueue.head != NULL){
    tcb *tmp = readyQueue.head;
    readyQueue.head = readyQueue.head->next;
    free(tmp->thread_context->uc_stack.ss_sp);
    free(tmp->thread_context);
    free(tmp);
    }
}

void t_terminate(){
  tcb *tmp = runningQueue.head;
  free(tmp->thread_context->uc_stack.ss_sp);
  free(tmp->thread_context);
  free(tmp);
  runningQueue.head = readyQueue.head;
  readyQueue.head = readyQueue.head->next;

  //printf("head id %d\n", readyQueue.head->thread_id);
  setcontext(runningQueue.head->thread_context);
}

int addThread_ToReadyQueue(tcb *t){
  if (readyQueue.head == NULL){
    //printf("[ready Queue is empty. adding %d.]\n", tcb->thread_id);
    t->next = NULL;
    readyQueue.head = t;
    readyQueue.tail = t;
  }
  else {
    //printf("[Adding %d to end of ready queue.]\n", tcb->thread_id);
    readyQueue.tail->next = t;
    readyQueue.tail = t;
    readyQueue.tail->next = NULL;
  }
}

int addThread_ToRunningQueue(tcb *t){
  if (runningQueue.head == NULL){
    t->next = NULL;
    runningQueue.head = t;
    runningQueue.tail = t;
  }
  else {
    runningQueue.tail->next = t;
    runningQueue.tail = t;
    runningQueue.tail->next = NULL;
  }
}

int sem_init(sem_t **sp, int sem_count)
{
  *sp = malloc(sizeof(sem_t));
  (*sp)->count = sem_count;
  (*sp)->q = NULL;
}

int addThread_ToSemQueue(sem_t *s, tcb *t){
  if (s->q == NULL){
    s->q = t;
  }
  else {
    tcb *tmp;
    tmp = s->q;
    while (tmp->next != NULL){
      tmp = tmp->next;
    }
    tmp->next = t;
    t->next = NULL;
  }
}

void sem_wait(sem_t *s){
  sigrelse(SIGINT);
  s->count--;
  if (s->count < 0){
    addThread_ToSemQueue(s, runningQueue.head);

    tcb *tmp = runningQueue.head;
    runningQueue.head = readyQueue.head;
    readyQueue.head = readyQueue.head->next;
    swapcontext(tmp->thread_context, runningQueue.head->thread_context);

    sighold(SIGINT);
  }
  else {
    sighold(SIGINT);
  }
}

void sem_signal(sem_t *s){
  sigrelse(SIGINT);
  s->count++;
  if (s->count <= 0){
    tcb *tmp = s->q;
    s->q = s->q->next;
    addThread_ToReadyQueue(tmp);
  }
  sighold(SIGINT);
}

void sem_destroy(sem_t **s){

}
