#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#define ORDER 29
#define MAX_CANDIDATES 3
#define MAX_ATTEMPTS 2000
#define POP_SIZE 20          // GAで保持する個体数（定規の数）
#define MAX_DIFF 50000       // 差分マップのサイズ

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

// 新規マーク配置の検証用
bool is_valid_mark(int next_val, const int* marks, int count, const bool* diff_map) {
    for (int i = 0; i < count; i++) {
        int d = next_val - marks[i];
        if (d <= 0 || d >= MAX_DIFF || diff_map[d]) return false;
    }
    return true;
}

// 【バグ修正版】ランダム貪欲による定規生成
bool generate_greedy_ruler(Ruler* ruler, int target_max) {
    static bool diff_map[MAX_DIFF];
    memset(diff_map, 0, sizeof(diff_map));

    ruler->marks[0] = 0; // 正しい初期化
    int count = 1;

    while (count < ORDER) {
        int last_val = ruler->marks[count - 1];
        int remaining = ORDER - count;
        
        if (last_val + remaining * 2 > target_max) return false;

        int candidates[MAX_CANDIDATES];
        int cand_count = 0;
        int check_val = last_val + 1;
        int attempts = 0;

        while (cand_count < MAX_CANDIDATES && attempts < MAX_ATTEMPTS) {
            if (is_valid_mark(check_val, ruler->marks, count, diff_map)) {
                if (count == 1 && check_val > (target_max - check_val)) break; // 鏡像排除
                candidates[cand_count] = check_val;
                cand_count++;
            }
            check_val++;
            attempts++;
        }

        if (cand_count == 0) return false;

        int chosen_val = candidates[get_large_rand() % cand_count];
        for (int i = 0; i < count; i++) {
            diff_map[chosen_val - ruler->marks[i]] = true;
        }
        ruler->marks[count] = chosen_val;
        count++;
    }
    ruler->length = ruler->marks[ORDER - 1];
    return true;
}

// GA: 2つの親から子を作る（交叉＋突然変異）
bool crossover_and_mutate(const Ruler* parentA, const Ruler* parentB, Ruler* child, int target_max) {
    // 交叉点（半分より手前で切る）
    int cross_point = 10 + (rand() % 5); 
    
    // 前半は親Aをコピー
    for (int i = 0; i < cross_point; i++) {
        child->marks[i] = parentA->marks[i];
    }

    // 後半は親Bの「隙間の幅」を引き継いで配置してみる
    for (int i = cross_point; i < ORDER; i++) {
        int gap = parentB->marks[i] - parentB->marks[i - 1];
        
        // 確率で突然変異（隙間を少し揺らす）
        if (rand() % 10 == 0) { 
            gap += (rand() % 5) - 2; // -2 〜 +2 マス変化
            if (gap <= 0) gap = 1;
        }
        child->marks[i] = child->marks[i - 1] + gap;
    }

    // 出来上がった定規がゴロム定規の条件を満たし、かつ目標長以下か判定
    child->length = child->marks[ORDER - 1];
    if (child->length <= target_max && is_valid_ruler(child->marks, ORDER)) {
        return true;
    }
    return false;
}

int main() {
    srand((unsigned int)time(NULL));
    Ruler population[POP_SIZE];
    int current_best = 2000;

    printf("== 29次ゴロム定規 ハイブリッド探索（貪欲 vs GA） ==\n");
    printf("1. 初期プール（20個体）を生成中...\n");

    int initialized = 0;
    while (initialized < POP_SIZE) {
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

        // アプローチ1: 純粋なランダム貪欲（新しい可能性の開拓）
        Ruler new_greedy;
        if (generate_greedy_ruler(&new_greedy, current_best - 1)) {
            // 現在のベストより短いものが新規で見つかった場合
            current_best = new_greedy.length;
            greedy_wins++;
            
            // プールの最悪個体と入れ替え
            int worst_idx = 0;
            for (int i = 1; i < POP_SIZE; i++) {
                if (population[i].length > population[worst_idx].length) worst_idx = i;
            }
            population[worst_idx] = new_greedy;

            printf("\n【貪欲法の勝利！】新記録達成！ 長さ: %d\n", current_best);
            printf("マーク座標: ");
            for (int i = 0; i < ORDER; i++) printf("%d ", current_best == new_greedy.length ? new_greedy.marks[i] : 0); // 安全表示
            printf("\n");
        }

        // アプローチ2: 遺伝アルゴリズム（既存の劣最適解のすり合わせと改良）
        int pA = rand() % POP_SIZE;
        int pB = rand() % POP_SIZE;
        Ruler child;
        if (crossover_and_mutate(&population[pA], &population[pB], &child, current_best - 1)) {
            current_best = child.length;
            ga_wins++;

            // プールの最悪個体と入れ替え
            int worst_idx = 0;
            for (int i = 1; i < POP_SIZE; i++) {
                if (population[i].length > population[worst_idx].length) worst_idx = i;
            }
            population[worst_idx] = child;

            printf("\n【遺伝アルゴリズム(GA)の勝利！】進化しました！ 長さ: %d\n", current_best);
            printf("マーク座標: ");
            for (int i = 0; i < ORDER; i++) printf("%d ", child.marks[i]);
            printf("\n");
        }

        // 定期進捗表示
        if (cycles % 100000 == 0) {
            printf("\rサイクル数: %llu | 現在の最高記録: %d (貪欲勝利: %llu, GA勝利: %llu)", cycles, current_best, greedy_wins, ga_wins);
            fflush(stdout);
        }
    }

    return 0;
}

