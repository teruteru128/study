#include <stdio.h>
#include <gmp.h>
#include <ecm.h>

int main() {
    // 1. パラメータの準備
    int n_exp = 1148;
    int k_val = 21181;
    
    mpz_t s_n, factor;
    mpz_inits(s_n, factor, NULL);
    
    // S_n = k * 2^n + 1
    mpz_ui_pow_ui(s_n, 2, n_exp);
    mpz_mul_ui(s_n, s_n, k_val);
    mpz_add_ui(s_n, s_n, 1);

    // ecm用のパラメータ設定
    ecm_params params;
    ecm_init(params);
    
    // B1, B2 の設定 (例: B1=2000)
    double b1 = 2000.0;
    
    printf("n=%d の因数探索を開始します...\n", n_exp);

    // 2. ECMの実行
    // ecm_factor(見つかった因数, 対象の数, B1, パラメータ)
    // 戻り値: 1 (Stage 1で発見), 2 (Stage 2で発見), 0 (未発見), <0 (エラー)
    int sigma = 0; // 0を指定すると内部でランダムなシードが生成される
    int res = ecm_factor(factor, s_n, b1, params);

    if (res > 0) {
        gmp_printf("\n[成功] 因数が見つかりました (%s): %Zd\n", 
                   (res == 1 ? "Stage 1" : "Stage 2"), factor);
        
        // ここで残りの数 (s_n / factor) を計算して、
        // さらに分解を続けるか判断する処理を追加可能
    } else {
        printf("\n[失敗] この曲線では因数が見つかりませんでした。\n");
    }

    // 後片付け
    ecm_clear(params);
    mpz_clears(s_n, factor, NULL);
    return 0;
}

