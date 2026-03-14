
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <json-c/json.h>
#include <curl/curl.h>

struct Memory { char *response; size_t size; };

static size_t write_cb(void *data, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;
    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if (!ptr) return 0;
    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
    return realsize;
}

void enrich_card(int card_id, struct json_object *card_obj, const char* lang) {
    CURL *curl;
    struct Memory chunk;
    char url[256];
    // 正しいURL形式に修正
    snprintf(url, sizeof(url), "https://wikigacha.com/api/card?id=%d&lang=%s", card_id, lang);

    int success = 0;
    while (!success) {
        chunk.response = malloc(1);
        chunk.size = 0;
        curl = curl_easy_init();
        if (!curl) return;

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "wikigacha-enrich-tool/1.2");

        CURLcode res = curl_easy_perform(curl);
        long http_code = 0;
        char *content_type = NULL;
        
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);

        if (res == CURLE_OK && http_code == 200) {
            // Content-Type が application/json で始まるか確認
            if (content_type && strstr(content_type, "application/json") != NULL) {
                struct json_object *api_res = json_tokener_parse(chunk.response);
                if (api_res) {
                    struct json_object *major, *minor;
                    if (json_object_object_get_ex(api_res, "major_category", &major))
                        json_object_object_add(card_obj, "major_category", json_object_get(major));
                    if (json_object_object_get_ex(api_res, "minor_category", &minor))
                        json_object_object_add(card_obj, "minor_category", json_object_get(minor));
                    json_object_put(api_res);
                    success = 1;
                } else {
                    fprintf(stderr, "\n[Error] JSON Parse Failed for ID %d\n", card_id);
                    success = 1; // 不正なJSONの場合はスキップ
                }
            } else {
                fprintf(stderr, "\n[Error] Invalid Content-Type: %s (ID %d)\n", content_type ? content_type : "none", card_id);
                success = 1; // JSON以外が返ってきたらスキップ
            }
        } else if (http_code == 429) {
            curl_off_t retry_after = 0;
            curl_easy_getinfo(curl, CURLINFO_RETRY_AFTER, &retry_after);
            if (retry_after <= 0) retry_after = 60;
            fprintf(stderr, "\n[Rate Limit] 429. Sleeping %lds...\n", (long)retry_after);
            sleep((unsigned int)retry_after);
        } else {
            fprintf(stderr, "\n[Error] HTTP %ld / ID %d. Skipping...\n", http_code, card_id);
            success = 1;
        }

        curl_easy_cleanup(curl);
        free(chunk.response);
    }
}

void process_array(struct json_object *root, const char* key, const char* lang) {
    struct json_object *array;
    if (!json_object_object_get_ex(root, key, &array)) return;

    int n = json_object_array_length(array);
    fprintf(stderr, "\n--- Start: %s (%d cards) ---\n", key, n);

    for (int i = 0; i < n; i++) {
        struct json_object *card = json_object_array_get_idx(array, i);
        struct json_object *id_obj, *major_obj;
        json_object_object_get_ex(card, "id", &id_obj);
        int card_id = json_object_get_int(id_obj);

        // 既存のカテゴリがあればスキップ（再開時に便利）
        if (!json_object_object_get_ex(card, "major_category", &major_obj)) {
            fprintf(stderr, "\r[%d/%d] Fetching ID %d... ", i + 1, n, card_id);
            enrich_card(card_id, card, lang);
            usleep(200000); // 0.2s
        }
    }
    fprintf(stderr, "\nFinished %s.\n", key);
}

int main(int argc, char *argv[]) {
    if (argc < 2) { fprintf(stderr, "Usage: %s <decrypted.json>\n", argv[0]); return 1; }

    struct json_object *root = json_object_from_file(argv[1]);
    if (!root) { fprintf(stderr, "Invalid JSON input.\n"); return 1; }

    process_array(root, "cards_jp", "JP");
    process_array(root, "cards_en", "EN");

    // 出力（パイプで受け取れるよう標準出力へ）
    printf("%s\n", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PLAIN));
    
    json_object_put(root);
    return 0;
}

