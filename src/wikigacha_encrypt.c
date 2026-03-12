
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>

// Base64エンコード
char* base64_encode(const unsigned char* input, int length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    char* b64text = strndup(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return b64text;
}

void encrypt_to_backup(const char* input_json_file, const char* password) {
    // 1. 入力データの読み込み
    FILE *f = fopen(input_json_file, "rb");
    if (!f) { perror("Input file error"); return; }
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    unsigned char *plaintext = malloc(len + 1);
    fread(plaintext, 1, len, f); fclose(f);
    plaintext[len] = '\0';

    // 2. パラメータ生成 (Salt: 16byte, IV: 12byte)
    unsigned char salt[16], iv[12], key[32], tag[16];
    RAND_bytes(salt, sizeof(salt));
    RAND_bytes(iv, sizeof(iv));
    int iterations = 210000;

    // 3. 鍵生成 (PBKDF2)
    PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), iterations, EVP_sha256(), 32, key);

    // 4. AES-256-GCM 暗号化
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char *ciphertext = malloc(len + 16); // データ + タグ分
    int outlen = 0, finalen = 0;

    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);
    EVP_EncryptUpdate(ctx, ciphertext, &outlen, plaintext, len);
    EVP_EncryptFinal_ex(ctx, ciphertext + outlen, &finalen);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);

    // Ciphertext + Tag を結合したバッファを作成
    int total_ct_len = outlen + finalen + 16;
    unsigned char *combined_ct = malloc(total_ct_len);
    memcpy(combined_ct, ciphertext, outlen + finalen);
    memcpy(combined_ct + outlen + finalen, tag, 16);

    // 5. JSON構築 (libjson-c)
    struct json_object *root = json_object_new_object();
    struct json_object *kdf = json_object_new_object();
    struct json_object *cipher = json_object_new_object();

    json_object_object_add(root, "format", json_object_new_string("wikigacha-idb-backup"));
    json_object_object_add(root, "version", json_object_new_int(1));

    json_object_object_add(kdf, "name", json_object_new_string("PBKDF2"));
    json_object_object_add(kdf, "hash", json_object_new_string("SHA-256"));
    json_object_object_add(kdf, "iterations", json_object_new_int(iterations));
    json_object_object_add(kdf, "salt", json_object_new_string(base64_encode(salt, 16)));
    json_object_object_add(root, "kdf", kdf);

    json_object_object_add(cipher, "name", json_object_new_string("AES-GCM"));
    json_object_object_add(cipher, "iv", json_object_new_string(base64_encode(iv, 12)));
    json_object_object_add(root, "cipher", cipher);

    json_object_object_add(root, "ciphertext", json_object_new_string(base64_encode(combined_ct, total_ct_len)));

    // 標準出力にJSONを表示
    printf("%s\n", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));

    // クリーンアップ
    EVP_CIPHER_CTX_free(ctx);
    json_object_put(root);
    free(plaintext); free(ciphertext); free(combined_ct);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <decrypted_json_file> <password>\n", argv[0]);
        return 1;
    }
    encrypt_to_backup(argv[1], argv[2]);
    return 0;
}
