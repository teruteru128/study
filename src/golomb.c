
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>

#define MAX_MARKS 32
#define MAX_LENGTH 1024

const int LOWER_BOUNDS[] = {
    0, 0, 1, 3, 6, 11, 17, 25, 34, 44, 55, 72, 85, 106, 127,
    151, 177, 199, 216, 246, 283, 333, 356, 372, 425, 480, 492, 553, 585, 650
};

// スレッド間で共有する暫定最短
int best_limit = 801;

// ビットセット操作のインライン関数（スレッド個別データ用）
static inline void set_bit(uint64_t *bs, int d) { bs[d >> 6] |= (1ULL << (d & 63)); }
static inline void clear_bit(uint64_t *bs, int d) { bs[d >> 6] &= ~(1ULL << (d & 63)); }
static inline int check_bit(uint64_t *bs, int d) { return (bs[d >> 6] >> (d & 63)) & 1ULL; }

// 再帰関数（スレッドごとに独立した marks と bitset を持つ）
void solve_parallel(int k, int current_pos, int num_marks, int *local_marks, uint64_t *local_bs) {
    // 枝刈り
    if (current_pos + LOWER_BOUNDS[num_marks - k] >= best_limit) return;

    if (k == num_marks) {
        #pragma omp critical
        {
            if (current_pos < best_limit) {
                best_limit = current_pos;
                printf("\n--- New Record: %d ---\n", best_limit);
                for (int i = 0; i < num_marks; i++) printf("%d%s", local_marks[i], i == k - 1 ? "" : ", ");
                printf("\n---------------------\n");
            }
        }
        return;
    }

    int start = local_marks[k - 1] + 1;
    // 枝刈り効率化のため best_limit を動的にチェック
    for (int pos = start; pos < best_limit - LOWER_BOUNDS[num_marks - k]; pos++) {
        int added_diffs[MAX_MARKS];
        int count = 0;
        int conflict = 0;

        for (int i = 0; i < k; i++) {
            int d = pos - local_marks[i];
            if (check_bit(local_bs, d)) { conflict = 1; break; }
            added_diffs[count++] = d;
        }

        if (!conflict) {
            for (int i = 0; i < count; i++) set_bit(local_bs, added_diffs[i]);
            local_marks[k] = pos;
            solve_parallel(k + 1, pos, num_marks, local_marks, local_bs);
            for (int i = 0; i < count; i++) clear_bit(local_bs, added_diffs[i]);
        }
    }
}

int main() {
    int num_marks = 29;
    
    // 初期固定値 (パターンCベース)
    int initial_marks[MAX_MARKS] = {0, 4, 5, 11};
    uint64_t initial_bs[16] = {0};
    
    // 初期ビット登録
    int init_count = 4;
    for(int i=0; i<init_count; i++) {
        for(int j=0; j<i; j++) {
            set_bit(initial_bs, initial_marks[i] - initial_marks[j]);
        }
    }

    printf("Starting parallel search for OGR-%d (Current Limit: %d)...\n", num_marks, best_limit);

    // marks[4] のループを並列化
    #pragma omp parallel
    {
        // 各スレッドに専用のメモリ領域を確保
        int local_marks[MAX_MARKS];
        uint64_t local_bs[16];
        memcpy(local_marks, initial_marks, sizeof(int) * MAX_MARKS);
        memcpy(local_bs, initial_bs, sizeof(uint64_t) * 16);

        // marks[4] の範囲をスレッド間で分割
        #pragma omp for schedule(dynamic, 1)
        for (int pos = initial_marks[init_count-1] + 1; pos < 801; pos++) {
            int added_diffs[MAX_MARKS];
            int count = 0;
            int conflict = 0;

            for (int i = 0; i < init_count; i++) {
                int d = pos - local_marks[i];
                if (check_bit(local_bs, d)) { conflict = 1; break; }
                added_diffs[count++] = d;
            }

            if (!conflict) {
                for (int i = 0; i < count; i++) set_bit(local_bs, added_diffs[i]);
                local_marks[init_count] = pos;
                solve_parallel(init_count + 1, pos, num_marks, local_marks, local_bs);
                for (int i = 0; i < count; i++) clear_bit(local_bs, added_diffs[i]);
            }
        }
    }

    return 0;
}
