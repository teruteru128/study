
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <changebase.h>
#include <gmp.h>
#include <math.h>
#include <openssl/evp.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

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
int main(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm result;
    struct tm *t = localtime_r(&ts.tv_sec, &result);
    printf("%d, %p\n", t == &result, (void *)t);
    printf("%ld\n", (ts.tv_sec + result.tm_gmtoff) % 86400L);
    printf("%ld.%09ld\n", ts.tv_sec, ts.tv_nsec);
    printf("%02d:%02d:%02d, %ld, %s\n", result.tm_hour, result.tm_min,
           result.tm_sec, result.tm_gmtoff, result.tm_zone);
    // 毎秒ループして(ts.tv_sec + result.tm_gmtoff) %
    // 86400Lが12840(時間が3時34分)だったらなんでや！する、もしくは (result.tm_hour == 3
    // && result.tm_min == 34 && result.tm_sec == 0) だったら334する
    return EXIT_SUCCESS;
}
