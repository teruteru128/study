
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gmp.h>
#include <time.h>

int main(int argc, char *argv[], char *envp[])
{
	mpz_t p, term1, power_n;
	mpz_inits(p, term1, power_n, NULL);
	struct timespec start, finish, diff;
	int result;
	int n = 5000;
	while(n < 10000)
	{
		mpz_ui_pow_ui(power_n, 100, n);
		mpz_mul_ui(term1, power_n, 19000);
		mpz_add_ui(term1, term1, 22481);
		mpz_divexact_ui(p, term1, 99);
		clock_gettime(CLOCK_MONOTONIC, &start);
		result = mpz_probab_prime_p(p, 24);
		clock_gettime(CLOCK_MONOTONIC, &finish);
		if(finish.tv_nsec - start.tv_nsec < 0)
		{
			diff.tv_sec = finish.tv_sec - start.tv_sec - 1;
			diff.tv_nsec = finish.tv_nsec - start.tv_nsec + 1000000000;
		}else{
			diff.tv_sec = finish.tv_sec - start.tv_sec;
			diff.tv_nsec = finish.tv_nsec - start.tv_nsec;
		}
		if(result != 0)
		{
			printf("%d: prime\n", n);
		} else if(diff.tv_sec >= 1)
		{
			printf("%d: not prime(%ld.%09ld)\n", n, diff.tv_sec, diff.tv_nsec);
		}
		n++;
	}
	mpz_clears(p, term1, power_n, NULL);
	return 0;
}
