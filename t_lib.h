/*
 * types used by thread library
 */
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <signal.h>

struct tcb
{
  int thread_id;
  int thread_priority;
	ucontext_t  *thread_context;
	struct tcb *next;
};
typedef struct tcb tcb;

struct TCB_Queue{
	tcb *head;
	tcb *tail;
};
typedef struct TCB_Queue TCB_Queue;

typedef struct {
  int count;
  tcb *q;
} sem_t;

int addThread_ToReadyQueue(tcb *tcb);
int addThread_ToRunningQueue(tcb *tcb);
