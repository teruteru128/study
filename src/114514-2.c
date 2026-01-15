
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <stdint.h>
#include <endian.h>

#define ELM 1024
#define BITS (ELM * 64)

int main(int argc, char *argv[], char *envp[])
{
	if (argc < 3)
	{
		return 1;
	}
	char *path = argv[1];
	FILE *in = fopen(path, "rb");
	if (in == NULL)
	{
		perror("fopen");
		return 1;
	}
	uint64_t primes[ELM];
	fseek(in, 8, SEEK_SET);
	mpz_t p, power_n;
	mpz_inits(p, power_n, NULL);
	mpz_set_ui(p, 114514);
	mpz_ui_pow_ui(power_n, 10, 114508);
	mpz_mul(p, p, power_n);
	mpz_add_ui(p, p, strtoull(argv[2], NULL, 10));
	size_t num = 0;
	size_t note = 100000000ULL;
	while (num < 137438953280ULL)
	{
		size_t len = fread(primes, sizeof(uint64_t), ELM, in);
		for (int i = 0; i < len; i++)
		{
			primes[i] = be64toh(primes[i]);
		}
		size_t bits = len * 64;
		for (int index = 0; index < bits; index++)
		{
			if (((primes[index >> 6] >> (index & 0x3f)) & 1) == 0)
			{
				if (mpz_divisible_ui_p(p, (num + index) * 2 + 1) != 0)
				{
					printf("div: %zu\n", (num + index) * 2 + 1);
					goto done;
				}
			}
		}
		num += bits;
		if(num >= note)
		{
			fprintf(stderr, "note: %zu\n", num);
			note += 100000000ULL;
		}
	}
done:
	fclose(in);
	// mpz_nextprime(p, p);
	size_t length = mpz_sizeinbase(p, 10) + 2;
	char *str = malloc(length);
	mpz_get_str(str, 10, p);
	// printf("%s\n", str);
	free(str);
	mpz_clears(p, power_n, NULL);
	return 0;
}
