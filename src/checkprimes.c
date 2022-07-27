
#include <gmp.h>
#include <stdio.h>

/**
 * @brief ファイルに書いた整数が素数かどうか判定する
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        return 1;
    }

    FILE *fin = fopen(argv[1], "r");
    if (fin == NULL)
    {
        perror("fopen");
        return 1;
    }
    char buf[BUFSIZ];
    mpz_t p;
    mpz_init(p);
    int result = 0;
    size_t size = 0;
    while (fgets(buf, BUFSIZ, fin) != NULL)
    {
        mpz_set_str(p, buf, 10);
        result = mpz_probab_prime_p(p, 10);
        size = mpz_sizeinbase(p, 2);
        gmp_printf("%Zd : %d, %zu\n", p, result, size);
    }
    fclose(fin);
    mpz_clear(p);
    return 0;
}
