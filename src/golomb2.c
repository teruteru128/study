#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ORDER 29
#define MAX_DIFF 100000      // 差分判定用ビットマップ配列のサイズ
#define SAVE_FILE "golomb_pool.dat"
#define BACKTRACK_LIMIT 200000 // 各試行で掘り下げる最大ステップ数（ドツボにハマる前に見切る限界値）

// セーブファイルとの互換性を保つための構造体
typedef struct {
    int marks[ORDER];
    int length;
} Ruler;

// Xorshift乱数（スレッド安全・超高速）
unsigned int get_thread_rand(unsigned int* state) {
    unsigned int x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

// 差分チェック（重複があればfalse）
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

// 制限付き深さ優先探索（DFS）の再帰関数
bool dfs_solve(int count, int* marks, bool* diff_map, int target_max, unsigned long long* steps) {
    (*steps)++;
    if (*steps > BACKTRACK_LIMIT) return false; // タイムアウト（見切り）

    if (count == ORDER) {
        return true; // 29次完成！
    }

    int last_val = marks[count - 1];
    
    // 残りのマークを最低限詰めて置いたとしてもターゲット（現在のベスト-1）を超えるなら枝切り
    int remaining = ORDER - count;
    if (last_val + (remaining * (remaining + 1)) / 2 > target_max) {
        return false;
    }

    // 次のマークの座標を探索
    int start_val = last_val + 1;
    for (int v = start_val; v <= target_max; v++) {
        
        // 差分重複チェック
        bool valid = true;
        for (int i = 0; i < count; i++) {
            int d = v - marks[i];
            if (d >= MAX_DIFF || diff_map[d]) {
                valid = false;
                break;
            }
        }

        if (valid) {
            // 鏡像排除（2個目のマークが大きすぎるものはスキップ）
            if (count == 1 && v > 45) {
                break; 
            }

            // 差分を登録して進む
            for (int i = 0; i < count; i++) diff_map[v - marks[i]] = true;
            marks[count] = v;

            if (dfs_solve(count + 1, marks, diff_map, target_max, steps)) {
                return true; // 完成したシグナルを上に返す
            }

            // バックトラック：差分を解除して戻る
            for (int i = 0; i < count; i++) diff_map[v - marks[i]] = false;
        }
    }

    return false;
}

// セーブファイル読み込み
int load_best_length() {
    Ruler pool[40];
    FILE* fp = fopen(SAVE_FILE, "rb");
    if (fp == NULL) return 967; // ファイルがなければ967をデフォルトに
    size_t cnt = fread(pool, sizeof(Ruler), 40, fp);
    fclose(fp);
    if (cnt > 0) return pool[0].length; // 一番優秀な長さを返す
    return 967;
}

// セーブファイル保存
void save_new_best(int* marks, int length) {
    Ruler pool[40];
    // 既存のファイルを一度読み込んでプールを埋める
    FILE* fp = fopen(SAVE_FILE, "rb");
    if (fp != NULL) {
        fread(pool, sizeof(Ruler), 40, fp);
        fclose(fp);
    }
    
    // 全要素のトップに今回の新記録を強制上書き
    for (int i = 0; i < 40; i++) {
        pool[i].length = length;
        memcpy(pool[i].marks, marks, sizeof(int) * ORDER);
    }

    fp = fopen(SAVE_FILE, "wb");
    if (fp != NULL) {
        fwrite(pool, sizeof(Ruler), 40, fp);
        fclose(fp);
    }
}

int main() {
    srand((unsigned int)time(NULL));
    int current_best = load_best_length();

    printf("== 29次ゴロム定規 【真・根本刷新版】反復制限マルチスタートDFS ==\n");
    int threads = omp_get_max_threads();
    printf("使用スレッド数 (CPUコア数): %d\n", threads);
    printf("ターゲット防衛ライン (この長さ未満を探索): %d\n", current_best);
    printf("1スレッドあたりの見切り制限: %d ステップ\n\n", BACKTRACK_LIMIT);

    unsigned long long total_attempts = 0;
    double start_time = omp_get_wtime();

    #pragma omp parallel reduction(+:total_attempts)
    {
        int tid = omp_get_thread_num();
        unsigned int thread_seed = (unsigned int)(time(NULL) ^ tid);
        
        // スレッド個別の作業領域
        int local_marks[ORDER];
        bool local_diff[MAX_DIFF];

        while (1) {
            total_attempts++;
            memset(local_diff, 0, sizeof(local_diff));
            
		    // --- ランダム初期化フェーズ（修正版） ---
            // 29次の黄金の初期配置 [0, 1, 5, 8] を完全に固定する
            local_marks[0] = 0;
            local_marks[1] = 1;
            local_marks[2] = 5;
            local_marks[3] = 8;
            
            memset(local_diff, 0, sizeof(local_diff));
            local_diff[1] = true; // 1-0
            local_diff[5] = true; // 5-0
            local_diff[8] = true; // 8-0
            local_diff[4] = true; // 5-1
            local_diff[7] = true; // 8-1
            local_diff[3] = true; // 8-5

            // 5個目のマークの選択にランダム性（マルチスタート）を持たせる
            // 8の直後ではなく、少し離れたランダムな位置（例: 9〜30の間）からDFSを開始する
            int rand_start_v = 9 + (get_thread_rand(&thread_seed) % 22);
            
            // 5個目の暫定マークを置いてみる
            bool valid = true;
            for (int i = 0; i < 4; i++) {
                int d = rand_start_v - local_marks[i];
                if (d >= MAX_DIFF || local_diff[d]) { valid = false; break; }
            }

            if (!valid) continue; // ダメなら次のアテンプトへ

            // 5個目を仮登録してDFSへ
            for (int i = 0; i < 4; i++) local_diff[rand_start_v - local_marks[i]] = true;
            local_marks[4] = rand_start_v;

            // --- 制限付きDFSフェーズ ---
            // 5個目（count=5）から深さ優先探索を開始
            unsigned long long steps = 0;
            if (dfs_solve(5, local_marks, local_diff, current_best - 1, &steps)) {
 
                // 厳密チェックを通過した場合のみ記録更新
                if (is_valid_ruler(local_marks, ORDER)) {
                    #pragma omp critical
                    {
                        if (local_marks[ORDER - 1] < current_best) {
                            current_best = local_marks[ORDER - 1];
                            save_new_best(local_marks, current_best);
                            
                            printf("\n\n【★新記録達成！★】 長さ: %d (既知の最短: 633)\n", current_best);
                            printf("マーク座標: ");
                            for (int i = 0; i < ORDER; i++) printf("%d ", local_marks[i]);
                            printf("\n\n");
                        }
                    }
                }
            }

            // 定期的な進捗表示（スレッド0のみが代表して出力）
            if (tid == 0 && total_attempts % 5000 == 0) {
                double elapsed = omp_get_wtime() - start_time;
                printf("\r総試行回数: %llu 万回 | 経過時間: %.1f 秒 | 現在のターゲット: %d 以下...", 
                       (total_attempts * threads) / 10000, elapsed, current_best);
                fflush(stdout);
            }
        }
    }

    return 0;
}

