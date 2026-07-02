
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

/**
 * 与えられた M, s, z から x, y を逆算・判定する関数。
 * すべての変数は内部で完結しており、スレッドセーフ（メモリ安全）。
 */
int test_combination(mpz_t x, mpz_t y, const mpz_t M, const mpz_t s, const mpz_t z) {
    // M が s で割り切れない場合は即座に終了
    if (!mpz_divisible_p(M, s)) return 0;

    mpz_t t, rem, D, k, temp_x, temp_y;
    int is_solved = 0;

    mpz_inits(t, rem, D, k, temp_x, temp_y, NULL);

    // t = M / s
    mpz_divexact(t, M, s);

    // rem = 4t - s^2
    mpz_mul_ui(rem, t, 4);
    mpz_mul(temp_x, s, s);
    mpz_sub(rem, rem, temp_x);

    // 判別式条件: rem >= 0 かつ 3で割り切れるか
    if (mpz_sgn(rem) >= 0 && mpz_divisible_ui_p(rem, 3)) {
        mpz_divexact_ui(D, rem, 3);

        // D が完全平方数か
        if (mpz_perfect_square_p(D)) {
            mpz_sqrt(k, D);

            // s と k の奇偶が一致するか (s + k が偶数)
            mpz_add(temp_x, s, k);
            if (mpz_even_p(temp_x)) {
                // x = (s + k) / 2, y = (s - k) / 2
                mpz_divexact_ui(x, temp_x, 2);
                mpz_sub(temp_y, s, k);
                mpz_divexact_ui(y, temp_y, 2);
                is_solved = 1;
            }
        }
    }

    mpz_clears(t, rem, D, k, temp_x, temp_y, NULL);
    return is_solved;
}

/**
 * 特定の s（1つのタスク）に対して独立して探索を行う関数。
 * 状態を共有しないため、この関数単位で将来並列実行が可能。
 */
int check_single_s(mpz_t x_out, mpz_t y_out, mpz_t z_out, unsigned long s_ui, int n_val) {
    // 【高度数論フィルタ1】 114 ≡ 6 (mod 9) より、x, y, z ≡ 2 (mod 3) が必須。
    // s = x + y なので、s ≡ 2 + 2 ≡ 4 ≡ 1 (mod 3) でなければならない。
    // これを満たさない s は、一切計算を行わずに 1ステップで完全排除（2/3の s をスキップ）
    if (s_ui % 3 != 1) {
        return 0; 
    }

    // スレッドセーフのためのローカル変数群の初期化
    mpz_t n, s, z, z3, M;
    int found = 0;
    mpz_inits(n, s, z, z3, M, NULL);
    
    mpz_set_si(n, n_val);
    mpz_set_ui(s, s_ui);

    // 【高度数論フィルタ2】 z^3 ≡ n (mod s) の簡易判定
    // z ≡ 2 (mod 3) かつ z^3 ≡ 114 (mod s) を満たす局所的な z_mod の存在チェック
    // 将来的な並列化時、s が小さいうちはテーブル化、大きい場合はシャンクス・トネリの3次拡張等に置き換え可能
    unsigned long z_mod = 0;
    int has_cube_root = 0;
    long target_mod = n_val % (long)s_ui;
    if (target_mod < 0) target_mod += s_ui;

    for (unsigned long i = 2; i < s_ui; i += 3) { // i ≡ 2 (mod 3) のみ走査
        unsigned long i3 = (i * i % s_ui) * i % s_ui;
        if (i3 == (unsigned long)target_mod) {
            has_cube_root = 1;
            z_mod = i;
            break; 
        }
    }

    if (!has_cube_root) {
        goto cleanup; // 局所解がなければ即スキップ
    }

    // z = z_mod + k * s の周辺を探索 (変数間の競合がない安全なループ)
    long k_start = -20000; 
    long k_end = 20000;

    for (long k = k_start; k <= k_end; k++) {
        // z = z_mod + k * s
        mpz_set_si(z, k);
        mpz_mul(z, z, s);
        mpz_add_ui(z, z, z_mod);

        // 正負両方のzを独立して検証
        for (int sign = 0; sign < 2; sign++) {
            if (sign == 1) mpz_neg(z, z);

            // z ≡ 2 (mod 3) の条件を厳格にチェック（符号反転対策）
            long z_m3 = mpz_fdiv_ui(z, 3);
            if (z_m3 != 2) continue;

            mpz_pow_ui(z3, z, 3);
            mpz_sub(M, n, z3);

            if (test_combination(x_out, y_out, M, s, z)) {
                mpz_set(z_out, z);
                found = 1;
                goto cleanup;
            }
        }
    }

cleanup:
    mpz_clears(n, s, z, z3, M, NULL);
    return found;
}

void search_three_cubes_advanced(int n_val, unsigned long s_max) {
    printf("n = %d の高度数論探索を開始します (sの上限: %lu)...\n", n_val, s_max);

    // 最終出力を格納する変数（メインスレッドが保持）
    mpz_t final_x, final_y, final_z;
    mpz_inits(final_x, final_y, final_z, NULL);

    int global_found = 0;

    // このループは将来的に '#pragma omp parallel for' で安全に割当て可能
    for (unsigned long s_ui = 1; s_ui <= s_max; s_ui++) {
        
        // 早期離脱フラグのチェック（他スレッドで見つかっていたらスキップ）
        if (global_found) break; 

        // 各ループ（タスク）ごとに独立した変数を用意して渡す（メモリ安全の核心）
        mpz_t local_x, local_y, local_z;
        mpz_inits(local_x, local_y, local_z, NULL);

        if (check_single_s(local_x, local_y, local_z, s_ui, n_val)) {
            // クリティカルセクション（一意の書き込み）
            if (!global_found) {
                mpz_set(final_x, local_x);
                mpz_set(final_y, local_y);
                mpz_set(final_z, local_z);
                global_found = 1;
            }
        }

        mpz_clears(local_x, local_y, local_z, NULL);

        if (s_ui % 100000 == 0) {
            printf("s = %lu までチェック完了 (数論フィルタにより大半をスキップ済)...\n", s_ui);
        }
    }

    if (global_found) {
        printf("\n★【解発見】★\n");
        gmp_printf("x = %Zd\n", final_x);
        gmp_printf("y = %Zd\n", final_y);
        gmp_printf("z = %Zd\n", final_z);

        // 数学的一致性の検証
        mpz_t x3, y3, z3, sum;
        mpz_inits(x3, y3, z3, sum, NULL);
        mpz_pow_ui(x3, final_x, 3);
        mpz_pow_ui(y3, final_y, 3);
        mpz_pow_ui(z3, final_z, 3);
        mpz_add(sum, x3, y3);
        mpz_add(sum, sum, z3);
        gmp_printf("検証: x^3 + y^3 + z^3 = %Zd\n", sum);
        mpz_clears(x3, y3, z3, sum, NULL);
    } else {
        printf("指定された範囲内に解は見つかりませんでした。\n");
    }

    mpz_clears(final_x, final_y, final_z, NULL);
}

int main() {
    // s の上限を 2,000,000 に設定してテスト
    search_three_cubes_advanced(114, 2000000);
    return 0;
}

