#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ORDER 29
#define MAX_CANDIDATES 3
#define MAX_ATTEMPTS 2000
#define POP_SIZE 40          
#define MAX_DIFF 100000      
#define THREAD_CYCLES 500000 
#define SAVE_FILE "golomb_pool.dat"
#define STAGNANT_LIMIT 100  // 【新設定】何世代更新がなかったら大手術を行うか

typedef struct {
    int marks[ORDER];
    int length;
} Ruler;

// Xorshift乱数
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

// マーク配置の検証
bool is_valid_mark(int next_val, const int* marks, int count, const bool* diff_map) {
    for (int i = 0; i < count; i++) {
        int d = next_val - marks[i];
        if (d <= 0 || d >= MAX_DIFF || diff_map[d]) return false;
    }
    return true;
}

// ランダム貪欲による定規生成（途中からの再構築にも対応）
bool generate_greedy_ruler_from(Ruler* ruler, int start_count, int target_max, unsigned int* rand_state) {
    bool diff_map[MAX_DIFF] = {false}; 
    
    // start_countまでの既存の差分をマップに登録
    for (int i = 0; i < start_count; i++) {
        for (int j = i + 1; j < start_count; j++) {
            int d = ruler->marks[j] - ruler->marks[i];
            if (d > 0 && d < MAX_DIFF) diff_map[d] = true;
        }
    }

    int count = start_count;
    while (count < ORDER) {
        int last_val = ruler->marks[count - 1];
        if (last_val + (ORDER - count) > target_max) return false;

        int candidates[MAX_CANDIDATES];
        int cand_count = 0;
        int check_val = last_val + 1;
        int attempts = 0;

        while (cand_count < MAX_CANDIDATES && attempts < MAX_ATTEMPTS) {
            if (is_valid_mark(check_val, ruler->marks, count, diff_map)) {
                if (count == 1 && check_val > 50) break; 
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
            gap += (get_thread_rand(rand_state) % 3) - 1; 
            if (gap <= 0) gap = 1;
        }
        child->marks[i] = child->marks[i - 1] + gap;
    }

    child->length = child->marks[ORDER - 1];
    return (child->length <= target_max && is_valid_ruler(child->marks, ORDER));
}

int compare_rulers(const void* a, const void* b) {
    return ((Ruler*)a)->length - ((Ruler*)b)->length;
}

void save_pool(const Ruler* pool) {
    FILE* fp = fopen(SAVE_FILE, "wb");
    if (fp == NULL) return;
    fwrite(pool, sizeof(Ruler), POP_SIZE, fp);
    fclose(fp);
}

bool load_pool(Ruler* pool) {
    FILE* fp = fopen(SAVE_FILE, "rb");
    if (fp == NULL) return false;
    size_t read_cnt = fread(pool, sizeof(Ruler), POP_SIZE, fp);
    fclose(fp);
    return (read_cnt == POP_SIZE);
}

int main() {
    srand((unsigned int)time(NULL));
    Ruler global_pool[POP_SIZE];
    int current_best = 99999;
    unsigned int main_seed = (unsigned int)rand();

    printf("== 29次ゴロム定規 超高速マルチスレッド・並行進化（大手術機能付き） ==\n");
    int threads = omp_get_max_threads();
    printf("使用スレッド数: %d\n", threads);

    if (load_pool(global_pool)) {
        qsort(global_pool, POP_SIZE, sizeof(Ruler), compare_rulers);
        current_best = global_pool[0].length;
        printf("1. セーブデータを読み込みました。 (ベスト長さ: %d)\n", current_best);
    } else {
        printf("1. 初期プールを新規生成中...\n");
        int initialized = 0;
        while (initialized < POP_SIZE) {
            if (generate_greedy_ruler_from(&global_pool[initialized], 1, 1500, &main_seed)) {
                if (global_pool[initialized].length < current_best) {
                    current_best = global_pool[initialized].length;
                }
                initialized++;
            }
        }
        qsort(global_pool, POP_SIZE, sizeof(Ruler), compare_rulers);
        save_pool(global_pool);
    }

    printf("\n2. 並行進化探索スタート\n");
    unsigned long long total_generations = 0;
    int stagnant_generations = 0; // 停滞している世代数

    while (1) {
        total_generations++;
        stagnant_generations++;
        
        Ruler thread_bests[threads];
        bool thread_found[threads];
        memset(thread_found, 0, sizeof(thread_found));

        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            unsigned int thread_seed = (unsigned int)(time(NULL) ^ tid ^ total_generations);
            int local_best_len = current_best;
            Ruler local_best_ruler;
            bool found_any = false;

            for (int c = 0; c < THREAD_CYCLES; c++) {
                if (get_thread_rand(&thread_seed) % 2 == 0) {
                    Ruler tmp;
                    if (generate_greedy_ruler_from(&tmp, 1, local_best_len - 1, &thread_seed)) {
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

            if (found_any) {
                thread_bests[tid] = local_best_ruler;
                thread_found[tid] = true;
            }
        }

        bool updated = false;
        for (int i = 0; i < threads; i++) {
            if (thread_found[i] && thread_bests[i].length < current_best) {
                current_best = thread_bests[i].length;
                global_pool[POP_SIZE - 1] = thread_bests[i];
                qsort(global_pool, POP_SIZE, sizeof(Ruler), compare_rulers);
                updated = true;
                stagnant_generations = 0; // 更新されたので停滞カウンターをリセット
            }
        }

        if (updated) {
            printf("\n【新記録達成！】 世代: %llu | 現在の最高記録(長さ): %d\n", total_generations, current_best);
            printf("マーク座標: ");
            for (int i = 0; i < ORDER; i++) printf("%d ", global_pool[0].marks[i]);
            printf("\n");
            save_pool(global_pool);
        } else {
            printf("\r世代: %llu | 最高記録: %d | 停滞: %d/%d 世代...", total_generations, current_best, stagnant_generations, STAGNANT_LIMIT);
            fflush(stdout);
        }

        // --- 【大手術ロジック】 ---
        if (stagnant_generations >= STAGNANT_LIMIT) {
            printf("\n【警告】%d世代連続で更新がありません。大手術（カタストロフィ突然変異）を実行します。\n", STAGNANT_LIMIT);
            
            // 上位3個体のエリートだけは完全にそのまま残す
            for (int p = 3; p < POP_SIZE; p++) {
                // ベースとして最高のエリート（global_pool）をコピー
                global_pool[p] = global_pool[0];
                
                // 後半（15〜29個目のマーク）を切り捨て、現在のベスト未満を目標に貪欲法で無理やり再構築を試みる
                // 何度かリトライして、成功した個体だけをプールに戻す
                bool success = false;
                for (int retry = 0; retry < 50; retry++) {
                    if (generate_greedy_ruler_from(&global_pool[p], 15, current_best + 200, &main_seed)) {
                        success = true;
                        break;
                    }
                }
                
                // もし再構築に失敗したら、完全な新規ランダム定規を割り当てる
                if (!success) {
                    generate_greedy_ruler_from(&global_pool[p], 1, 1500, &main_seed);
                }
            }
            
            // プールを再ソートして保存
            qsort(global_pool, POP_SIZE, sizeof(Ruler), compare_rulers);
            current_best = global_pool[0].length;
            save_pool(global_pool);
            
            printf("大手術完了。新しい可能性で探索を再開します。現在のプール最高長: %d\n", current_best);
            stagnant_generations = 0; // カウンターリセット
        }
    }

    return 0;
}

