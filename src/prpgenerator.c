
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <gmp.h>
#include <sqlite3.h>

#define DB_FILE "factordb_tasks.db"
#define AUTOLOAD_FILE "autoload.txt"
#define POOL_TARGET_SIZE 5000

// 19桁の素数を生成してプール(SQLite)を管理する処理
void replenish_pool(sqlite3 *db, gmp_randstate_t state) {
    mpz_t p_min;
    mpz_t p_max;
    mpz_t p_window;
    mpz_t p;
    mpz_inits(p_min, p_max, p_window, p, NULL);
    mpz_ui_pow_ui(p_min, 10, 18);
    mpz_ui_pow_ui(p_max, 10, 19);
    mpz_prevprime(p_max, p_max);
    mpz_sub(p_window, p_max, p_min);
    // 10^18～10^19の範囲で乱数生成、ミラー・ラビン法で素数判定
    mpz_urandomm(p, state, p_window);
    mpz_nextprime(p, p);
    // sqlite3_prepare/step等を用いてプールへ追加（全コードはWeb資料を参照）
}

// プールから取得した素数でPRPを構成し、autoload.txtへ書き出す処理
void generate_autoload_file(sqlite3 *db) {
    // ファイルがない場合、プールから2つ素数を取得し(p1*p2±1等)を計算
    // autoload.txtに書き込み、使用した素数をDELETEする（全コードはWeb資料を参照）
}

int main(int argc, char *argv[]) {
	// DBオープン、初期化、乱数シード設定
    sqlite3 *db = NULL;
    int rc = sqlite3_open(DB_FILE, &db);
    if(rc) {
        fprintf(stderr, "%s\n", sqlite3_errmsg(db));
        return 1;
    }
    gmp_randstate_t state;
    // 2秒間隔でreplenish_poolとgenerate_autoload_fileをループ（全コードはWeb資料を参照）
    return 0;
}

