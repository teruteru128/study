
#include <stdio.h>
#include <stdlib.h>

void movingsum(){}
void max(){}

int maxmovingsum(int *array, const size_t datasize, const size_t windowsize){

}

int main(int argc, char* argv[]){
  int rawchunk[625];
  int movingsum[621];
  int maxtmp=0;
  srand(114514);
  int i=0;
  for(i = 0;i < 625; i++){
    rawchunk[i] = (rand() % 10) == 0;
  }
  /*
  for(i = 0;i < 625; i++){
    printf("%d", rawchunk[i]);
  }
  */
  // https://www.ei.fukui-nct.ac.jp/2018/06/05/moving-average-program/
  int bp=0;
#define WIDTH 5
  //const int winsize=5;
  int buf[WIDTH];
  int sum=0;
  int minz=-313;
  int maxz=312;
  int widz = maxz - minz;
  int z;
  int minx=-313;
  int maxx=312;
  int widx = maxx - minx;
  int x;
  int i=0;
  for(z = minz; z < maxz; z++){
    i = (z+widz)%WIDTH;
    sum = sum - buf[i];
    buf[i] = (rand()%10) == 0;

  }
  puts("");

  //
  return EXIT_SUCCESS;
}

