
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[])
{
	mpz_t p, factor;
	mpz_inits(p, factor, NULL);
	mpz_set_ui(p, 114514);
	mpz_ui_pow_ui(factor, 10, 114508);
	mpz_mul(p, p, factor);
	mpz_nextprime(p, p);
	size_t length = mpz_sizeinbase(p, 10) + 2;
	char *str = malloc(length);
	mpz_get_str(str, 10, p);
	printf("%s\n", str);
	free(str);
	mpz_clear(p);
	return 0;
}
