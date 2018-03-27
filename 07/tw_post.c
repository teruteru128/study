
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <oauth.h>
#include <curl/curl.h>

#define CONTENT_SIZE 141
#define TWEET_URL "https://api.twitter.com/1.1/statuses/update.json"
#define KEY_STATUS "status="

typedef struct {
    char *content;
    char *url_enc_args;
    char *postdata;
    int argc;
    char **argv;
    struct curl_slist *header;
} twitter;

static int create_twitter(twitter *self) {
    //Initialize instance
    *self = (twitter) {
        .content = "test tweet",
        .url_enc_args = NULL,
        .argc = 0,
        .argv = malloc(0),
        .header = NULL,
        .postdata = NULL,
    };

    if (self->argv == NULL) {
        return -1;
    }

    return 0;
}

static int create_base_arg(twitter *self, const char *url) {
    size_t ser_url_size = strlen(url) + strlen(self->url_enc_args) + 2;
    char ser_url[ser_url_size];
    int r = snprintf(ser_url, ser_url_size, "%s?%s", url, self->url_enc_args);
    if (r < 0) {
        return r;
    }
    free(self->url_enc_args);
    self->url_enc_args = NULL;

    //Get user arguments
    self->argc = oauth_split_url_parameters(ser_url, &self->argv);
    //Get OAuth arguments
    char *tmp_url = oauth_sign_array2(
                        &self->argc,
                        &self->argv,
                        NULL,
                        OA_HMAC,
                        "POST",
                        "XXXXXXXXXXXXXXXXXXXXXXXXX",//consumer_key
                        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",//consumer_secret
                        "XXXXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",//access_token
                        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"//access_token_secret
                    );
    if (tmp_url == NULL) {
        return -1;
    }
    free(tmp_url);

    return 0;
}

static int create_encoded_tweet(twitter *self) {
    size_t size = sizeof KEY_STATUS + CONTENT_SIZE;
    char string[size];
    string[0] = '\0';
    strncat(string, KEY_STATUS, size);
    strncat(string, self->content, size);
    self->url_enc_args = oauth_url_escape(string);
    return self->url_enc_args == NULL ? -1 : 0;
}

static int create_request_header(twitter *self) {
    char *auth_params = oauth_serialize_url_sep(self->argc, 1, self->argv, ", ", 6);
    if (auth_params == NULL) {
        return -1;
    }
    char auth_header[4096];
    sprintf(auth_header, "Authorization: OAuth %s", auth_params);
    free(auth_params);

    self->header = curl_slist_append(self->header, auth_header);
    if (self->header == NULL) {
        return -1;
    }
    return 0;
}

static int create_postdata(twitter *self) {
    self->postdata = oauth_serialize_url_sep(self->argc, 1, self->argv, "&", 1);
    if (self->postdata == NULL) {
        return -1;
    }
    return 0;
}

static int send_post_request(CURL *handle, twitter *self, const char *url,
                             size_t (*write_callback)(char *ptr, size_t size, size_t nmemb, void *userdata)) {
    int r = 0;

    //Set postdata
    r = curl_easy_setopt(handle, CURLOPT_POSTFIELDS, self->postdata);
    if (r != CURLE_OK) {
        return r;
    }
    //Set request header
    r = curl_easy_setopt(handle, CURLOPT_HTTPHEADER, self->header);
    if (r != CURLE_OK) {
        return r;
    }
    r = curl_easy_setopt(handle, CURLOPT_URL, url);
    if (r != CURLE_OK) {
        return r;
    }
    r = curl_easy_setopt(handle, CURLOPT_POST, 1);
    if (r != CURLE_OK) {
        return r;
    }
    r = curl_easy_setopt(handle, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    if (r != CURLE_OK) {
        return r;
    }
    r = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
    if (r != CURLE_OK) {
        return r;
    }
    r = curl_easy_perform(handle);
    if (r != CURLE_OK) {
        return r;
    }
    //Check response code
    long rc = 0;
    r = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &rc);
    if (r != CURLE_OK) {
        return r;
    }
    if (rc / 100 != 2) {
        return -1;
    }

    return 0;
}

static size_t post_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t r = size * nmemb;
    printf("Posted to twitter. Response size: %zu\n", r);
    printf("%s\n", ptr);
    return r;
}

static void free_argv(twitter *self) {
    for (int i = 0; i < self->argc; i++) {
        free(self->argv[i]);
    }
    free(self->argv);
    self->argc = 0;
    self->argv = NULL;
}

static void free_twitter(twitter *self) {
    free_argv(self);
    free(self->url_enc_args);
    free(self->postdata);
    curl_slist_free_all(self->header);
}

static int tweet() {
    twitter t;
    int r = create_twitter(&t);
    if (r != 0) {
        free_twitter(&t);
        return r;
    }

    //Create URL encoded user arguments string
    r = create_encoded_tweet(&t);
    if (r != 0) {
        free_twitter(&t);
        return -1;
    }

    //Create base arg for header and postdata
    r = create_base_arg(&t, TWEET_URL);
    if (r != 0) {
        free_twitter(&t);
        return r;
    }

    //Create request header
    r = create_request_header(&t);
    if (r != 0) {
        free_twitter(&t);
        return r;
    }

    //Create postdata
    r = create_postdata(&t);
    if (r != 0) {
        free_twitter(&t);
        return r;
    }

    //Free OAuth arguments
    free_argv(&t);

    //Tweet with curl
    CURL *handle = curl_easy_init();
    if (handle == NULL) {
        free_twitter(&t);
        return -1;
    }
    r = send_post_request(handle, &t, TWEET_URL, post_callback);

    //Free and clean up
    free_twitter(&t);
    curl_easy_cleanup(handle);

    return r;
}


int main(int argc, char *argv[]) {
    int r = curl_global_init(CURL_GLOBAL_ALL);
    if (r != 0) {
        return EXIT_FAILURE;
    }

    r = tweet();
    curl_global_cleanup();

    return r != 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
