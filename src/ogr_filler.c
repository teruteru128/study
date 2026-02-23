
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_MARKS 32
#define MAX_LEN 2048

uint64_t diff_bs[32]; // ビットセット
int marks[MAX_MARKS];
int g_num_marks, g_max_len, g_fixed_count;

static inline void set_bit(int d) { diff_bs[d >> 6] |= (1ULL << (d & 63)); }
static inline int check_bit(int d) { return (diff_bs[d >> 6] >> (d & 63)) & 1ULL; }
static inline void clear_bit(int d) { diff_bs[d >> 6] &= ~(1ULL << (d & 63)); }

void solve(int k, int last_pos) {
    if (k == g_num_marks) {
        printf("Found: [");
        for (int i = 0; i < g_num_marks; i++) printf("%d%s", marks[i], i == k - 1 ? "" : ", ");
        printf("]\n");
        return;
    }

    // 最後のマークは必ず g_max_len に固定したい場合や、その手前で探す場合
    // ここでは「最大長さ以下」ですべての組み合わせを探します
    for (int pos = last_pos + 1; pos <= g_max_len; pos++) {
        int added[MAX_MARKS], count = 0;
        bool conflict = false;

        for (int i = 0; i < k; i++) {
            int d = pos - marks[i];
            if (check_bit(d)) { conflict = true; break; }
            added[count++] = d;
        }

        if (!conflict) {
            for (int i = 0; i < count; i++) set_bit(added[i]);
            marks[k] = pos;
            solve(k + 1, pos);
            for (int i = 0; i < count; i++) clear_bit(added[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <num_marks> <max_length> <fixed_mark1> <fixed_mark2> ...\n", argv[0]);
        printf("Example: %s 5 11 0 1 3 (Finds 0,1,3,?,? within length 11)\n", argv[0]);
        return 1;
    }

    g_num_marks = atoi(argv[1]);
    g_max_len = atoi(argv[2]);
    g_fixed_count = argc - 3;

    // 既知のマークをセット
    for (int i = 0; i < g_fixed_count; i++) {
        marks[i] = atoi(argv[i + 3]);
        if (i > 0 && marks[i] <= marks[i-1]) {
            printf("Error: Marks must be in increasing order.\n");
            return 1;
        }
    }

    // 既知のマーク間の距離をビットセットに登録
    for (int i = 0; i < g_fixed_count; i++) {
        for (int j = 0; j < i; j++) {
            int d = marks[i] - marks[j];
            if (check_bit(d)) {
                printf("Error: Fixed marks already violate Golomb property (dist %d duplicated).\n", d);
                return 1;
            }
            set_bit(d);
        }
    }

    printf("Searching for %d more marks to complete Order-%d, Max Length %d...\n", 
           g_num_marks - g_fixed_count, g_num_marks, g_max_len);

    solve(g_fixed_count, marks[g_fixed_count - 1]);

    return 0;
}

