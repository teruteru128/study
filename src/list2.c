
#include <stdio.h>
#include <stdlib.h>
#define NIL 100
typedef int infotype;

typedef struct item{
  infotype info;
  struct item *next;
  struct item *prev;
} *pointer;

pointer add_list(infotype x, pointer p){
  pointer q;
  q = calloc(1, sizeof(*q));

  if(q==NULL){
    perror("メモリ不足");
    exit(EXIT_FAILURE);
  }
  q->info = x;
  q->next = p;
  q->prev = NULL;
  return q;
}

void show_list(pointer p){
  while(p!=NULL){
    printf(" %d", p->info);
    p = p->next;
  }
  printf("\n");
}

pointer reverse_list(pointer p){
  pointer q = NULL;
  pointer t;

  while(p != NULL){
    t = p->next;
    p->next = p->prev;
    p->prev = t;
    p = t;
  }
  return q;
}

int main(void){
  infotype x;
  pointer head = NULL;
  pointer tail = NULL;

  for(x = 0; x <= 9;x++){
    head = add_list(x, head);
  }
  show_list(head);
  head = reverse_list(head);
  show_list(head);
  return EXIT_SUCCESS;
}

