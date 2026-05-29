#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#define ORDER 29
#define MAX_CANDIDATES 3
#define MAX_ATTEMPTS 2000
#define POP_SIZE 20          // GAで保持する個体数（定規の数）
#define MAX_DIFF 100000      // 差分マップのサイズを10万に拡大してさらに安全に

// ゴロム定規の構造体
typedef struct {
    int marks[ORDER];
    int length;
} Ruler;

// 高精度乱数
int get_large_rand() {
    return (rand() << 15) | rand();
}

// 差分チェック（重複があればfalse）
bool is_valid_ruler(const int* marks, int count) {
    static bool local_diff[MAX_DIFF];
    memset(local_diff, 0, sizeof(local_diff));
    
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            int d = marks[j] - marks[i];
            if (d <= 0 || d >= MAX_DIFF || local_diff[d]) return false;
            local_diff[d] = true;
        }
    }
    return true;
}

// 新規マーク配置の検証用（安全ガード付き）
bool is_valid_mark(int next_val, const int* marks, int count, const bool* diff_map) {
    for (int i = 0; i < count; i++) {
        int d = next_val - marks[i];
        if (d <= 0 || d >= MAX_DIFF) return false; // 配列範囲外を事前にガード
        if (diff_map[d]) return false;
    }
    return true;
}

// ランダム貪欲による定規生成（安全版）
bool generate_greedy_ruler(Ruler* ruler, int target_max) {
    static bool diff_map[MAX_DIFF];
    memset(diff_map, 0, sizeof(diff_map));

    ruler->marks[0] = 0;
    int count = 1;

    while (count < ORDER) {
        int last_val = ruler->marks[count - 1];
        int remaining = ORDER - count;
        
        // 残りのマークを最低1マス空けで置いてもターゲットを超えるなら終了
        if (last_val + remaining > target_max) return false;

        int candidates[MAX_CANDIDATES];
        int cand_count = 0;
        int check_val = last_val + 1;
        int attempts = 0;

        while (cand_count < MAX_CANDIDATES && attempts < MAX_ATTEMPTS) {
            if (is_valid_mark(check_val, ruler->marks, count, diff_map)) {
                // 鏡像排除：2個目のマーク（最初の隙間）が大きすぎるものは除外（安全のため上限50）
                if (count == 1 && check_val > 50) break; 
                
                candidates[cand_count] = check_val;
                cand_count++;
            }
            check_val++;
            attempts++;
        }

        if (cand_count == 0) return false;

        int chosen_val = candidates[get_large_rand() % cand_count];
        
        // 差分マップの更新（安全ガード付き）
        for (int i = 0; i < count; i++) {
            int d = chosen_val - ruler->marks[i];
            if (d > 0 && d < MAX_DIFF) {
                diff_map[d] = true;
            }
        }
        ruler->marks[count] = chosen_val;
        count++;
    }
    
    ruler->length = ruler->marks[ORDER - 1];
    // 最終チェック
    if (ruler->length <= target_max && is_valid_ruler(ruler->marks, ORDER)) {
        return true;
    }
    return false;
}

// GA: 2つの親から子を作る（交叉＋突然変異）
bool crossover_and_mutate(const Ruler* parentA, const Ruler* parentB, Ruler* child, int target_max) {
    // 交叉点（半分より手前で切る）
    int cross_point = 10 + (rand() % 5); 
    
    // 前半は親Aをコピー
    for (int i = 0; i < cross_point; i++) {
        child->marks[i] = parentA->marks[i];
    }

    // 後半は親Bの「隙間の幅」を引き継いで配置
    for (int i = cross_point; i < ORDER; i++) {
        int gap = parentB->marks[i] - parentB->marks[i - 1];
        
        // 確率で突然変異（隙間を少し揺らす）
        if (rand() % 10 == 0) { 
            gap += (rand() % 3) - 1; // -1 〜 +1 マス変化
            if (gap <= 0) gap = 1;
        }
        child->marks[i] = child->marks[i - 1] + gap;
    }

    child->length = child->marks[ORDER - 1];
    if (child->length <= target_max && is_valid_ruler(child->marks, ORDER)) {
        return true;
    }
    return false;
}

int main() {
    srand((unsigned int)time(NULL));
    Ruler population[POP_SIZE];
    int current_best = 99999;

    printf("== 29次ゴロム定規 ハイブリッド探索（貪欲 vs GA） ==\n");
    printf("1. 初期プール（20個体）を生成中...\n");

    int initialized = 0;
    while (initialized < POP_SIZE) {
        // 初期ターゲットは少し余裕を持たせて1500に設定
        if (generate_greedy_ruler(&population[initialized], 1500)) {
            if (population[initialized].length < current_best) {
                current_best = population[initialized].length;
            }
            initialized++;
            printf("\r初期個体生成: [%d / %d] (現在のベスト: %d)", initialized, POP_SIZE, current_best);
            fflush(stdout);
        }
    }
    printf("\n\n2. 対決探索スタート（貪欲法と遺伝アルゴリズムを同時並行）\n");

    unsigned long long cycles = 0;
    unsigned long long greedy_wins = 0;
    unsigned long long ga_wins = 0;

    while (1) {
        cycles++;

        // アプローチ1: 純粋なランダム貪欲
        Ruler new_greedy;
        if (generate_greedy_ruler(&new_greedy, current_best - 1)) {
            current_best = new_greedy.length;
            greedy_wins++;
            
            int worst_idx = 0;
            for (int i = 1; i < POP_SIZE; i++) {
                if (population[i].length > population[worst_idx].length) worst_idx = i;
            }
            population[worst_idx] = new_greedy;

            printf("\n【貪欲法の勝利！】新記録達成！ 長さ: %d\n", current_best);
            printf("マーク座標: ");
            for (int i = 0; i < ORDER; i++) printf("%d ", new_greedy.marks[i]);
            printf("\n");
        }

        // アプローチ2: 遺伝アルゴリズム(GA)
        int pA = rand() % POP_SIZE;
        int pB = rand() % POP_SIZE;
        Ruler child;
        if (crossover_and_mutate(&population[pA], &population[pB], &child, current_best - 1)) {
            current_best = child.length;
            ga_wins++;

            int worst_idx = 0;
            for (int i = 1; i < POP_SIZE; i++) {
                if (population[i].length > population[worst_idx].length) worst_idx = i;
            }
            population[worst_idx] = child;

            printf("\n【遺伝アルゴリズム(GA)의 勝利！】進化しました！ 長さ: %d\n", current_best);
            printf("マーク座標: ");
            for (int i = 0; i < ORDER; i++) printf("%d ", child.marks[i]);
            printf("\n");
        }

        // 500,000サイクルごとに進捗を表示
        if (cycles % 500000 == 0) {
            printf("\rサイクル数: %llu | 現在の最高記録: %d (貪欲勝利: %llu, GA勝利: %llu)", cycles, current_best, greedy_wins, ga_wins);
            fflush(stdout);
        }
    }

    return 0;
}

