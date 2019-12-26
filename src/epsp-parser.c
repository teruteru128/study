
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <regex.h>
#include "epsp-parser.h"

#define LINE_PATTERN "([[:digit:]]+) ([[:digit:]]+) ?(.+)?"

static regex_t* line_pattern = NULL;

void parser_init(void){
  if(line_pattern == NULL){
    line_pattern = malloc(sizeof(regex_t));
    if(line_pattern==NULL){
      perror("line_pattern malloc failed");
    }
    int errorcode = regcomp(line_pattern, LINE_PATTERN, REG_EXTENDED|REG_NEWLINE);
  }

}

void parser_free(void){
  if(line_pattern != NULL){
    regfree(line_pattern);
    line_pattern = NULL;
  }
}

/**
  failed : false(0)
success : true(1)
*/
static int parse_internal(epsp_packet* packet, const char* line){
  // 一応nullチェック
  if(packet == NULL){
    return 0;
  }
  // 作業用コピー copy_lineはfree
  char* copy_line = strdup(line);
  // 予め改行文字(CRLF)は削除する
  // 半角スペースで分割する
  char* code_str = strtok(copy_line, " ");
  char* hop_str = strtok(NULL, " ");
  char* data_str = strtok(NULL, " ");
  char* trash_str = strtok(NULL, " ");
  // 分割した結果0個以下または4個以上の場合失敗
  if(code_str == NULL){
    // 0個
    goto clean;
  }
  if(trash_str != NULL){
    // 4個以上
    goto clean;
  }

  // 初期値
  packet->code = -1;
  packet->hop_count = -1;
  packet->data = NULL;

  {
    char* catch = NULL;
    packet->code = strtol(code_str, &catch, 10);
    if(*catch != '\0'){
      goto clean;
    }
    if(hop_str != NULL){
      packet->hop_count = strtol(hop_str, &catch, 10);
      if(*catch != '\0'){
        goto clean;
      }
    }
  }
  if(data_str != NULL){
    // 本当は分割して配列として渡したい
    packet->data = strdup(data_str);
  }
  free(copy_line);
  return 1;

  /*
  regmatch_t match[4];
  size_t size = sizeof(match)/sizeof(regmatch_t);
  int errorcode = regexec(line_pattern, line, size, match, 0);
  if(errorcode != 0){
    return NULL;
  }
  // データをコード、ホップ数、その他データにパースする。失敗したらその時点で終了
  epsp_packet* packet = malloc(sizeof(epsp_packet));
  packet->code = strtol(line+match[1].rm_so, NULL, 0);
  packet->hop_count = strtol(line+match[2].rm_so, NULL, 0);
  if(match[3].rm_so == -1 || match[3].rm_eo == -1){
    packet->data = NULL;
  }else{
    size_t data_start = match[3].rm_so;
    size_t data_end = match[3].rm_eo;
    size_t data_len = data_end - data_start;
    packet->data = malloc(data_len + 1);
    memcpy(packet->data, line+data_start, data_len);
    packet->data[data_len] = 0;
  }
  */
clean:
  free(copy_line);
  return 0;
}

/**
    https://github.com/teruteru128/epsp-peer-cs/blob/master/Client/Common/Net/Packet.cs
*/
epsp_packet* parse_epsp_packet(char* line){
  if(line_pattern == NULL || line == NULL){
    return NULL;
  }

  // allocate
  // tryparse(call internal parser)
  // return packet OR Exception
  epsp_packet* packet = malloc(sizeof(epsp_packet));
  if(parse_internal(packet, line)){
  }else{
    //failed
    epsp_packet_free(packet);
    return NULL;
  }
}

/*
 * https://p2pquake.github.io/epsp-specifications/epsp-specifications.html
 */
void print_reg_error(int errorcode, regex_t* buf){
    size_t len = regerror(errorcode, buf, NULL, 0);
    char* msg = (char*)calloc(len, sizeof(char));
    regerror(errorcode, buf, msg, len);
    printf("%s\n", msg);
    free(msg);
}

/*
   max(strlen(src)+1, n)バイトの領域を新たに割り当てしディープコピー
   strdupを使うといい string.h
 */
char* strnclone(const char* src, const size_t n){
  char* dest = calloc(n+1, sizeof(char));
  return strncpy(dest, src, n);
}

/* strlen(src)+1バイトの領域を新たに割り当てしディープコピー */
char* strclone(const char* src){
  size_t len = strlen(src);
  return strnclone(src, len);
}

string_list* string_list_insert_after(string_list* list, string_node* node, string_node* new_node){
  new_node->prev=node;
  new_node->next = node->next;
  if(node->next == NULL){
    list->lastNode = new_node;
  }else{
    node->next->prev = new_node;
  }
  node->next = new_node;
  return list;
}

string_list* string_list_insert_before(string_list* list, string_node* node, string_node* new_node){
  new_node->prev=node->prev;
  new_node->next=node;
  if(node->prev==NULL){
    list->firstNode=new_node;
  }else{
    node->prev->next=new_node;
  }
  node->prev=new_node;
  return list;
}

string_list* string_list_insert_first(string_list* list, string_node* new_node){
  if(list->firstNode == NULL){
    list->firstNode=new_node;
    list->lastNode=new_node;
    new_node->prev=NULL;
    new_node->next=NULL;
  }else{
    string_list_insert_before(list, list->firstNode, new_node);
  }
  return list;
}

string_list* string_list_insert_last(string_list* list, string_node* new_node){
  if(list->lastNode==NULL){
    string_list_insert_first(list, new_node);
  }else{
    string_list_insert_after(list, list->lastNode, new_node);
  }
  return list;
}

string_list* string_list_remove(string_list* list, string_node* node){
  if(node->prev==NULL){
    list->firstNode = node->next;
  }else{
    node->prev->next = node->next;
  }
  if(node->next == NULL){
    list->lastNode=node->prev;
  }else{
    node->next->prev = node->prev;
  }
}

size_t string_list_length(string_list* a){
  if(a == NULL||a->firstNode==NULL){
    return 0;
  }
  string_node* b;
  size_t count = 1;
  for(b=a->firstNode;b->next!=NULL;b=b->next){
    count++;
  }
  return count;
}

/*
  文字列の配列を返す
  https://hg.openjdk.java.net/jdk/jdk12/file/06222165c35f/src/java.base/share/classes/java/lang/String.java#l2274
*/
/*
int split(string_array** dest, regex_t pattern, char* input, size_t limit){
}
*/

string_list* split(char* src, char* regex, int limit)
{
  char ch = 0;
  size_t regex_len = strlen(regex);
  if(((regex_len == 1 && strchr(".$|()[{^?*+\\", ch=regex[0])==NULL)||
  (regex_len == 2 && regex[0] =='\\' && (((ch=regex[1])-'0')|('9'-ch))<0&&((ch-'a')|('z'-ch)) < 0 &&((ch-'A')|('Z'-ch)) < 0))){
    int off=0;
    int next = 0;
    int limited = limit > 0;
    string_list* list = malloc(sizeof(string_list));
    while((next = regex[off]) != '\0'){
      if(!limited||string_list_size(list)<limit-1){
        char*s=malloc(next-off+1);
        strncpy(s, regex, next-off);
        s[next-off]=0;
        string_list_insert_last(list, s);
      }else{}
    }
  }
  return NULL;
}

int split_regex(string_array** dest, char* pattern, char* str, size_t limit){
}

int split_strtok(string_array** dest, char* delim, char* src, size_t limit){
}

/*
  プロトコルフォーマットに合致しているか検査しないといけないので面倒くさい可能性が高い
*/
int split_by_strtok(const char* str){
  char* target = strdup(str);

  free(target);
  return EXIT_FAILURE;
}

void free_string_array(string_array* str){
}
int split_by_regex(char* str, char* regex){
  regex_t regbuf;
  int errco=0;
  if((errco=regcomp(&regbuf, regex, REG_EXTENDED|REG_NEWLINE)) != 0){
    print_reg_error(errco, &regbuf);
    return 1;
  }

  regmatch_t match[8];
  size_t size = sizeof(match) / sizeof(regmatch_t);
  if((errco=regexec(&regbuf, str, size, match, 0)) != 0){
    print_reg_error(errco, &regbuf);
    regfree(&regbuf);
    return EXIT_FAILURE;
  }
  epsp_packet* packet = NULL;
  if(errco == REG_NOMATCH){
    //no match
    regfree(&regbuf);
    return 0;
  }
  packet = malloc(sizeof(epsp_packet));
  packet->code      = strtol(str + match[1].rm_so, NULL, 10);
  packet->hop_count = strtol(str + match[2].rm_so, NULL, 10);
  if(match[3].rm_so == -1 || match[3].rm_eo == -1){
    // not found
    packet->data = NULL;
  }else{
    size_t data_start = match[3].rm_so;
    size_t data_end = match[3].rm_eo;
    size_t data_len = data_end - data_start;
    packet->data = malloc(data_len + 1);
    memcpy(packet->data, str+data_start, data_len);
    packet->data[data_len] = 0;
  }
  free(packet->data);
  free(packet);

  for(size_t i = 0; i < size; i++){
    size_t start = match[i].rm_so;
    size_t end = match[i].rm_eo;
    if(start==-1||end==-1){
      fprintf(stderr, "no match : %ld\n", i);
      continue;
    }
    size_t len = end - start;
    printf("%ld : %ld, %ld , %ld: ", len, start, end, len);
    // substr ここから
    // strdupはない場合もある
    //char* group = strdup(str+start, len);
    // malloc + memset + strncpy_s 現環境にstrncpy_s無し
    //char* group = malloc(len+1);
    //memset(group, 0, len+1);
    //strncpy_s(group, len+1, str+start, len);
    // malloc + strncpy + null終端挿入
    //char* group = malloc(len+1);
    //strncpy(group, str+start, len);
    //group[len] = 0;
    // malloc + memcpy + null終端挿入
    char* group = malloc(len+1);
    memcpy(group, str+start, len);
    group[len] = 0;
    // substr ここまで
    printf("%s\n", group);
    free(group);
  }
  regfree(&regbuf);
  fprintf(stderr, "OK\n");
  return EXIT_SUCCESS;
}
