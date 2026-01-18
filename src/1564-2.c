
#include <gmp.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <curl/curl.h>

#define FIELDS_SIZE (BUFSIZ * 2)

/**
 * (1564!)+nを試し割りして割り切れたらfactordbにアップロードするプログラム
 */
int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "a\n");
        return 1;
    }
    char *fdbuser = getenv("FDB_SESSION_ID");
    if (fdbuser == NULL || strlen(fdbuser) == 0)
    {
        fprintf(stderr, "環境変数 FDB_SESSION_ID がセットされていません。\n");
        return 1;
    }
    curl_global_init(CURL_GLOBAL_ALL);
    FILE *sievefile = fopen(argv[1], "rb");
    FILE *factorials = fopen(argv[2], "r");
    if (sievefile == NULL || factorials == NULL)
    {
        perror("fopen");
        return 1;
    }
    uint64_t offsetbits = 0;
    uint64_t offsetbytes = 0;
    uint64_t offsetlongelements = 0;
    if (argc >= 4)
    {
        offsetbits = strtoull(argv[3], NULL, 10);
        offsetlongelements = ((offsetbits - 1) >> 6) + 1;
        offsetbytes = offsetlongelements << 3;
    }
    // 100,007,936 = 65536 * 1526 = 1562624 * 64, 65536bit単位で1億bitを超える最小データサイズ
    uint64_t *sieve = malloc(1562624 * sizeof(uint64_t) * 10);
    // 冒頭8バイトはこのファイルのエラトステネスのふるいに含まれるbit数(uint64_t, big endian)
    fseek(sievefile, 8 + offsetbytes, SEEK_SET);
    ssize_t elemetns = fread(sieve, sizeof(uint64_t), 1562624 * 10, sievefile);
    fclose(sievefile);
    for (int i = 0; i < elemetns; i++)
    {
        sieve[i] = be64toh(sieve[i]);
    }
    size_t loadbits = elemetns * 64;
    sievefile = NULL;
    mpz_t p, factorial1564, diff;
    mpz_init(p);
    mpz_init(factorial1564);
    mpz_init(diff);
    mpz_fac_ui(factorial1564, 1564);
    char line[BUFSIZ];
    int index;
    long factor;
    CURLcode ret;
    CURL *hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_URL, "https://factordb.com/reportfactor.php");
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(hnd, CURLOPT_CAINFO, "/usr/lib/ssl/cert.pem");
    curl_easy_setopt(hnd, CURLOPT_CAPATH, "/usr/lib/ssl/certs");
    curl_easy_setopt(hnd, CURLOPT_PROXY_CAPATH, "/usr/lib/ssl/certs");
    char cookie[64];
    snprintf(cookie, 64, "fdbuser=%s", fdbuser);
    curl_easy_setopt(hnd, CURLOPT_COOKIE, cookie);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, 30);
    curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 30);
    char postfields[FIELDS_SIZE];
    long http_code = 0;
    int retry_count = 0;
    int max_retries = 3;
    int sleep_time;
    while (fgets(line, BUFSIZ, factorials) != NULL)
    {
        factor = -1;
        // 改行消し
        line[strcspn(line, "\r\n")] = '\0';
        // セット
        mpz_set_str(p, line, 10);
        // 始点からの距離を計測
        mpz_sub(diff, p, factorial1564);
        // 試し割り
        for (index = 1; index < loadbits; index++)
        {
            if (((sieve[index >> 6] >> (index & 0x3f)) & 1) == 0)
            {
                if (mpz_divisible_ui_p(p, (offsetbits + index) * 2 + 1) != 0)
                {
                    factor = index * 2 + 1;
                    break;
                }
            }
        }
        if (factor != -1)
        {
            // 因数が見つかった
            fprintf(stderr, "(1564!) + %lu divides by: %lu\n", mpz_get_ui(diff), factor);
            snprintf(postfields, FIELDS_SIZE, "number=%s&factor=%ld", line, factor);
            curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, postfields);
            sleep_time = 5;
            do
            {
                ret = curl_easy_perform(hnd);
                if (ret != CURLE_OK)
                {
                    curl_easy_getinfo(hnd, CURLINFO_HTTP_CODE, &http_code);
                    if (ret == CURLE_OPERATION_TIMEDOUT || ret == CURLE_COULDNT_CONNECT || (http_code / 100) == 5)
                    {
                        retry_count++;
                        if (retry_count < max_retries)
                        {
                            fprintf(stderr, "Retrying in %d seconds...\n", sleep_time);
                            sleep(sleep_time); // 5秒待機
                            sleep_time *= 2;
                        }
                        else
                        {
                            fprintf(stderr, "Max retries reached. Aborting.\n");
                            break; // リトライ上限に達したらループを抜ける
                        }
                    }
                    else
                    {
                        // その他のエラーはリトライしない
                        break;
                    }
                }
                else
                {
                    // CURLE_OK
                    break;
                }
            } while (1);
        }
        else
        {
            fprintf(stderr, "見つかりませんでした。スキップします。: %lu\n", mpz_get_ui(diff));
        }
    }
    mpz_clear(diff);
    mpz_clear(factorial1564);
    mpz_clear(p);
    fclose(factorials);
    curl_easy_cleanup(hnd);
    curl_global_cleanup();
    return 0;
}
