
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
#include <time.h>
#include <unistd.h>
#include <wchar.h>

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
    const wchar_t in[] = L"はくらりしひくたひくりのし\nなりすこすなり\nますひ"
                         L"しり\nてすひりさく";
    const wchar_t a[] = L"ぬふあうえおやゆよわほへー"
                        L"たていすかんなにらせ゛゜"
                        L"ちとしはきくまのりれけむ"
                        L"つさそひこみもねるめろ";
    const char b[] = "1234567890-^\\qwertyuiop@[asdfghjkl;:]zxcvbnm,./\\";

    size_t max_len = wcslen(in);
    size_t j = 0;
    for (size_t i = 0; i < max_len; i++)
    {
        if (in[i] == L'\n')
        {
            wprintf(L"\n");
            continue;
        }
        for (j = 0; j < 49; j++)
        {
            if (in[i] == a[j])
            {
                break;
            }
        }
        wprintf(L"%c", (b[j]-3) < 'a' ? (b[j]-3) + 26:(b[j]-3));
    }
    wprintf(L"\n");
    return EXIT_SUCCESS;
}
