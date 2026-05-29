#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define ORDER 29          // マーク数（29次）
#define MAX_CANDIDATES 5  // 次のマーク候補を何個までリストアップするか（ランダム性の幅）
#define MAX_ATTEMPTS 1000 // 1つの位置を探す際、どこまで座標を進めて諦めるか

// 差分が重複していないかチェックする関数
bool is_valid_mark(int next_val, int* marks, int count, const bool* diff_map, int max_diff) {
    for (int i = 0; i < count; i++) {
        int d = next_val - marks[i];
        // すでに存在する差分か、または範囲外（安全のため）なら無効
        if (d <= 0 || (d < max_diff && diff_map[d])) {
            return false;
        }
    }
    return true;
}

// 1回の構築に挑戦する関数（成功したらtrue）
bool try_build_golomb(int* marks, bool* diff_map, int max_diff) {
    // 最初のマークは固定
    marks[0] = 0;
    int count = 1;

    // 差分マップの初期化
    for (int i = 0; i < max_diff; i++) diff_map[i] = false;

    // マークを29個になるまで追加していく
    while (count < ORDER) {
        int last_val = marks[count - 1];
        int candidates[MAX_CANDIDATES];
        int cand_count = 0;

        // 次の座標候補を探す
        int check_val = last_val + 1;
        int attempts = 0;

        while (cand_count < MAX_CANDIDATES && attempts < MAX_ATTEMPTS) {
            if (is_valid_mark(check_val, marks, count, diff_map, max_diff)) {
                candidates[cand_count] = check_val;
                cand_count++;
            }
            check_val++;
            attempts++;
        }

        // 候補が1つも見つからなかった（手詰まり）場合は、この挑戦は失敗
        if (cand_count == 0) {
            return false;
        }

        // 見つかった候補の中からランダムに1つ選択（ここが貪欲法＋ランダムのキモ）
        int chosen_val = candidates[rand() % cand_count];

        // 選んだマークを確定し、新しい差分をマップに記録
        for (int i = 0; i < count; i++) {
            int d = chosen_val - marks[i];
            diff_map[d] = true;
        }
        marks[count] = chosen_val;
        count++;
    }

    return true; // 29個無事に配置完了
}

int main() {
    // 乱数の初期化
    srand((unsigned int)time(NULL));

    int marks[ORDER];
    // 29次の定規の長さはおおよそ500〜1000前後になるため、差分マップは大きめに確保
    int max_diff = 5000; 
    bool* diff_map = (bool*)malloc(sizeof(bool) * max_diff);

    printf("29次ゴロム定規（劣最適解）を探索中...\n");

    unsigned long long trials = 0;
    clock_t start_time = clock();

    while (1) {
        trials++;
        if (try_build_golomb(marks, diff_map, max_diff)) {
            // 成功した場合
            clock_t end_time = clock();
            double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;

            printf("\n成功しました！ (試行回数: %llu 回, かかった時間: %.2f 秒)\n", trials, duration);
            printf("定規の長さ (最長マーク): %d\n", marks[ORDER - 1]);
            printf("マーク配置:\n");
            for (int i = 0; i < ORDER; i++) {
                printf("%d ", marks[i]);
            }
            printf("\n");
            break;
        }

        // 進捗表示（10000回ごと）
        if (trials % 10000 == 0) {
            printf("\r探索試行数: %llu 回...", trials);
            fflush(stdout);
        }
    }

    free(diff_map);
    return 0;
}

