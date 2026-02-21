
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_MARKS 32
#define MAX_LENGTH 1024

int marks[MAX_MARKS];
bool diff_used[MAX_LENGTH + 1];
int num_marks;
int best_length = 1024;

void solve(int k, int current_pos)
{
    if(current_pos >= best_length) return;

    if(k == num_marks)
    {
        best_length = current_pos;
        printf("%d\n", best_length);
        for(int i = 0; i < num_marks; i++)
        {
            printf("%d ", marks[i]);
        }
        printf("\n");
        return;
    }
	// 次のマークの位置を試行（順次短くする戦略のため、上限を制限）
    for (int pos = marks[k - 1] + 1; pos < best_length; pos++) {
        bool conflict = false;
        int diffs[MAX_MARKS];
        int count = 0;

        // 新しいマークによって生じる全距離がユニークかチェック
        for (int i = 0; i < k; i++) {
            int d = pos - marks[i];
            if (diff_used[d]) {
                conflict = true;
                break;
            }
            diffs[count++] = d;
        }

        if (!conflict) {
            // 距離を登録して次の階層へ
            for (int i = 0; i < count; i++) diff_used[diffs[i]] = true;
            marks[k] = pos;
            
            solve(k + 1, pos);

            // 戻る際に距離登録を解除（バックトラック）
            for (int i = 0; i < count; i++) diff_used[diffs[i]] = false;
        }
    }
}

int main(int argc, char *argv[], char *envp[])
{
	marks[0] = 0;
	solve(1, 0);
    return 0;
}

