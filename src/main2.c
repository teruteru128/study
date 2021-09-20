
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "timeutil.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 *
 * 対称鍵暗号,EVP_CIPHER:https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
 * 認証付き暗号:https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
 * エンベロープ暗号化(ハイブリッド暗号？):https://wiki.openssl.org/index.php/EVP_Asymmetric_Encryption_and_Decryption_of_an_Envelope
 * 署名と検証,EVP_DigestSign:https://wiki.openssl.org/index.php/EVP_Signing_and_Verifying
 * メッセージダイジェスト:https://wiki.openssl.org/index.php/EVP_Message_Digests
 * 鍵合意(鍵交換),EVP_PKEY_CTX,EVP_PKEY_derive:https://wiki.openssl.org/index.php/EVP_Key_Derivation
 * EVPとRSAを使って署名/検証
 * EVP+ed25519(EdDSA)
 * EVP+X25519(EdDH)
 * EVP+ChaCha20/Poly1305
 * メッセージ認証符号 (OpenSSL 3～),EVP_MAC_new_ctx
 * 鍵導出関数,EVP_KDF:https://wiki.openssl.org/index.php/EVP_Key_Derivation
 * strsep,トークン分割(空フィールド対応版)
 * versionsort
 * strverscmp
 * alphasort
 * tor geoip file 読み込み関数
 * geoip_load_file
 * https://youtu.be/MCzo6ZMfKa4
 * ターミュレーター
 *
 * regex.h
 * - 最左最短一致
 * - 否定先読み
 * - 強欲な数量子
 *
 * TODO: P2P地震情報 ピア接続受け入れ＆ピアへ接続
 *
 * 標準入力と標準出力を別スレッドで行うアプリ
 * リクエストを投げると適当なデータを投げ返す簡単なサーバープログラム
 * リクエスト長さは8バイトに対応
 *
 * --help
 * --version
 * --server-mode
 *   フォアグラウンドで起動
 * --daemon-mode
 *   デーモン化処理付きでバックグラウンドで起動
 *
 * 複数スレッドをpthread_cond_tで止めてメインスレッドでtimerfdを使って指定時刻まで待ち、pthread_cond_broadcastで一斉に起動する
 *
 * ファイルからGMPのmpzに整数を読み込んだりOpenSSLのBIGNUMに整数を読み込んだり乱数を読み込んだりを共通化したい
 */
/**
 * @brief sanbox func.
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    (void)argc, (void)argv;
    int ret = EXIT_SUCCESS;

    unsigned char *publickey = malloc(4831838208UL);
    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *ctx_ma = EVP_MD_CTX_new();
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];

    FILE *fin = fopen("publicKeys-aligned.bin", "rb");
    size_t items = fread(publickey, 72, 0x4000000UL, fin);
    if (items != 0x4000000UL)
    {
        fclose(fin);
        free(publickey);
        return EXIT_FAILURE;
    }
    fclose(fin);

    size_t nlz_count[21] = { 0 };

    size_t i;
    size_t j;
    size_t ii;
    size_t jj;
    size_t iii;
    size_t jjj;
    size_t ii_max = 0;
    size_t jj_max = 0;
    size_t iii_max = 0;
    size_t jjj_max = 0;
    for (i = 0; i < 4831838208UL; i += 0x480UL)
    {
        ii_max = i + 0x480UL;
        for (j = 0; j < 4831838208UL; j += 0x480UL)
        {
            jj_max = j + 0x480UL;
            for (iii = ii; iii < ii_max; iii += 0x48UL)
            {
                EVP_DigestInit(ctx_ma, sha512);
                EVP_DigestUpdate(ctx_ma, publickey + iii, 65);
                for (jjj = jj; jjj < jj_max; jjj += 0x48UL)
                {
                    EVP_MD_CTX_copy(ctx, ctx_ma);
                    EVP_DigestUpdate(ctx, publickey + jjj, 65);
                    EVP_DigestFinal(ctx, hash, NULL);
                    EVP_DigestInit(ctx, ripemd160);
                    EVP_DigestUpdate(ctx, hash, 64);
                    EVP_DigestFinal(ctx, hash, NULL);
                    nlz_count[__builtin_clzl(*(unsigned long *)hash) >> 3]++;
                }
            }
        }
    }
    for (size_t k = 0; k < 21; k++)
    {
        if (k != 0)
            fputs(",", stdout);
        printf("%zu", nlz_count[k]);
    }
    fputs("\n", stdout);
    EVP_MD_CTX_free(ctx);
    EVP_MD_CTX_free(ctx_ma);

    free(publickey);

    return ret;
}
