
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PRIME_NUMBER 100000

/* 素数の一覧表。nPrimeNumber個が小さい順番に入って行く */
int primeNumber[MAX_PRIME_NUMBER];
int nPrimeNumber;
/* 素数かどうかを確認済みの最大の数 */
int primeNumberChecked;

/*
 * mが素数かどうか確認する
 * m/2以下の素数が全てprimeNumberテーブルに入っているものとする。
 */
int isPrimeNumber(int m)
{
  int i;
  for (i = 0; i < nPrimeNumber; i++)
  {
    if (primeNumber[i] > m / 2)
      return 1;
    if (m % primeNumber[i] == 0)
    {
      return 0;
    }
  }
  return 1;
}

/*
 * n以下の素数をprimeNumberテーブルに入れる。
 */
void generatePrimrNumbers(int n)
{
  int i;

  if (n <= primeNumberChecked)
  {
    /* すでに十数一覧を作成済みなのでなにもすることはない */
    return;
  }
  /* 足りない分の素数表を作成する */
  for (i = primeNumberChecked + 1; i <= n; i++)
  {
    if (isPrimeNumber(i))
    {
      if (nPrimeNumber >= MAX_PRIME_NUMBER)
      {
        printf("Oops, too many prime numbers.\n");
        exit(1);
      }
      primeNumber[nPrimeNumber] = i;
      nPrimeNumber++;
      //printf("%d: %d is a prime number\n", nPrimeNumber, i);
    }
  }
  primeNumberChecked = n;
  return;
}

/*
 * n以下の素数を数える
 */
int countPrimeNumbers(int n)
{
  int count, i;

  generatePrimrNumbers(n);
  count = 0;
  for (i = 0; i < nPrimeNumber; i++)
  {
    if (primeNumber[i] > n)
    {
      break;
    }
    count++;
  }
  return count;
}

void *threadFunc(void *arg)
{
  int n = (int)arg;
  int x;
  x = countPrimeNumbers(n);
  printf("Number of prime numbers under  %d is %d\n", n, x);
  return NULL;
}

int main()
{
  int numberList[6] = {1, 10, 100, 1000, 10000, 100000};
  pthread_t threads[6];
  int i;

  nPrimeNumber = 0;
  primeNumberChecked = 1;

  for (i = 0; i < 6; i++)
  {
    if (pthread_create(&threads[i], NULL, threadFunc, (void *)numberList[i]) != 0)
    {
      printf("Can't create thread (%d)\n", i);
      exit(1);
    }
  }

  for (i = 0; i < 6; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("Done\n");
  return 0;
}
