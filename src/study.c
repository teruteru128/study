
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
//#include "gettext.h"
#include "gettextsample.h"
#include "random.h"
#include <linux/random.h>
#define D_SIZE (1024)
#define ADDRESS_BITS_PER_WORD (6)
#define BITS_PER_WORD (1<<ADDRESS_BITS_PER_WORD)
#define BIT_INDEX_MASK (BITS_PER_WORD - 1)

// http://hg.openjdk.java.net/jdk/jdk/file/fb39a8d1d101/src/java.base/share/classes/java/util/BitSet.java
typedef struct bitset_t bitset;
typedef struct bitset_t {
	size_t wordsInUse;
	bool sizeIsSticky;
  size_t words_length;
	uint64_t* words;
}bitset;
void checkInvariants(bitset* set) {
        assert(set->wordsInUse == 0 || set->words[set->wordsInUse - 1] != 0);
        assert(set->wordsInUse >= 0 && set->wordsInUse <= set->words_length);
        assert(set->wordsInUse == set->words_length || set->words[set->wordsInUse] == 0);
    }
size_t wordIndex(size_t bitIndex){
	return bitIndex >> ADDRESS_BITS_PER_WORD;
}
static void ensureCapacity(bitset*set, size_t wordsRequired){
    if(set->words_length < wordsRequired){
      size_t request = 2 * (set->words_length) > (wordsRequired) ? (set->words_length) : (wordsRequired);
      uint64_t* tmp = realloc(set->words, sizeof(uint64_t)*request);
      if(tmp != NULL){
        memset(tmp+set->words_length, 0, sizeof(uint64_t)*(request-set->words_length));
        set->words_length=request;
        set->sizeIsSticky=false;
        set->words = tmp;
      }
    }
}
static void expandTo(bitset*set, size_t wordIndex){
  size_t wordsRequired = wordIndex+1;
  if(set->wordsInUse<wordsRequired){
    ensureCapacity(set, wordsRequired);
    set->wordsInUse = wordsRequired;
  }
}
bool bitset_get(bitset*set, size_t bitIndex){
	checkInvariants(set);
	size_t index = wordIndex(bitIndex);
	return (index < set->wordsInUse) && ((set->words[index] & (1L << bitIndex)) != 0);
}
static void initWords(bitset*set, size_t nbits){
  set->words_length = wordIndex(nbits-1)+1;
  set->words = malloc(sizeof(uint64_t)*set->words_length);
  memset(set->words, 0, sizeof(uint64_t)*set->words_length);
}
void bitset_init(bitset* set){
  set->wordsInUse = 0;
  initWords(set, BITS_PER_WORD);
  set->sizeIsSticky=false;
}
void bitset_init2(bitset* set, size_t nbits){
  if(nbits<0){
    return;
  }
  initWords(set, nbits);
  set->sizeIsSticky=true;
}
/*
bitset* bitset_alloc(){
  bitset* set = malloc(sizeof(bitset));
  memset(set, 0, sizeof(bitset));
  bitset_init(set);
  return set;
}
*/
void bitset_free(bitset* set){
  if(set->words!=NULL){
    free(set->words);
    set->words = NULL;
  }
}
void bitset_set(bitset*set, size_t bitIndex){
  size_t index = wordIndex(bitIndex);
  expandTo(set, index);
  set->words[index] |= (1L<<bitIndex);

  checkInvariants(set);
}

static void print_reg_error(int errorcode, regex_t* buf){
    size_t len = regerror(errorcode, buf, NULL, 0);
    char* msg = malloc(len);
    if(msg == NULL){
      perror("print_reg_error malloc");
      return;
    }
    regerror(errorcode, buf, msg, len);
    fprintf(stderr, "%s\n", msg);
    free(msg);
}

static const char *cmdline_ops[]={
  "orz",
  "yattaze",
  "dappun",
  "Nabeatsu",
  "FizzBuzz",
  "uuidtest",
  "ankokudan_decode",
  NULL
};

int main(int argc, char* argv[]){
  // alloc/free か init/clear のどっちかにしたい
  bitset set;
  bitset_init(&set);
  bitset_set(&set, 19937);
  printf("%d\n", bitset_get(&set, 19937));
  bitset_free(&set);

/*
  char* p = " ";
  regex_t ptn;
  int errorcode = regcomp(&ptn, p, REG_EXTENDED|REG_NEWLINE);
  if(errorcode != 0){
    print_reg_error(errorcode, &ptn);
    return EXIT_FAILURE;
  }

  regmatch_t match[5];
  size_t size = sizeof(match)/ sizeof(regmatch_t);

  char* string = "111111 22 333";

  errorcode = regexec(&ptn, string, size, match, 0);
  if(errorcode != 0){
    print_reg_error(errorcode, &ptn);
    return EXIT_FAILURE;
  }

  printf("match[0] -> so : %d, eo : %d\n", match[0].rm_so, match[0].rm_eo);
  int i = 0;
  for(;i < 5;i++){
    printf("%d, %d\n", match[i].rm_so, match[i].rm_eo);
  }
  if(match[1].rm_so == -1 || match[4].rm_so != -1){
    fputs("packet failed", stderr);
  }
  printf("%s\n", &string[match[1].rm_so]);
  printf("%s\n", &string[match[2].rm_so]);
  printf("%s\n", &string[match[3].rm_so]);
*/
  return EXIT_SUCCESS;
}

