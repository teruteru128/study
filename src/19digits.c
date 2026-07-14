
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gmp.h>
#include <curl/curl.h>

// 定数定義
#define POWER_OF_TWO 22       // 2 の累乗数 (2^22)
#define PRIME_TEST_REPS 24    // GMPの確率的素数判定の試行回数 (判定精度に影響)
#define MAX_RETRIES 3         // cURLリクエスト失敗時の最大リトライ回数
#define MAX_PRIMES_TO_FIND 2000 // 探索する素数の最大個数
#define CURL_TIMEOUT_SEC 30L  // cURLのタイムアウト時間（秒）

/**
 * cURLのデータ受信コールバック関数。
 * APIからのレスポンスを標準出力に書き出します。
 */
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t relsize = size * nmemb;
    fwrite(ptr, size, nmemb, stdout);
    fputs("\n", stdout);
    return relsize;
}

int main(int argc, char *argv[], char *envp[])
{
    // 引数チェック (探索開始用のインデックス i が必要)
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <starting_index>\n", argv[0]);
        return 1;
    }

    // 環境変数から FactorDB のセッションIDを取得
    const char *fdb_session = getenv("FDB_SESSION_ID");
    if (fdb_session == NULL)
    {
        fprintf(stderr, "Error: FDB_SESSION_ID environment variable is not set.\n");
        return 1;
    }

    // Cookieヘッダー用のバッファを作成
    char cookie[128];
    snprintf(cookie, sizeof(cookie), "fdbuser=%s", fdb_session);

    // GMP 変数の初期化
    mpz_t p, factor1;
    mpz_init(p);
    mpz_init(factor1);
    
    // factor1 = 2^22 を計算
    mpz_ui_pow_ui(factor1, 2, POWER_OF_TWO);

    // cURL の初期化
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *hnd = curl_easy_init();
    if (!hnd)
    {
        fprintf(stderr, "Error: Failed to initialize cURL.\n");
        mpz_clear(p);
        mpz_clear(factor1);
        curl_global_cleanup();
        return 1;
    }

    // cURL の基本オプションを設定
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(hnd, CURLOPT_COOKIE, cookie);
    curl_easy_setopt(hnd, CURLOPT_TIMEOUT, CURL_TIMEOUT_SEC);
    curl_easy_setopt(hnd, CURLOPT_FAILONERROR, 1L); // HTTPステータス 400以上をエラーとして扱う

    char url_buffer[BUFSIZ];
    CURLcode res;
    
    // コマンドライン引数からループの開始インデックス i を取得
    long i = strtol(argv[1], NULL, 10);
    
    int p_count = 0;       // 発見した素数のカウンタ
    int retry_count = 0;   // リトライ用カウンタ

    // 指定された個数の素数が見つかるまでループ
    while (p_count < MAX_PRIMES_TO_FIND)
    {
        // p = i * (2^22) + 1 を計算
        mpz_set(p, factor1);
        mpz_mul_ui(p, p, i);
        mpz_add_ui(p, p, 1);

        // 確率的素数判定を実行 (0を返さなければ素数、または高確率で素数)
        if (mpz_probab_prime_p(p, PRIME_TEST_REPS) != 0)
        {
            retry_count = 0;

            // 素数 p を10進数文字列に変換するためのバッファを動的に確保 (バッファオーバーフロー対策)
            // mpz_sizeinbase は指定した基数での桁数の近似値を返すため、終端文字分 (+2) を加えて確保
            size_t str_size = mpz_sizeinbase(p, 10) + 2;
            char *strpbuffer = malloc(str_size);
            if (strpbuffer == NULL)
            {
                fprintf(stderr, "Error: Memory allocation failed.\n");
                break;
            }
            mpz_get_str(strpbuffer, 10, p);

            printf("%s is prime\n", strpbuffer);

            // FactorDB API の URL を構築
            snprintf(url_buffer, sizeof(url_buffer), "https://factordb.com/api?query=%s", strpbuffer);
            curl_easy_setopt(hnd, CURLOPT_URL, url_buffer);

            // APIリクエストの実行 (失敗時は指数バックオフでリトライ)
            do
            {
                res = curl_easy_perform(hnd);
                if (res == CURLE_OK)
                {
                    break; // 送信成功
                }
                
                // 指数バックオフ: 2 << retry_count (2秒, 4秒, 8秒...) スリープ
                unsigned int sleep_time = 2 << retry_count;
                fprintf(stderr, "API request failed (%s). Retrying in %u seconds...\n", 
                        curl_easy_strerror(res), sleep_time);
                sleep(sleep_time);
                
                retry_count++;
            } while (retry_count < MAX_RETRIES);

            // 動的に確保したバッファを解放
            free(strpbuffer);

            // 発見した素数の数をカウントアップ
            p_count++;
        }
        
        // 次のインデックスへ
        i++;
    }

    // 後処理 (リソースの解放)
    curl_easy_cleanup(hnd);
    curl_global_cleanup();
    mpz_clear(p);
    mpz_clear(factor1);

    return 0;
}
