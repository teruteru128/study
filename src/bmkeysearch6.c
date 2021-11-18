
#include <assert.h>
#include <bitmessage.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int calchash(EVP_MD_CTX *mdctx, const EVP_MD *sha512,
                    const EVP_MD *ripemd160, char *ekey, char *skey,
                    size_t *casted)
{
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    unsigned int mdlen = 0;
    unsigned long tmp = 0;

    EVP_DigestInit(mdctx, sha512);
    EVP_DigestUpdate(mdctx, ekey, 65);
    EVP_DigestUpdate(mdctx, skey, 65);
    EVP_DigestFinal(mdctx, hash, &mdlen);
    assert(mdlen == 64);
    EVP_DigestInit(mdctx, ripemd160);
    EVP_DigestUpdate(mdctx, hash, 64);
    EVP_DigestFinal(mdctx, hash, &mdlen);
    assert(mdlen == 20);
    *casted = htobe64(*(unsigned long *)hash);
    return 0;
}

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

    FILE *fin = fopen("publicKeys.bin", "rb");

    unsigned char key1[63 * 65] = "";
    unsigned char key2[63 * 65] = "";

    size_t t = fread(key1, 65, 63, fin);

    if (t < 63)
    {
        perror("fread");
        fclose(fin);
        return EXIT_FAILURE;
    }

    fclose(fin);

    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    unsigned int mdlen = 0;
    unsigned long tmp = 0;

    size_t counts[21] = { 0 };

    for (size_t i = 0; i < 4095; i += 65)
    {
        for (size_t j = 0; j < 4095; j += 65)
        {
            // TODO: 関数化
            calchash(mdctx, sha512, ripemd160, key1 + i, key1 + j, &tmp);
            counts[((tmp == 0) ? 64UL : (size_t)__builtin_clzl(tmp)) >> 3]++;
        }
    }
    //(void)fseek(fin, 63L * 0, SEEK_SET);
    t = fread(key2, 65, 63, fin);
    for (size_t i = 0; i < 4095; i += 65)
    {
        for (size_t j = 0; j < 4095; j += 65)
        {
            calchash(mdctx, sha512, ripemd160, key1 + i, key2 + j, &tmp);
            counts[((tmp == 0) ? 64UL : (size_t)__builtin_clzl(tmp)) >> 3]++;
        }
    }
    fclose(fin);
    for (size_t i = 0; i < 21; i++)
    {
        printf("%zu\n", counts[i]);
    }

    return ret;
}
