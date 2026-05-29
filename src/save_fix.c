#include <stdio.h>
#include <stdlib.h>

#define ORDER 29
#define POP_SIZE 40
#define SAVE_FILE "golomb_pool.dat"

typedef struct {
    int marks[ORDER];
    int length;
} Ruler;

int main() {
    Ruler pool[POP_SIZE];

    // あなたが発見した「長さ967」の正確な座標データ
    int my_best_marks[ORDER] = {
        0, 1, 5, 8, 17, 19, 40, 53, 77, 97, 
        119, 147, 180, 206, 235, 273, 316, 357, 428, 490, 
        565, 639, 690, 721, 822, 852, 942, 957, 967
    };

    // 1. 全個体（40個）のベースとしてこの優秀なデータを書き込む
    for (int p = 0; p < POP_SIZE; p++) {
        pool[p].length = 967;
        for (int i = 0; i < ORDER; i++) {
            pool[p].marks[i] = my_best_marks[i];
        }
        
        // 遺伝アルゴリズムの多様性を壊さないため、
        // 2番目以降の個体は末尾を少しずつズラして別個体としてプールに配置する
        if (p > 0) {
            pool[p].marks[ORDER - 1] = 967 + p;
            pool[p].length = 967 + p;
        }
    }

    // 2. セーブファイルの新規作成と書き出し
    FILE* fp = fopen(SAVE_FILE, "wb");
    if (fp == NULL) {
        printf("エラー: ファイル '%s' を作成できませんでした。\n", SAVE_FILE);
        return 1;
    }

    size_t written = fwrite(pool, sizeof(Ruler), POP_SIZE, fp);
    fclose(fp);

    if (written == POP_SIZE) {
        printf("成功: あなたが発見した長さ967の定規を '%s' に保存しました！\n", SAVE_FILE);
        printf("メインの探索プログラムを再起動して、967から続きが始まるか確認してください。\n");
    } else {
        printf("エラー: データの書き込み中に不具合が発生しました。\n");
    }

    return 0;
}

