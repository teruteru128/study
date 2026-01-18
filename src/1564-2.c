
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
#include <pthread.h>

#define MAX_THREADS 8
#define FIELDS_SIZE (BUFSIZ * 2)

typedef struct
{
    uint64_t *sieve;
    size_t loadbits;
    uint64_t offsetbits;
    char **numbers;
    int start_idx;
    int end_idx;
    const char *fdb_cookie;
} thread_data_t;

// FactorDBへのアップロード関数 (スレッドセーフ)
int upload_to_factordb(const char *number, long factor, const char *cookie)
{
    CURL *hnd = curl_easy_init();
    if (!hnd)
        return -1;

    char postfields[FIELDS_SIZE];
    snprintf(postfields, FIELDS_SIZE, "number=%s&factor=%ld", number, factor);

    curl_easy_setopt(hnd, CURLOPT_URL, "https://factordb.com/reportfactor.php");
    curl_easy_setopt(hnd, CURLOPT_COOKIE, cookie);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, postfields);
    curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(hnd, CURLOPT_FAILONERROR, 1L);

    int retry_count = 0;
    int max_retries = 3;
    CURLcode res;

    do
    {
        res = curl_easy_perform(hnd);
        if (res == CURLE_OK)
            break;

        fprintf(stderr, "[Thread] Upload failed (res=%d), retrying... (%d/%d)\n", res, retry_count + 1, max_retries);
        sleep(2 << retry_count); // 指数バックオフ
        retry_count++;
    } while (retry_count < max_retries);

    curl_easy_cleanup(hnd);
    return (res == CURLE_OK) ? 0 : -1;
}

// スレッド実行関数
void *worker(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;
    mpz_t p, factorial1564, diff;

    mpz_inits(p, factorial1564, diff, NULL);
    mpz_fac_ui(factorial1564, 1564);

    for (int i = data->start_idx; i < data->end_idx; i++)
    {
        mpz_set_str(p, data->numbers[i], 10);
        mpz_sub(diff, p, factorial1564);

        long factor = -1;
        for (size_t idx = 1; idx < data->loadbits; idx++)
        {
            // sieveのビットチェック
            if (((data->sieve[idx >> 6] >> (idx & 63)) & 1) == 0)
            {
                uint64_t divisor = (data->offsetbits + idx) * 2 + 1;
                if (mpz_divisible_ui_p(p, divisor))
                {
                    factor = divisor;
                    break;
                }
            }
        }

        if (factor != -1)
        {
            char *diff_str = mpz_get_str(NULL, 10, diff);
            fprintf(stderr, "[Found] (1564!) + %s is divisible by %ld\n", diff_str, factor);
            upload_to_factordb(data->numbers[i], factor, data->fdb_cookie);
            free(diff_str);
        }
        else
        {
            char *diff_str = mpz_get_str(NULL, 10, diff);
            fprintf(stderr, "[Skip] No factor found for diff: %s\n", diff_str);
            free(diff_str);
        }
    }

    mpz_clears(p, factorial1564, diff, NULL);
    return NULL;
}

/**
 * (1564!)+nを試し割りして割り切れたらfactordbにアップロードするプログラム
 */
int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <sieve_file> <factorials_file> [offsetbits]\n", argv[0]);
        return 1;
    }

    const char *fdb_session = getenv("FDB_SESSION_ID");
    if (!fdb_session)
    {
        fprintf(stderr, "Error: FDB_SESSION_ID is not set.\n");
        return 1;
    }
    char cookie[128];
    snprintf(cookie, sizeof(cookie), "fdbuser=%s", fdb_session);

    // ファイル読み込み
    FILE *sieve_fp = fopen(argv[1], "rb");
    FILE *fact_fp = fopen(argv[2], "r");
    if (!sieve_fp || !fact_fp)
    {
        perror("File opening failed");
        return 1;
    }

    uint64_t offsetbits = (argc >= 4) ? strtoull(argv[3], NULL, 10) : 0;
    uint64_t offsetbytes = ((offsetbits == 0 ? 0 : (offsetbits - 1) >> 6) + 1) << 3;

    // Sieveデータのロード (約120MB)
    size_t sieve_elements = 1562624 * 10;
    uint64_t *sieve = malloc(sieve_elements * sizeof(uint64_t));
    fseek(sieve_fp, 8 + offsetbytes, SEEK_SET);
    size_t read_elements = fread(sieve, sizeof(uint64_t), sieve_elements, sieve_fp);
    fclose(sieve_fp);

    for (size_t i = 0; i < read_elements; i++)
    {
        sieve[i] = be64toh(sieve[i]);
    }

    // 入力リストの読み込み
    char **numbers = NULL;
    char line[BUFSIZ];
    int count = 0;
    while (fgets(line, BUFSIZ, fact_fp))
    {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0)
            continue;
        numbers = realloc(numbers, sizeof(char *) * (count + 1));
        numbers[count++] = strdup(line);
    }
    fclose(fact_fp);

    // マルチスレッド設定
    curl_global_init(CURL_GLOBAL_ALL);
    int num_threads = (count < MAX_THREADS) ? count : MAX_THREADS;
    pthread_t threads[MAX_THREADS];
    thread_data_t t_data[MAX_THREADS];

    int batch_size = count / num_threads;
    for (int i = 0; i < num_threads; i++)
    {
        t_data[i].sieve = sieve;
        t_data[i].loadbits = read_elements * 64;
        t_data[i].offsetbits = offsetbits;
        t_data[i].numbers = numbers;
        t_data[i].fdb_cookie = cookie;
        t_data[i].start_idx = i * batch_size;
        t_data[i].end_idx = (i == num_threads - 1) ? count : (i + 1) * batch_size;

        pthread_create(&threads[i], NULL, worker, &t_data[i]);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // 後片付け
    for (int i = 0; i < count; i++)
        free(numbers[i]);
    free(numbers);
    free(sieve);
    curl_global_cleanup();

    return 0;
}
