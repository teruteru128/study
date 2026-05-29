#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define ORDER 29
#define MAX_CANDIDATES 3
#define MAX_ATTEMPTS 2000

// ターゲットを一度「1500」まで緩めます。これで確実に完成ルートに乗ります。
#define TARGET_MAX_LEN 1500 

// 高精度な乱数生成（rand()の32767の壁を突破）
int get_large_rand() {
    return (rand() << 15) | rand();
}

bool is_valid_mark(int next_val, int* marks, int count, const bool* diff_map, int max_diff) {
    for (int i = 0; i < count; i++) {
        int d = next_val - marks[i];
        if (d <= 0 || (d < max_diff && diff_map[d])) {
            return false;
        }
    }
    return true;
}

bool try_build_golomb(int* marks, bool* diff_map, int max_diff) {
    marks[0] = 0;
    int count = 1;

    for (int i = 0; i < max_diff; i++) diff_map[i] = false;

    while (count < ORDER) {
        int last_val = marks[count - 1];
        
        // 簡易下限チェック（緩めに設定）
        int remaining = ORDER - count;
        if (last_val + remaining * 2 > TARGET_MAX_LEN) {
            return false; 
        }

        int candidates[MAX_CANDIDATES];
        int cand_count = 0;
        int check_val = last_val + 1;
        int attempts = 0;

        while (cand_count < MAX_CANDIDATES && attempts < MAX_ATTEMPTS) {
            if (is_valid_mark(check_val, marks, count, diff_map, max_diff)) {
                // 鏡像排除
                if (count == 1 && check_val > (TARGET_MAX_LEN - check_val)) {
                    break; 
                }
                candidates[cand_count] = check_val;
                cand_count++;
            }
            check_val++;
            attempts++;
        }

        if (cand_count == 0) return false;

        // 高精度乱数で選択
        int chosen_val = candidates[get_large_rand() % cand_count];

        for (int i = 0; i < count; i++) {
            int d = chosen_val - marks[i];
            diff_map[d] = true;
        }
        marks[count] = chosen_val;
        count++;
    }
    return true;
}

int main() {
    srand((unsigned int)time(NULL));

    int marks[ORDER];
    int max_diff = 50000; 
    bool* diff_map = (bool*)malloc(sizeof(bool) * max_diff);

    printf("29次ゴロム定規 高速生成テスト（目標: %d 以下）\n", TARGET_MAX_LEN);

    unsigned long long trials = 0;
    clock_t start_time = clock();

    while (1) {
        trials++;
        if (try_build_golomb(marks, diff_map, max_diff)) {
            clock_t end_time = clock();
            printf("\n【成功】29次ゴロム定規が完成しました！(試行: %llu 回, 時間: %.2f 秒)\n", trials, (double)(end_time - start_time) / CLOCKS_PER_SEC);
            printf("長さ: %d\nマーク座標: ", marks[ORDER - 1]);
            for (int i = 0; i < ORDER; i++) printf("%d ", marks[i]);
            printf("\n");
            break;
        }

        if (trials % 10000 == 0) {
            printf("\r試行数: %llu 回...", trials);
            fflush(stdout);
        }
    }

    free(diff_map);
    return 0;
}

