
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>



int main(int argc, char* argv[]){
	mpz_t number;
	mpz_t p;
	int answer = 0;
	mpz_init(number);
	mpz_init(p);
	mpz_set_str(number, "51515155212112155512544451215879794313484631643513515461313510654159642752672634875312153543513515153543513564564984646463564346842311254454685465132211546454881133115554645161516484849874321321348523202152305448405648254896348510549", 10);
	mpz_nextprime(p, number);
	answer = mpz_probab_prime_p(p, 25);
	mpz_out_str(stdout, 10, p);
	switch(answer)
	{
		case 0:
			puts("is not prime");
			break;
		case 1:
			puts("is probably prime");
			break;
		case 2:
			puts("is definitely prime");
			break;
	}
	return 0;
}
