#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/**
 * ナベアツbot 30万カウントダウンツール
 * 使い方: ./program <snowflake_id> [current_count] [options]
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "エラー: Snowflake IDを指定してください。\n");
        fprintf(stderr, "使い方: %s <snowflake_id> [current_count] [options]\n", argv[0]);
        fprintf(stderr, "オプション:\n");
        fprintf(stderr, "  --target=<num>, --target <num>       目標カウント (デフォルト: 300000)\n");
        fprintf(stderr, "  --seconds=<num>, --seconds <num>    1カウント辺りの秒数 (デフォルト: 5400)\n");
        fprintf(stderr, "  --verbose                           詳細情報を表示\n");
        return EXIT_FAILURE;
    }

    int64_t snowflake_id = 0;
    size_t current_count = 0;
    int positional_count = 0;

    // デフォルト値の設定
    long target = 300000;
    long seconds_per_count = 5400;
    int verbose = 0;

    // 引数解析ループ
    for (int i = 1; i < argc; i++) {
        // --- 1. --target の処理 (=あり/なし両対応) ---
        if (strncmp(argv[i], "--target=", strlen("--target=")) == 0) {
            target = strtol(argv[i] + strlen("--target="), NULL, 10);
        } 
        else if (strcmp(argv[i], "--target") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "エラー: --target の引数が不足しています!\n");
                return EXIT_FAILURE;
            }
            target = strtol(argv[i + 1], NULL, 10);
            i++;
        }
        // --- 2. --seconds の処理 (=あり/なし両対応) ---
        else if (strncmp(argv[i], "--seconds=", strlen("--seconds=")) == 0) {
            seconds_per_count = strtol(argv[i] + strlen("--seconds="), NULL, 10);
        }
        else if (strcmp(argv[i], "--seconds") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "エラー: --seconds の引数が不足しています!\n");
                return EXIT_FAILURE;
            }
            seconds_per_count = strtol(argv[i + 1], NULL, 10);
            i++;
        }
        // --- 3. --verbose の処理 ---
        else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        }
        // --- 4. 位置引数 (Snowflake ID, 現在のカウント) の処理 ---
        else {
            char *endptr;
            switch (positional_count) {
                case 0:
                    snowflake_id = (int64_t)strtoll(argv[i], &endptr, 10);
                    if (*endptr != '\0') {
                        fprintf(stderr, "エラー: 不正なSnowflake IDです: %s\n", argv[i]);
                        return EXIT_FAILURE;
                    }
                    break;
                case 1:
                    current_count = (size_t)strtoull(argv[i], &endptr, 10);
                    if (*endptr != '\0') {
                        fprintf(stderr, "エラー: 不正なカウント数です: %s\n", argv[i]);
                        return EXIT_FAILURE;
                    }
                    break;
                default:
                    // 想定外の余計な引数は無視、またはエラーにしても良い
                    break;
            }
            positional_count++;
        }
    }

    // Snowflake IDからタイムスタンプ（ミリ秒➔秒）を復元
    int64_t time_with_milli = (snowflake_id >> 22) + 1288834974657LL;
    time_t start_seconds = time_with_milli / 1000;

    // 目標時刻の計算
    if ((long)current_count >= target) {
        printf("既に目標の %ld カウントを超えています。\n", target);
        return EXIT_SUCCESS;
    }
    long remaining_counts = target - (long)current_count;
    time_t diff_seconds = (time_t)remaining_counts * seconds_per_count;
    time_t target_time = start_seconds + diff_seconds;

    // 【Verbose表示】計算の前提条件と日時のデバッグ表示
    if (verbose) {
        char start_time_str[64];
        char target_time_str[64];
        
        // time_t を人間が読める文字列に変換
        struct tm *tm_start = localtime(&start_seconds);
        strftime(start_time_str, sizeof(start_time_str), "%Y-%m-%d %H:%M:%S", tm_start);
        
        struct tm *tm_target = localtime(&target_time);
        strftime(target_time_str, sizeof(target_time_str), "%Y-%m-%d %H:%M:%S", tm_target);

        printf("[VERBOSE] --- 設定サマリー ---\n");
        printf("[VERBOSE] Snowflake ID: %lld\n", (long long)snowflake_id);
        printf("[VERBOSE] 基準ツイート日時: %s\n", start_time_str);
        printf("[VERBOSE] 現在のカウント: %zu\n", current_count);
        printf("[VERBOSE] 目標のカウント: %ld\n", target);
        printf("[VERBOSE] 残りカウント数: %ld\n", remaining_counts);
        printf("[VERBOSE] 1カウント辺り : %ld 秒\n", seconds_per_count);
        printf("[VERBOSE] 目標達成予定日時: %s\n", target_time_str);
        printf("[VERBOSE] ---------------------\n");
    }

    // カウントダウンループ
    double timediff = 0;
    while ((timediff = difftime(target_time, time(NULL))) > 0) {
        if (verbose) {
            // verbose の時は \r で上書きせず、毎秒のログとして改行して残す
            printf("[VERBOSE] 残り時間: %.0f秒 (%.9f counts)\n", timediff, timediff / seconds_per_count);
        } else {
            // 通常時は1行でプログレス表示
            printf("\r残りカウント: %.9f counts", timediff / seconds_per_count);
            fflush(stdout);
        }
        sleep(1);
    }

    if (!verbose) {
        printf("\r"); // 通常時の表示クリア用
    }
    printf("目標の %ld カウントに到達しました！\n", target);
    return EXIT_SUCCESS;
}

