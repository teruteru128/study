
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_MARKS 32
#define MAX_LENGTH 1024

// 下限値テーブル（OGRの既知の最短長、または数学的下限）
// ここでは探索を加速させるため、次数nに対して必要な最小長を定義
const int LOWER_BOUNDS[] = {
    0, 0, 1, 3, 6, 11, 17, 25, 34, 44, 55, 72, 85, 106, 127, 
    151, 177, 199, 216, 246, 283, 333, 356, 372, 425, 480, 492, 553, 585, 650
};

int marks[MAX_MARKS];
uint64_t diff_bitset[16]; // 1024ビット分 (64ビット * 16)
int num_marks = 29;
int best_limit = 1024;

// ビットセットの操作関数
static inline void set_bit(int d) { diff_bitset[d >> 6] |= (1ULL << (d & 63)); }
static inline void clear_bit(int d) { diff_bitset[d >> 6] &= ~(1ULL << (d & 63)); }
static inline int check_bit(int d) { return (diff_bitset[d >> 6] >> (d & 63)) & 1ULL; }

void solve(int k, int current_pos) {
    // 1. 強力な枝刈り: 現在の長さ + 残りマークに必要な最小長 >= 暫定ベスト
    if (current_pos + LOWER_BOUNDS[num_marks - k] >= best_limit)
    {
        return;
    }

    if (k == num_marks) {
        best_limit = current_pos;
        printf("New Record: %d [", best_limit);
        for (int i = 0; i < num_marks; i++) printf("%d%s", marks[i], i==k-1?"":", ");
        printf("]\n");
        return;
    }

    // 対称性の排除: 2番目のマークは (全体の長さ / 2) よりも手前に置く
    int start = marks[k - 1] + 1;
    for (int pos = start; pos < best_limit; pos++) {
        int added_diffs[MAX_MARKS];
        int count = 0;
        int conflict = 0;

        for (int i = 0; i < k; i++) {
            int d = pos - marks[i];
            if (check_bit(d)) { conflict = 1; break; }
            added_diffs[count++] = d;
        }

        if (!conflict) {
            for (int i = 0; i < count; i++) set_bit(added_diffs[i]);
            marks[k] = pos;
            solve(k + 1, pos);
            for (int i = 0; i < count; i++) clear_bit(added_diffs[i]);
        }
    }
}

int main(int argc, char *argv[], char *envp[])
{
	marks[0] = 0;
    /*0 25 67 89 137 171 212 221 245 273 311 338 394 406 449 463 500 535 554 614 635*/
    /*New Record: 807 [0, 25, 67, 89, 137, 171, 212, 221, 245, 273, 311, 338, 394, 406, 449, 463, 500, 535, 554, 614, 635, 637, 638, 654, 683, 781, 796, 801, 807]*/
    /*New Record: 801 [0, 25, 67, 89, 137, 171, 212, 221, 245, 273, 311, 338, 394, 406, 449, 463, 500, 535, 554, 614, 633, 662, 677, 709, 780, 790, 796, 797, 801]*/
#if 0
    marks[1] = 25;
    marks[2] = 67;
    marks[3] = 89;
    marks[4] = 137;
    marks[5] = 171;
    marks[6] = 212;
    marks[7] = 221;
    marks[8] = 245;
    marks[9] = 273;
    marks[10] = 311;
    marks[11] = 338;
    marks[12] = 394;
    marks[13] = 406;
    marks[14] = 449;
    marks[15] = 463;
    marks[16] = 500;
    marks[17] = 535;
    marks[18] = 554;
    marks[19] = 614;
    //marks[20] = 635;
	solve(20, 614);
#endif
    /*0 42 46 52 90 101 133 186 257 307 337 393 426 455 492 532 590*/
#if 0
    marks[1] = 42;
    marks[2] = 46;
    marks[3] = 52;
    marks[4] = 90;
    marks[5] = 101;
    marks[6] = 133;
    marks[7] = 186;
    marks[8] = 257;
    marks[9] = 307;
    marks[10] = 337;
    marks[11] = 393;
    marks[12] = 426;
    marks[13] = 455;
    marks[14] = 492;
    marks[15] = 532;
    marks[16] = 590;
	solve(17, 590);
#endif
    /*0 6 13 24 51 67 86 103 123 149 171 204 218 278 293 337 386 478 569 594 626*/
#if 0
    marks[1] = 6;
    marks[2] = 13;
    marks[3] = 24;
    marks[4] = 51;
    marks[5] = 67;
    marks[6] = 86;
    marks[7] = 103;
    marks[8] = 123;
    marks[9] = 149;
    marks[10] = 171;
    marks[11] = 204;
    marks[12] = 218;
    marks[13] = 278;
    marks[14] = 293;
    marks[15] = 337;
    marks[16] = 386;
    marks[17] = 478;
    marks[18] = 569;
    marks[19] = 594;
    marks[20] = 626;
	solve(21, 626);
#endif
    marks[1] = 4;
    marks[2] = 5;
    marks[3] = 11;
    best_limit = 801;
    set_bit(1); // 5-4
    set_bit(4); // 4-0 
    set_bit(5); // 5-0
    set_bit(6); // 11-5
    set_bit(7); // 11-4
    set_bit(11); // 11-0
    solve(4, 11);
    return 0;
}

