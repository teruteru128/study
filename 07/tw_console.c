
#include <stdio.h>
#include <oauth.h>
#include <nghttp2/nghttp2.h>
#include <curl/curl.h>

//static const char* request_token_uri = "https://api.twitter.com/oauth/request_token";
//static const char* access_token_uri = "https://api.twitter.com/oauth/access_token";
static const char* update_uri = "https://api.twitter.com/1.1/statuses/update.json";
static const char* c_key = "gcuJV4HNAhbk1FfxoDcLUha8j";
static const char* c_secret = "IxGTRgCgSW26rxS4MhCEK5jnEp18NjLHPpiAsHaZhetWUkdDNg";
static const char* t_key = "533912006-yDjGv6XCMi476XibsGOFc0p8VIzN8mbT8XMSv0F7";
static const char* t_secret = " Jyn4vm2eMmYfP7T6tlTlTH151AfgDkfuaQYVZRTanvsYB";

static int post_tweet(const char* status) {
    char *req_url = NULL;
    char *postarg = NULL;
    CURL *curl;
    CURLcode res;
    req_url = oauth_sign_url2(update_uri, &postarg, OA_HMAC, NULL, c_key, c_secret, t_key, t_secret);
    printf("%s\n", req_url);
    printf("%s\n", postarg);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl){
        curl_easy_setopt(curl, CURLOPT_URL, req_url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postarg);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    if (req_url) free(req_url);
    if (postarg) free(postarg);
    return 0;
}

static int printHelp(const char* name){
    printf("%s \"tweet\"\n", name);
    return 0;
}

int main(int argc, char** argv) {
    if(argc < 2){
        printHelp(argv[0]);
    }
    post_tweet(argv[1]);
}
