
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
    // GMPの一時変数を初期化
    mpz_t n, n_cubed, numerator;
    mpz_init(n);
    mpz_init(n_cubed);
    mpz_init(numerator);
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
    // ループ開始インデックスの初期値を mpz_t にセット
    mpz_set_si(n, i);
    int p_count = 0;       // 発見した素数のカウンタ
    int retry_count = 0;   // リトライ用カウンタ

    // 指定された個数の素数が見つかるまでループ
    while (p_count < MAX_PRIMES_TO_FIND)
    {
        // 1. n^3 を計算
        mpz_pow_ui(n_cubed, n, 3);

        // 2. 分子 (numerator) = n^3 + 9 を計算
        mpz_add_ui(numerator, n_cubed, 9);

        // 3. 分子から 7*n を引く (numerator = n^3 - 7n + 9)
        mpz_submul_ui(numerator, n, 7);

        // 4. 数学的に必ず3で割り切れるため、divexact で高速に商を求める
        mpz_divexact_ui(p, numerator, 3);

        // p が 2 以上の正の数の場合のみ素数判定を行う
        if (mpz_cmp_si(p, 1) > 0)
        {
            // 確率的素数判定
            if (mpz_probab_prime_p(p, PRIME_TEST_REPS) != 0)
            {
                retry_count = 0;

                // 文字列バッファを動的に確保
                size_t str_size = mpz_sizeinbase(p, 10) + 2;
                char *strpbuffer = malloc(str_size);
                if (strpbuffer == NULL)
                {
                    fprintf(stderr, "Error: Memory allocation failed.\n");
                    break;
                }
                mpz_get_str(strpbuffer, 10, p);

                // n の値もデバッグ用に表示すると便利です
                gmp_printf("n = %Zd: %s is prime\n", n, strpbuffer);

                // FactorDB API の URL を構築して送信
                snprintf(url_buffer, sizeof(url_buffer), "https://factordb.com/api?query=%s", strpbuffer);
                curl_easy_setopt(hnd, CURLOPT_URL, url_buffer);

                do
                {
                    res = curl_easy_perform(hnd);
                    if (res == CURLE_OK)
                    {
                        break;
                    }
                    unsigned int sleep_time = 2 << retry_count;
                    sleep(sleep_time);
                    retry_count++;
                } while (retry_count < MAX_RETRIES);

                free(strpbuffer);
                // 2. リトライ上限に達した（送信に失敗した）場合の終了処理
                if (retry_count >= MAX_RETRIES)
                {
                    fprintf(stderr, "\n[ERROR] Failed to post prime to FactorDB after %d retries. Exiting...\n", MAX_RETRIES);
                    
                    // メモリリークを防ぐため、確保済みの全リソースをここで適切に解放します
                    curl_easy_cleanup(hnd);
                    curl_global_cleanup();
                    
                    mpz_clear(p);
                    mpz_clear(factor1);
                    mpz_clear(n);
                    mpz_clear(n_cubed);
                    mpz_clear(numerator);

                    return 1; // エラーコード 1 で即座に終了
                }
                p_count++;
            }
        }
        
        // n を 1 進める (n = n + 1)
        mpz_add_ui(n, n, 1);
    }

    // 後片付け（忘れずに解放）
    mpz_clear(n);
    mpz_clear(n_cubed);
    mpz_clear(numerator);
    // 後処理 (リソースの解放)
    curl_easy_cleanup(hnd);
    curl_global_cleanup();

    return 0;
}
