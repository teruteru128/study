
#include <stdio.h>
#include <stdlib.h>
#include "string_list.h"

/*
  XXX strを追加するときにコピー(strdup)して追加するかそのまま追加するか
*/
string_list* string_list_add(string_list* dest, char* str){
  string_list* work;
  if(dest == NULL){
    // リストを新規作成
    work = malloc(sizeof(string_list));
    work->str = str;
    work->next = NULL;
    return work;
  }else{
    // 既存リストに文字列を追加
    work = dest;
    while(work->next != NULL){
      // search last node
      work = work->next;
    }
    work->next = malloc(sizeof(string_list));
    work->next->str = str;
    work->next->next = NULL;
    return dest;
  }
}

string_list* string_list_get_last_node(string_list* list){
  string_list* tmp = list;
  while(tmp->next != NULL){
    tmp = tmp->next;
  }
  return tmp;
}

size_t string_list_size(string_list* list){
  if(list == NULL){
    return 0;
  }
  string_list* tmp = list;
  size_t count = 1;
  while(tmp->next != NULL){
    count++;
    tmp = tmp->next;
  }
  return count;
}

string_list* str_split(char* str, char* delim){

}

void string_list_free(string_list* list){
  if(list == NULL){
    return;
  }
  string_list* tmp = list;
  while(tmp->next != NULL){
    printf("%s\n", tmp->str);
    //free(tmp->str);
    tmp->str = NULL;
    string_list* tmp_b = tmp; // 一つ前のノードを取る
    tmp = tmp->next;
    free(tmp_b);
  }
  printf("%s\n", tmp->str);
  //free(tmp->str);
  tmp->str = NULL;
  free(tmp);
}
