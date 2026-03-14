
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>

#define MAX_MARKS 32
#define MAX_LENGTH 2048
#define CKPT_FILE "checkpoint.txt"

const int LOWER_BOUNDS[] = {
    0, 0, 1, 3, 6, 11, 17, 25, 34, 44, 55, 72, 85, 106, 127,
    151, 177, 199, 216, 246, 283, 333, 356, 372, 425, 480, 492, 553, 585, 650
};

int best_limit = 1024;
int last_completed_pos = 1; // marks[4]の開始位置

static inline void set_bit(uint64_t *bs, int d) { bs[d >> 6] |= (1ULL << (d & 63)); }
static inline void clear_bit(uint64_t *bs, int d) { bs[d >> 6] &= ~(1ULL << (d & 63)); }
static inline int check_bit(uint64_t *bs, int d) { return (bs[d >> 6] >> (d & 63)) & 1ULL; }

// チェックポイントの読み込み
void load_checkpoint() {
    FILE *fp = fopen(CKPT_FILE, "r");
    if (fp) {
        if (fscanf(fp, "%d %d", &best_limit, &last_completed_pos) == 2) {
            printf("Checkpoint loaded: Best=%d, Resume from pos=%d\n", best_limit, last_completed_pos);
        }
        fclose(fp);
    }
}

// チェックポイントの保存
void save_checkpoint(int current_pos) {
    #pragma omp critical(save_lock)
    {
        FILE *fp = fopen(CKPT_FILE, "w");
        if (fp) {
            fprintf(fp, "%d %d", best_limit, current_pos);
            fclose(fp);
        }
    }
}

void solve_recursive(int k, int current_pos, int num_marks, int *local_marks, uint64_t *local_bs) {
    if (current_pos + LOWER_BOUNDS[num_marks - k] >= best_limit) return;

    if (k == num_marks) {
        #pragma omp critical(best_lock)
        {
            if (current_pos < best_limit) {
                best_limit = current_pos;
                printf("\n[*] NEW BEST FOUND: %d\n", best_limit);
                FILE *log = fopen("found_rulers.log", "a");
                fprintf(log, "Length %d: ", best_limit);
                for (int i = 0; i < num_marks; i++) fprintf(log, "%d%s", local_marks[i], i==num_marks-1?"":", ");
                fprintf(log, "\n");
                fclose(log);
                save_checkpoint(last_completed_pos);
            }
        }
        return;
    }

    for (int pos = local_marks[k-1] + 1; pos < best_limit - LOWER_BOUNDS[num_marks - k]; pos++) {
        bool conflict = false;
        int diffs[MAX_MARKS], d_cnt = 0;
        for (int i = 0; i < k; i++) {
            int d = pos - local_marks[i];
            if (check_bit(local_bs, d)) { conflict = true; break; }
            diffs[d_cnt++] = d;
        }
        if (!conflict) {
            for (int i = 0; i < d_cnt; i++) set_bit(local_bs, diffs[i]);
            local_marks[k] = pos;
            solve_recursive(k + 1, pos, num_marks, local_marks, local_bs);
            for (int i = 0; i < d_cnt; i++) clear_bit(local_bs, diffs[i]);
        }
    }
}

int main() {
    load_checkpoint();
    int num_marks = 29;
    int initial_marks[] = {0};
    uint64_t initial_bs[16] = {0};

    for(int i=0; i<1; i++) {
        for(int j=0; j<i; j++) set_bit(initial_bs, initial_marks[i] - initial_marks[j]);
    }

    printf("Starting search... Press Ctrl+C to stop. Checkpoint saved periodically.\n");

    #pragma omp parallel
    {
        int local_marks[MAX_MARKS];
        uint64_t local_bs[16];
        memcpy(local_marks, initial_marks, sizeof(int)*1);
        memcpy(local_bs, initial_bs, sizeof(uint64_t)*16);

        #pragma omp for schedule(dynamic, 1)
        for (int p4 = last_completed_pos + 1; p4 < 801; p4++) {
            // 定期的な進捗表示 (スレッド0のみ)
            if (omp_get_thread_num() == 0) {
                printf("Searching marks[4] = %d (Best: %d)\n", p4, best_limit);
                last_completed_pos = p4;
                save_checkpoint(p4);
            }

            int diffs[4], d_cnt = 0;
            bool conflict = false;
            for (int i = 0; i < 1; i++) {
                int d = p4 - local_marks[i];
                if (check_bit(local_bs, d)) { conflict = true; break; }
                diffs[d_cnt++] = d;
            }

            if (!conflict) {
                for (int i = 0; i < d_cnt; i++) set_bit(local_bs, diffs[i]);
                local_marks[1] = p4;
                solve_recursive(1, p4, num_marks, local_marks, local_bs);
                for (int i = 0; i < d_cnt; i++) clear_bit(local_bs, diffs[i]);
            }
        }
    }

    printf("Search completed up to length 801.\n");
    return 0;
}
