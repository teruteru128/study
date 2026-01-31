
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gmp.h>
#include <curl/curl.h>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t relsize = size * nmemb;
    fwrite(ptr, size, nmemb, stdout);
    fputs("\n", stdout);
    return relsize;
}

int main(int argc, char *argv[], char *envp[])
{
    if(argc < 2)
    {
        return 1;
    }
    const char *fdb_session = getenv("FDB_SESSION_ID");
    if(fdb_session == NULL)
    {
        fprintf(stderr, "Error: FDB_SESSION_ID is not set.\n");
        return 1;
    }
    char cookie[128];
    snprintf(cookie, 128, "fdbuser=%s", fdb_session);
    mpz_t p, factor1;
    mpz_init(p);
    mpz_init(factor1);
    mpz_ui_pow_ui(factor1, 2, 22);
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *hnd = curl_easy_init();
    char url_buffer[BUFSIZ];
    char strpbuffer[512];
    CURLcode res;
    long i = strtol(argv[1], NULL, 10);
    int retry_count = 0;
    int max_retries = 3;
    int p_count = 0;
    int max_p = 2000;
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(hnd, CURLOPT_COOKIE, cookie);
    curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(hnd, CURLOPT_FAILONERROR, 1L);
    while(p_count < max_p)
    {
        mpz_set(p, factor1);
        mpz_mul_ui(p, p, i);
        mpz_add_ui(p, p, 1);
        if(mpz_probab_prime_p(p, 24) != 0)
        {
            retry_count = 0;
            mpz_get_str(strpbuffer, 10, p);
            printf("%s is prime\n", strpbuffer);
            
            snprintf(url_buffer, BUFSIZ, "https://factordb.com/api?query=%s", strpbuffer);
            curl_easy_setopt(hnd, CURLOPT_URL, url_buffer);

            do{
                res = curl_easy_perform(hnd);
                if(res == CURLE_OK)
                {
                    break;
                }
                sleep(2 << retry_count);
                retry_count++;
            }while(retry_count < max_retries);
            p_count++;
        }
        i++;
    }
    curl_easy_cleanup(hnd);
    curl_global_cleanup();
    mpz_clear(p);
    return 0;
}
