#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <omp.h> // マルチスレッド用ライブラリ

#define ORDER 29
#define MAX_CANDIDATES 3
#define MAX_ATTEMPTS 2000
#define POP_SIZE 40          // 共有プールのサイズ（少し大きめに拡大）
#define MAX_DIFF 100000      // 差分マップのサイズ
#define THREAD_CYCLES 500000 // 各スレッドが同期するまでに回すサイクル数

typedef struct {
    int marks[ORDER];
    int length;
} Ruler;

// スレッド安全な乱数生成 (Xorshift)
unsigned int get_thread_rand(unsigned int* state) {
    unsigned int x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

// 差分チェック
bool is_valid_ruler(const int* marks, int count) {
    bool local_diff[MAX_DIFF] = {false};
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            int d = marks[j] - marks[i];
            if (d <= 0 || d >= MAX_DIFF || local_diff[d]) return false;
            local_diff[d] = true;
        }
    }
    return true;
}

// マーク配置の検証（安全ガード付き）
bool is_valid_mark(int next_val, const int* marks, int count, const bool* diff_map) {
    for (int i = 0; i < count; i++) {
        int d = next_val - marks[i];
        if (d <= 0 || d >= MAX_DIFF || diff_map[d]) return false;
    }
    return true;
}

// ランダム貪欲による定規生成（スレッド安全版）
bool generate_greedy_ruler(Ruler* ruler, int target_max, unsigned int* rand_state) {
    bool diff_map[MAX_DIFF] = {false}; // スレッドごとのローカルスタックに確保
    ruler->marks[0] = 0;
    int count = 1;

    while (count < ORDER) {
        int last_val = ruler->marks[count - 1];
        if (last_val + (ORDER - count) > target_max) return false;

        int candidates[MAX_CANDIDATES];
        int cand_count = 0;
        int check_val = last_val + 1;
        int attempts = 0;

        while (cand_count < MAX_CANDIDATES && attempts < MAX_ATTEMPTS) {
            if (is_valid_mark(check_val, ruler->marks, count, diff_map)) {
                if (count == 1 && check_val > 50) break; // 鏡像排除
                candidates[cand_count] = check_val;
                cand_count++;
            }
            check_val++;
            attempts++;
        }

        if (cand_count == 0) return false;

        int chosen_val = candidates[get_thread_rand(rand_state) % cand_count];
        for (int i = 0; i < count; i++) {
            int d = chosen_val - ruler->marks[i];
            if (d > 0 && d < MAX_DIFF) diff_map[d] = true;
        }
        ruler->marks[count] = chosen_val;
        count++;
    }
    
    ruler->length = ruler->marks[ORDER - 1];
    return (ruler->length <= target_max && is_valid_ruler(ruler->marks, ORDER));
}

// GA: 交叉＋突然変異
bool crossover_and_mutate(const Ruler* parentA, const Ruler* parentB, Ruler* child, int target_max, unsigned int* rand_state) {
    int cross_point = 10 + (get_thread_rand(rand_state) % 5); 
    for (int i = 0; i < cross_point; i++) child->marks[i] = parentA->marks[i];

    for (int i = cross_point; i < ORDER; i++) {
        int gap = parentB->marks[i] - parentB->marks[i - 1];
        if (get_thread_rand(rand_state) % 10 == 0) { 
            gap += (get_thread_rand(rand_state) % 3) - 1; // -1 〜 +1
            if (gap <= 0) gap = 1;
        }
        child->marks[i] = child->marks[i - 1] + gap;
    }

    child->length = child->marks[ORDER - 1];
    return (child->length <= target_max && is_valid_ruler(child->marks, ORDER));
}

// 構造体の比較関数（qsort用: 長さの昇順）
int compare_rulers(const void* a, const void* b) {
    return ((Ruler*)a)->length - ((Ruler*)b)->length;
}

int main() {
    srand((unsigned int)time(NULL));
    Ruler global_pool[POP_SIZE];
    int current_best = 99999;
    unsigned int main_seed = (unsigned int)rand();

    printf("== 29次ゴロム定規 超高速マルチスレッド・ハイブリッド探索 ==\n");
    int threads = omp_get_max_threads();
    printf("検出されたCPUコア（スレッド）数: %d\n", threads);
    printf("1. 初期プール（%d個体）を生成中...\n", POP_SIZE);

    int initialized = 0;
    while (initialized < POP_SIZE) {
        if (generate_greedy_ruler(&global_pool[initialized], 1500, &main_seed)) {
            if (global_pool[initialized].length < current_best) {
                current_best = global_pool[initialized].length;
            }
            initialized++;
            printf("\r初期個体生成: [%d / %d] (現在のベスト: %d)", initialized, POP_SIZE, current_best);
            fflush(stdout);
        }
    }
    qsort(global_pool, POP_SIZE, sizeof(Ruler), compare_rulers);

    printf("\n\n2. 並行進化探索スタート\n");
    unsigned long long total_generations = 0;

    while (1) {
        total_generations++;
        
        // 各スレッドがこの世代で発見したベスト定規を一時保存するバッファ
        Ruler thread_bests[threads];
        bool thread_found[threads];
        memset(thread_found, 0, sizeof(thread_found));

        // --- ここからマルチスレッド並行処理 ---
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            unsigned int thread_seed = (unsigned int)(time(NULL) ^ tid ^ total_generations);
            int local_best_len = current_best;
            Ruler local_best_ruler;
            bool found_any = false;

            // 各スレッドが指定サイクル分、自律的に回す
            for (int c = 0; c < THREAD_CYCLES; c++) {
                // 50%の確率で新規貪欲、50%の確率でGA進化を試みる
                if (get_thread_rand(&thread_seed) % 2 == 0) {
                    Ruler tmp;
                    if (generate_greedy_ruler(&tmp, local_best_len - 1, &thread_seed)) {
                        local_best_len = tmp.length;
                        local_best_ruler = tmp;
                        found_any = true;
                    }
                } else {
                    int pA = get_thread_rand(&thread_seed) % POP_SIZE;
                    int pB = get_thread_rand(&thread_seed) % POP_SIZE;
                    Ruler tmp;
                    if (crossover_and_mutate(&global_pool[pA], &global_pool[pB], &tmp, local_best_len - 1, &thread_seed)) {
                        local_best_len = tmp.length;
                        local_best_ruler = tmp;
                        found_any = true;
                    }
                }
            }

            // スレッドの結果を回収
            if (found_any) {
                thread_bests[tid] = local_best_ruler;
                thread_found[tid] = true;
            }
        }
        // --- マルチスレッド処理 ここまで ---

        // 各スレッドの成果を中央プールに集約（同期フェーズ）
        bool updated = false;
        for (int i = 0; i < threads; i++) {
            if (thread_found[i] && thread_bests[i].length < current_best) {
                current_best = thread_bests[i].length;
                // プールの最悪個体（配列の最後尾）を上書き
                global_pool[POP_SIZE - 1] = thread_bests[i];
                // 常に短い順にソートを維持
                qsort(global_pool, POP_SIZE, sizeof(Ruler), compare_rulers);
                updated = true;
            }
        }

        if (updated) {
            printf("\n【新記録達成！】 世代: %llu | 現在の最高記録(長さ): %d\n", total_generations, current_best);
            printf("マーク座標: ");
            for (int i = 0; i < ORDER; i++) printf("%d ", global_pool[0].marks[i]);
            printf("\n");
        } else {
            printf("\r世代: %llu | 現在の最高記録(長さ): %d を維持したまま次世代へ...", total_generations, current_best);
            fflush(stdout);
        }
    }

    return 0;
}

