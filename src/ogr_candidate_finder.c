
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_MARKS 29
#define TARGET_LEN 801

uint64_t diff_bs[16] = {0}; // 1024ビット
int marks[MAX_MARKS];
int fixed_marks[] = {0, 4, 5, 11};
int num_fixed = 4;
int total_needed = 29;

static inline void set_bit(int d) { diff_bs[d >> 6] |= (1ULL << (d & 63)); }
static inline int check_bit(int d) { return (diff_bs[d >> 6] >> (d & 63)) & 1ULL; }
static inline void clear_bit(int d) { diff_bs[d >> 6] &= ~(1ULL << (d & 63)); }

void solve(int k, int last_pos) {
    // 29個すべて埋まった場合
    if (k == total_needed) {
        printf("\n[!] SUCCESS: Full OGR-29 found!\n");
        for (int i = 0; i < total_needed; i++) printf("%d%s", marks[i], i == total_needed - 1 ? "" : ", ");
        printf("\n");
        return;
    }

    // 次のマーク候補を探索
    for (int pos = last_pos + 1; pos < TARGET_LEN; pos++) {
        // 固定マークと重ならないかチェック（既にmarksに入っているため、厳密には不要だが効率のため）
        bool exists = false;
        for(int i=0; i<num_fixed; i++) if(pos == fixed_marks[i]) { exists = true; break; }
        if(exists) continue;

        int added_diffs[MAX_MARKS], d_cnt = 0;
        bool conflict = false;

        for (int i = 0; i < k; i++) {
            int d = (pos > marks[i]) ? (pos - marks[i]) : (marks[i] - pos);
            if (check_bit(d)) { conflict = true; break; }
            added_diffs[d_cnt++] = d;
        }

        if (!conflict) {
            for (int i = 0; i < d_cnt; i++) set_bit(added_diffs[i]);
            // 昇順を維持するために挿入位置を調整するか、単に最後に追加してソートするか
            // ここでは簡易的に、現在のlast_posより大きい場所を探す
            marks[k] = pos;
            // 進行状況の表示
            if (k < 22) { printf("\rLevel %d: trying pos %d...", k, pos); fflush(stdout); }
            
            solve(k + 1, pos);
            for (int i = 0; i < d_cnt; i++) clear_bit(added_diffs[i]);
        }
    }
}

int main() {
    // 固定マークの登録と距離チェック
    memcpy(marks, fixed_marks, sizeof(fixed_marks));
    for (int i = 0; i < num_fixed; i++) {
        for (int j = 0; j < i; j++) {
            int d = marks[i] - marks[j];
            if (check_bit(d)) {
                printf("Error: Initial marks conflict at distance %d\n", d);
                return 1;
            }
            set_bit(d);
        }
    }

    printf("Starting search for remaining 10 marks in OGR-29 (Len 635)...\n");
    // 固定マークの「隙間」を埋める必要があるため、ロジックを少し変更して
    // 0から635までの全域で空いている場所を探す形にするのが理想的です。
    // 今回は単純化のため、既存の最大値(635)の内側で、各固定値の「間」を探索します。
    
    // 注意: この単純な再帰では「固定マークの間の隙間」を効率よく埋められないため、
    // 実際には「空いている座標リスト」を先に作り、そこから10個選ぶ組合せ最適化として解くのが高速です。
    
    solve(num_fixed, 0); 

    return 0;
}

