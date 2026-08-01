
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

#define MAX_PRIMES 70000

// ASN.1の動的バッファ構造体
typedef struct {
    unsigned char *data;
    size_t length;
    size_t capacity;
} Buffer;

void buf_init(Buffer *buf) {
    buf->capacity = 4096;
    buf->length = 0;
    buf->data = malloc(buf->capacity);
}

void buf_free(Buffer *buf) {
    free(buf->data);
}

void buf_append_byte(Buffer *buf, unsigned char byte) {
    if (buf->length >= buf->capacity) {
        buf->capacity *= 2;
        buf->data = realloc(buf->data, buf->capacity);
    }
    buf->data[buf->length++] = byte;
}

void buf_append_mem(Buffer *buf, const unsigned char *src, size_t len) {
    while (buf->length + len > buf->capacity) {
        buf->capacity *= 2;
        buf->data = realloc(buf->data, buf->capacity);
    }
    memcpy(buf->data + buf->length, src, len);
    buf->length += len;
}

// DERの長さフィールドをエンコードする関数
void encode_length(Buffer *buf, size_t len) {
    if (len < 128) {
        buf_append_byte(buf, (unsigned char)len);
    } else {
        unsigned char bytes[8];
        int n = 0;
        size_t temp = len;
        while (temp > 0) {
            bytes[n++] = (unsigned char)(temp & 0xFF);
            temp >>= 8;
        }
        buf_append_byte(buf, (unsigned char)(0x80 | n));
        for (int i = n - 1; i >= 0; i--) {
            buf_append_byte(buf, bytes[i]);
        }
    }
}

// GMPのmpz_tをASN.1 INTEGER型(0x02)としてエンコードする関数
void encode_integer(Buffer *buf, mpz_t val) {
    buf_append_byte(buf, 0x02); // Tag for INTEGER
    
    // GMPの機能で極性を考慮したバイト列を出力
    size_t count = 0;
    unsigned char *temp_bytes = mpz_export(NULL, &count, 1, 1, 1, 0, val);
    
    // 0x00の場合の例外処理
    if (count == 0) {
        encode_length(buf, 1);
        buf_append_byte(buf, 0x00);
        free(temp_bytes);
        return;
    }

    // 最上位ビットが1の場合は、正の数を維持するために0x00のパディングが必要（DER規則）
    int pad = (temp_bytes[0] & 0x80) ? 1 : 0;
    
    encode_length(buf, count + pad);
    if (pad) {
        buf_append_byte(buf, 0x00);
    }
    buf_append_mem(buf, temp_bytes, count);
    free(temp_bytes);
}

// 複数のDERブロックを包むSEQUENCE型(0x30)を作成する関数
void wrap_sequence(Buffer *dest, Buffer *src) {
    buf_append_byte(dest, 0x30); // Tag for SEQUENCE
    encode_length(dest, src->length);
    buf_append_mem(dest, src->data, src->length);
}

// 鍵パラメータの計算とASN.1構造の組み立て
void generate_rsa_der(mpz_t *primes, int count, mpz_t e, const char *output_filename) {
    mpz_t n, phi, d, tmp, current_mod;
    mpz_t d_params[MAX_PRIMES];
    mpz_t inv_params[MAX_PRIMES];

    mpz_inits(n, phi, d, tmp, current_mod, NULL);
    mpz_set_ui(n, 1);
    mpz_set_ui(phi, 1);

    // 1. 各種鍵パラメータ計算
    for (int i = 0; i < count; i++) {
        mpz_mul(n, n, primes[i]);
        mpz_sub_ui(tmp, primes[i], 1);
        mpz_mul(phi, phi, tmp);
    }

    if (mpz_invert(d, e, phi) == 0) {
        fprintf(stderr, "Error: e and phi are not coprime.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        mpz_init(d_params[i]);
        mpz_sub_ui(tmp, primes[i], 1);
        mpz_mod(d_params[i], d, tmp);
    }

    mpz_init(inv_params[0]); 
    mpz_init(inv_params[1]); // q_inv = q^-1 mod p
    mpz_invert(inv_params[1], primes[1], primes[0]);

    mpz_mul(current_mod, primes[0], primes[1]);
    for (int i = 2; i < count; i++) {
        mpz_init(inv_params[i]);
        mpz_invert(inv_params[i], current_mod, primes[i]);
        mpz_mul(current_mod, current_mod, primes[i]);
    }

    // 2. ASN.1 DER エンコードの開始
    Buffer inner_seq, final_der;
    buf_init(&inner_seq);
    buf_init(&final_der);

    // Version: マルチプライム(3素数以上)の場合は 1 を指定 (RFC 8017)
    mpz_t version;
    mpz_init_set_ui(version, 1);
    encode_integer(&inner_seq, version);
    mpz_clear(version);

    // 基本パラメータの追加 (n, e, d, p, q, d_p, d_q, q_inv)
    encode_integer(&inner_seq, n);
    encode_integer(&inner_seq, e);
    encode_integer(&inner_seq, d);
    encode_integer(&inner_seq, primes[0]); // p
    encode_integer(&inner_seq, primes[1]); // q
    encode_integer(&inner_seq, d_params[0]); // d_p
    encode_integer(&inner_seq, d_params[1]); // d_q
    encode_integer(&inner_seq, inv_params[1]); // q_inv

    // 3つ目以降の素数情報を格納する OtherPrimeInfos (SEQUENCE OF OtherPrimeInfo) の構築
    if (count > 2) {
        Buffer other_primes_seq;
        buf_init(&other_primes_seq);

        for (int i = 2; i < count; i++) {
            Buffer single_prime_info;
            buf_init(&single_prime_info);

            // 各要素の追加: prime (r, s...), exponent (d_r, d_s...), coefficient (r_inv, s_inv...)
            encode_integer(&single_prime_info, primes[i]);
            encode_integer(&single_prime_info, d_params[i]);
            encode_integer(&single_prime_info, inv_params[i]);

            // 各素数の情報をSEQUENCEでラップし、OtherPrimeInfosに追加
            wrap_sequence(&other_primes_seq, &single_prime_info);
            buf_free(&single_prime_info);
        }
        
        // OtherPrimeInfos全体をSEQUENCEとしてメインのシーケンスに連結
        wrap_sequence(&inner_seq, &other_primes_seq);
        buf_free(&other_primes_seq);
    }

    // 最終的な RSAPrivateKey シーケンスの構築
    wrap_sequence(&final_der, &inner_seq);

    // 3. ファイルへの書き出し
    FILE *out = fopen(output_filename, "wb");
    if (out) {
        fwrite(final_der.data, 1, final_der.length, out);
        fclose(out);
        printf("Success: ASN.1 DER private key saved to '%s' (%zu bytes)\n", output_filename, final_der.length);
    } else {
        perror("Failed to open output file");
    }

    // 後始末
    buf_free(&inner_seq);
    buf_free(&final_der);
    mpz_clears(n, phi, d, tmp, current_mod, NULL);
    for (int i = 0; i < count; i++) {
        mpz_clear(d_params[i]);
        mpz_clear(inv_params[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <primes_file.txt> <output_key.der>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Failed to open primes file");
        return 1;
    }

    mpz_t primes[MAX_PRIMES];
    int count = 0;

    while (count < MAX_PRIMES) {
        mpz_init(primes[count]);
        if (mpz_inp_str(primes[count], fp, 10) > 0) {
            count++;
        } else {
            mpz_clear(primes[count]);
            break;
        }
    }
    fclose(fp);

    if (count < 4) {
        fprintf(stderr, "Error: Multi-prime RSA requires at least 4 primes. (Found: %d)\n", count);
        for(int i = 0; i < count; i++) mpz_clear(primes[i]);
        return 1;
    }

    mpz_t e;
    mpz_init_set_ui(e, 65537);

    generate_rsa_der(primes, count, e, argv[2]);

    mpz_clear(e);
    for (int i = 0; i < count; i++) {
        mpz_clear(primes[i]);
    }

    return 0;
}

