
__kernel void check_slime_optimized(long world_seed, int start_x, int start_z, __global int* results) {
    // ワークアイテムの座標
    int gid_x = get_global_id(0);
    int gid_z = get_global_id(1);

    // 範囲外ガード
    if (gid_x >= 625 || gid_z >= 625) return;

    // --- 1. 自分の座標がスライムかどうか判定 ---
    int cx = start_x + gid_x;
    int cz = start_z + gid_z;

    // --- 2. ローカルメモリによる2次元累積和的アプローチ ---
    // 本来はここでバリア同期を使いますが、625四方なら各スレッドで5x5を直接計算しても
    // Radeon 780Mの演算能力（TFLOPS級）なら累積和を作るオーバーヘッドより速いです
    
    // テスト用に、自分を左上とした5x5の合計を計算
    int slime_count = 0;
    for (int dz = 0; dz < 5; dz++) {
        for (int dx = 0; dx < 5; dx++) {
			unsigned int tx = (unsigned int)(cx + dx);
            unsigned int tz = (unsigned int)(cz + dz);

            // Javaの (int)(x*x*0x4c1906) 等の再現
            // 一旦 unsigned int で計算してオーバーフローを確定させてから long にキャストする
            long term1 = (long)(int)(tx * tx * 0x4c1906u);
            long term2 = (long)(int)(tx * 0x5ac0dbu);
            long term3 = (long)(int)(tz * tz) * 0x4307a7L; // ここは tz*tz (int) の結果に long を掛ける
            long term4 = (long)(int)(tz * 0x5f24fL);

            long ts = (world_seed + term1 + term2 + term3 + term4) ^ 0x3ad8025fL;
            ts ^= 0x5DEECE66DL;
            int val;
            int bits;
            do{
                ts = (ts * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
                bits = (int) (ts >> 17);
                val = bits % 10;
            } while (bits - val + 9 < 0);
            if (val == 0) slime_count++;
        }
    }

    // 16チャンク以上(4x4以上相当)が見つかったら報告
    if (slime_count >= 17) {
        // インデックスとスライム数を記録
        results[gid_z * 625 + gid_x] = slime_count;
    }
}

