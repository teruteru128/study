
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

// Base64デコード（バッファを動的に確保して返す）
unsigned char* base64_decode(const char* input, int* out_len) {
    int input_len = strlen(input);
    // デコード後のサイズは最大でも入力の3/4
    unsigned char* buffer = (unsigned char*)malloc(input_len);
    if (!buffer) return NULL;

    BIO *bio, *b64;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(input, input_len);
    bio = BIO_push(b64, bio);
    
    *out_len = BIO_read(bio, buffer, input_len);
    BIO_free_all(bio);
    
    if (*out_len <= 0) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

void decrypt_backup(const char* filename, const char* password) {
    struct json_object *root = json_object_from_file(filename);
    if (!root) {
        fprintf(stderr, "Error: JSON file not found or invalid format.\n");
        return;
    }

    struct json_object *kdf_obj, *cipher_obj, *salt_obj, *iter_obj, *iv_obj, *ct_obj;
    // 構造を辿って値を取得
    if (!json_object_object_get_ex(root, "kdf", &kdf_obj) ||
        !json_object_object_get_ex(kdf_obj, "salt", &salt_obj) ||
        !json_object_object_get_ex(kdf_obj, "iterations", &iter_obj) ||
        !json_object_object_get_ex(root, "cipher", &cipher_obj) ||
        !json_object_object_get_ex(cipher_obj, "iv", &iv_obj) ||
        !json_object_object_get_ex(root, "ciphertext", &ct_obj)) {
        fprintf(stderr, "Error: Required JSON fields are missing.\n");
        json_object_put(root);
        return;
    }

    int salt_len, iv_len, ct_len;
    unsigned char *salt = base64_decode(json_object_get_string(salt_obj), &salt_len);
    unsigned char *iv = base64_decode(json_object_get_string(iv_obj), &iv_len);
    unsigned char *ciphertext = base64_decode(json_object_get_string(ct_obj), &ct_len);
    int iterations = json_object_get_int(iter_obj);

    if (!salt || !iv || !ciphertext) {
        fprintf(stderr, "Error: Base64 decoding failed.\n");
        goto cleanup;
    }

    // 鍵生成 (AES-256)
    unsigned char key[32];
    if (!PKCS5_PBKDF2_HMAC(password, strlen(password), salt, salt_len, iterations, EVP_sha256(), 32, key)) {
        fprintf(stderr, "Error: Key generation failed.\n");
        goto cleanup;
    }

    // 復号
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char *plaintext = malloc(ct_len + 1);
    int outlen = 0, finalen = 0;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);

    // AES-GCMの仕様: ciphertextの末尾16バイトがTag
    int tag_len = 16;
    int data_len = ct_len - tag_len;
    if (data_len < 0) {
        fprintf(stderr, "Error: Ciphertext too short.\n");
        goto cipher_cleanup;
    }

    EVP_DecryptUpdate(ctx, plaintext, &outlen, ciphertext, data_len);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag_len, ciphertext + data_len);

    if (EVP_DecryptFinal_ex(ctx, plaintext + outlen, &finalen) > 0) {
        plaintext[outlen + finalen] = '\0';
        printf("--- DECRYPTED ---\n%s\n", plaintext);
    } else {
        printf("Decryption Failed: Verification error (wrong password?).\n");
    }

cipher_cleanup:
    EVP_CIPHER_CTX_free(ctx);
    free(plaintext);

cleanup:
    if (salt) free(salt);
    if (iv) free(iv);
    if (ciphertext) free(ciphertext);
    json_object_put(root);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <json_file> <password>\n", argv[0]);
        return 1;
    }
    decrypt_backup(argv[1], argv[2]);
    return 0;
}
