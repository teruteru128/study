
#include <stdio.h>
#include <string.h>

void showFizzBuzz(){
  int n;
  int tmp;
  for(n=1; n <= 40; n++){
    if(n % 3 == 0 && n % 5 == 0){
      printf("Fizz Buzz\n");
    }else if(n%3 == 0){
      printf("Fizz\n");
    }else if(n%5==0){
      printf("Buzz\n");
    }else{
      printf("%d\n", n);
    }
  }
}

void showNabeatsu(){
}

int main(int argc, char* argv[]){
  if(strstr(argv[0], "Nabeatsu")){
    showNabeatsu();
  }else if(strstr(argv[0], "FizzBuzz")){
    showFizzBuzz();
  }
}

