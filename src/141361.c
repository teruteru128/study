
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <gmp.h>
#include <endian.h>

#define ELM 1024
#define BITS (ELM * 64)

int main(int argc, char const *argv[], char *envp[])
{
    if (argc < 3)
    {
        return 1;
    }
    const char *path = argv[1];
    FILE *in = fopen(path, "rb");
    if (in == NULL)
    {
        perror("fopen");
        return 1;
    }
    fseek(in, 8 + 8, SEEK_SET);
    uint64_t primes[ELM];
    mpz_t n;
    mpz_init(n);
    mpz_ui_pow_ui(n, 6, 141361);
    mpz_sub_ui(n, n, 6);
    mpz_t remove_factor;
    mpz_init(remove_factor);
    if (mpz_even_p(n))
    {
        // 偶数除け
        mpz_set_ui(remove_factor, 2);
        uint64_t cnt = mpz_remove(n, n, remove_factor);
        printf("div: %zu(%" PRIu64 ")\n", 2L, cnt);
    }
    size_t num = 64;
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
                if (mpz_divisible_ui_p(n, (num + index) * 2 + 1) != 0)
                {
                    mpz_set_ui(remove_factor, (num + index) * 2 + 1);
                    uint64_t cnt = mpz_remove(n, n, remove_factor);
                    printf("div: %zu(%" PRIu64 ")\n", (num + index) * 2 + 1, cnt);
                    // goto done;
                }
            }
        }
        num += bits;
        if (num >= note)
        {
            fprintf(stderr, "note: %zu\n", num);
            while (num >= note)
                note += 100000000ULL;
        }
    }
done:
    mpz_clear(n);
    fclose(in);
    mpz_clear(remove_factor);
    return 0;
}
