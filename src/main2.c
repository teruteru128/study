
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <bm.h>
#include <changebase.h>
#include <gmp.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <netdb.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <printaddrinfo.h>
#include <regex.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

int readprivkey(PrivateKey *pkey, size_t index)
{
    size_t fileindex = index >> 24;
    char filename[PATH_MAX];
    snprintf(filename, PATH_MAX, "privateKeys%ld.bin", fileindex);

    FILE *fin = fopen(filename, "rb");
    if (fin == NULL)
    {
        return EXIT_FAILURE;
    }

    long seekpos = (long)(index & 0xFFFFFFUL) * PRIVATE_KEY_LENGTH;

    if (fseek(fin, seekpos, SEEK_SET) != 0)
    {
        perror("fseek");
        fclose(fin);
        return EXIT_FAILURE;
    }

    if (fread(pkey, PRIVATE_KEY_LENGTH, 1, fin) != 1)
    {
        perror("fread");
        fclose(fin);
        return EXIT_FAILURE;
    }

    fclose(fin);
    return EXIT_SUCCESS;
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
    if (argc != 3)
    {
        return 1;
    }

    size_t signindex = strtoul(argv[1], NULL, 10);
    size_t encindex = strtoul(argv[2], NULL, 10);

    PrivateKey signprivkey = "";
    PrivateKey encprivkey = "";

    if (readprivkey(&signprivkey, signindex) != 0)
    {
        perror("readprivkey 1");
        return EXIT_FAILURE;
    }
    if (readprivkey(&encprivkey, encindex) != 0)
    {
        perror("readprivkey 2");
        return EXIT_FAILURE;
    }

    PublicKey signpubkey = "";
    PublicKey encpubkey = "";

    getPublicKey(&signpubkey, &signprivkey);
    getPublicKey(&encpubkey, &encprivkey);

    unsigned char hash[EVP_MAX_MD_SIZE];

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();

    const EVP_MD *sha512 = EVP_sha512();
    const EVP_MD *ripemd160 = EVP_ripemd160();

    EVP_DigestInit(ctx, sha512);
    EVP_DigestUpdate(ctx, signpubkey, PUBLIC_KEY_LENGTH);
    EVP_DigestUpdate(ctx, encpubkey, PUBLIC_KEY_LENGTH);
    EVP_DigestFinal(ctx, hash, NULL);
    EVP_DigestInit(ctx, ripemd160);
    EVP_DigestUpdate(ctx, hash, 64);
    EVP_DigestFinal(ctx, hash, NULL);

    EVP_MD_CTX_free(ctx);

    char *signwif = encodeWIF(&signprivkey);
    char *encwif = encodeWIF(&encprivkey);

    char *address = encodeV4Address(hash, 20);

    char *format = formatKey(address, signwif, encwif);

    free(address);
    printf("%s\n", format);
    free(format);

    return EXIT_SUCCESS;
}
