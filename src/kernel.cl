
__kernel void search_perfect_seeds(long start_seed, __global long* found_seeds, __global int* found_count) {
    long world_seed = start_seed + get_global_id(0);
    
    // 625x625の範囲を走査
    for (int cz = -313; cz <= 312; cz++) {
        for (int cx = -313; cx <= 312; cx++) {
            
            // 5x5の範囲がすべてスライムかチェック
            int perfect = 1;
            int count = 0;
            for (int dz = 0; dz < 5; dz++) {
                for (int dx = 0; dx < 5; dx++) {
                    int tx = cx + dx;
                    int tz = cz + dz;
                    
                    // インライン判定ロジック
                    long ts = (world_seed + (long)(int)(tx*tx*0x4c1906u) + (long)(int)(tx*0x5ac0dbu) + 
                               (long)(int)(tz*tz)*0x4307a7L + (long)(int)(tz*0x5f24fL)) ^ 0x3ad8025fL;
                    ts = (ts ^ 0x5DEECE66DL) & ((1L << 48) - 1);
                    ts = (ts * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
                    if (((int)(ts >> 17) % 10) != 0) {
                        //perfect = 0;
                        //break; // 1つでもハズレたらこの座標は終了
                    }else{
                        count++;
                    }
                    if(count - (24 - (dx + 1) * dz - 1) < 18){
                        break;
                    }
                }
                //if (!perfect) break;
            }

            //if (perfect) {
            if (count >= 18) {
                // 発見！アトミックにカウントを増やして記録
                int idx = atomic_inc(found_count);
                if (idx < 1000) { // バッファ溢れ防止
                    found_seeds[idx] = world_seed;
                }
                return; // このシードで見つかったら次のシードへ
            }
        }
    }
}

