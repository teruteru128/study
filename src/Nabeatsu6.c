#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define VERSION "1.1.0"

// ヘルプメッセージを表示する関数
void print_help(const char *prog_name) {
    printf("使い方: %s <snowflake_id> [current_count] [オプション]\n\n", prog_name);
    printf("引数:\n");
    printf("  <snowflake_id>      Twitter/X の Snowflake ID（必須）\n");
    printf("  [current_count]     現在のカウント数（任意、デフォルト: 0）\n\n");
    printf("オプション:\n");
    printf("  -t, --target=<num>  目標カウントを指定 (デフォルト: 300000)\n");
    printf("  -s, --seconds=<num> 1カウント辺りの秒数を指定 (デフォルト: 5400)\n");
    printf("  -v, --verbose       詳細なデバッグ情報を表示\n");
    printf("  --help              このヘルプを表示して終了\n");
    printf("  --version           バージョン情報を表示して終了\n\n");
    printf("例:\n");
    printf("  %s 1234567890 299900 --target 300000 --verbose\n", prog_name);
}

int main(int argc, char *argv[]) {
    // 引数がない、またはヘルプのみを求めている場合の先行処理
    if (argc < 2) {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    int64_t snowflake_id = 0;
    size_t current_count = 0;
    int positional_count = 0;

    // デフォルト値
    long target = 300000;
    long seconds_per_count = 5400;
    int verbose = 0;

    // 引数解析ループ
    for (int i = 1; i < argc; i++) {
        // --- 1. ヘルプとバージョン ---
        if (strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }
        else if (strcmp(argv[i], "--version") == 0) {
            printf("ナベアツbot カウントダウンツール v%s\n", VERSION);
            return EXIT_SUCCESS;
        }
        // --- 2. --target / -t の処理 ---
        else if (strncmp(argv[i], "--target=", strlen("--target=")) == 0) {
            target = strtol(argv[i] + strlen("--target="), NULL, 10);
        }
        else if (strcmp(argv[i], "--target") == 0 || strcmp(argv[i], "-t") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "エラー: 引数 '%s' に値を指定してください。\n", argv[i]);
                return EXIT_FAILURE;
            }
            target = strtol(argv[i + 1], NULL, 10);
            i++;
        }
        else if (strncmp(argv[i], "-t", 2) == 0 && strlen(argv[i]) > 2) {
            // "-t300000" のようにスペースなしで結合されているパターン
            target = strtol(argv[i] + 2, NULL, 10);
        }
        // --- 3. --seconds / -s の処理 ---
        else if (strncmp(argv[i], "--seconds=", strlen("--seconds=")) == 0) {
            seconds_per_count = strtol(argv[i] + strlen("--seconds="), NULL, 10);
        }
        else if (strcmp(argv[i], "--seconds") == 0 || strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "エラー: 引数 '%s' に値を指定してください。\n", argv[i]);
                return EXIT_FAILURE;
            }
            seconds_per_count = strtol(argv[i + 1], NULL, 10);
            i++;
        }
        else if (strncmp(argv[i], "-s", 2) == 0 && strlen(argv[i]) > 2) {
            // "-s5400" のようにスペースなしで結合されているパターン
            seconds_per_count = strtol(argv[i] + 2, NULL, 10);
        }
        // --- 4. --verbose / -v の処理 ---
        else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        }
        // --- 5. 先頭がハイフンで始まる未知のオプション ---
        else if (argv[i][0] == '-') {
            fprintf(stderr, "エラー: 未知のオプションです: %s\n", argv[i]);
            fprintf(stderr, "'%s --help' で利用可能なオプションを確認してください。\n", argv[0]);
            return EXIT_FAILURE;
        }
        // --- 6. 位置引数 (Snowflake ID, 現在のカウント) ---
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
                    fprintf(stderr, "警告: 余分な引数は無視されます: %s\n", argv[i]);
                    break;
            }
            positional_count++;
        }
    }

    // 必須である Snowflake ID が入力されなかった場合のチェック
    if (positional_count < 1) {
        fprintf(stderr, "エラー: Snowflake ID（位置引数の1番目）は必須です。\n");
        return EXIT_FAILURE;
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
        
        struct tm *tm_start = localtime(&start_seconds);
        if (tm_start) strftime(start_time_str, sizeof(start_time_str), "%Y-%m-%d %H:%M:%S", tm_start);
        
        struct tm *tm_target = localtime(&target_time);
        if (tm_target) strftime(target_time_str, sizeof(target_time_str), "%Y-%m-%d %H:%M:%S", tm_target);

        printf("[VERBOSE] --- 設定サマリー ---\n");
        printf("[VERBOSE] Snowflake ID: %lld\n", (long long)snowflake_id);
        printf("[VERBOSE] 基準ツイート日時: %s\n", tm_start ? start_time_str : "変換エラー");
        printf("[VERBOSE] 現在のカウント: %zu\n", current_count);
        printf("[VERBOSE] 目標のカウント: %ld\n", target);
        printf("[VERBOSE] 残りカウント数: %ld\n", remaining_counts);
        printf("[VERBOSE] 1カウント辺り : %ld 秒\n", seconds_per_count);
        printf("[VERBOSE] 目標達成予定日時: %s\n", tm_target ? target_time_str : "変換エラー");
        printf("[VERBOSE] ---------------------\n");
    }

    // カウントダウンループ
    double timediff = 0;
    while ((timediff = difftime(target_time, time(NULL))) > 0) {
        if (verbose) {
            printf("[VERBOSE] 残り時間: %.0f秒 (%.9f counts)\n", timediff, timediff / seconds_per_count);
        } else {
            printf("\r残りカウント: %.9f counts", timediff / seconds_per_count);
            fflush(stdout);
        }
        sleep(1);
    }

    if (!verbose) {
        printf("\r"); 
    }
    printf("目標の %ld カウントに到達しました！\n", target);
    return EXIT_SUCCESS;
}

