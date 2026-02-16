
#include <curl/curl.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t relsize = size * nmemb;
  fwrite(ptr, size, nmemb, stdout);
  fputs("\n", stdout);
  return relsize;
}

int main(int argc, char *argv[], char *envp[]) {
  const char *fdb_session = getenv("FDB_SESSION_ID");
  if (fdb_session == NULL) {
    fprintf(stderr, "Error: FDB_SESSION_ID is not set.\n");
    return 1;
  }
  char cookie[128];
  snprintf(cookie, 128, "fdbuser=%s", fdb_session);

  mpz_t p, factor1;
  mpz_t hard_limit, soft_limit;
  mpz_t min;
  mpz_t window;
  mpz_init(p);
  mpz_init(factor1);
  mpz_init(hard_limit);
  mpz_set_ui(hard_limit, 10);
  mpz_pow_ui(hard_limit, hard_limit, 299);
  mpz_init(soft_limit);
  mpz_init(min);
  mpz_init(window);
  gmp_randstate_t state;
  gmp_randinit_default(state);
  int current_digits = 300;

  char url_buffer[BUFSIZ];
  char strpbuffer[512];
  int retry_count = 0;
  int max_retries = 3;
  int p_count = 0;
  int max_p = 10000;

  curl_global_init(CURL_GLOBAL_ALL);
  CURL *hnd = curl_easy_init();
  CURLcode res;
  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(hnd, CURLOPT_COOKIE, cookie);
  curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 30L);
  curl_easy_setopt(hnd, CURLOPT_FAILONERROR, 1L);
  size_t length;
  while (current_digits >= 295) {
    // min = hard_limit / 10;
    mpz_fdiv_q_ui(min, hard_limit, 10);
    // soft_limit = prevprime(hard_limit);
    mpz_prevprime(soft_limit, hard_limit);
    // window = soft_limit - min;
    mpz_sub(window, soft_limit, min);
    while (p_count < max_p) {
      mpz_urandomm(p, state, window);
      mpz_add(p, p, min);
      mpz_nextprime(p, p);
      retry_count = 0;
      mpz_get_str(strpbuffer, 10, p);
      length = strlen(strpbuffer);
      // printf("%s is prime\n", strpbuffer);

      snprintf(url_buffer, BUFSIZ, "https://factordb.com/api?query=%s",
               strpbuffer);
      curl_easy_setopt(hnd, CURLOPT_URL, url_buffer);

      do {
        res = curl_easy_perform(hnd);
        if (res == CURLE_OK) {
          break;
        }
        sleep(2 << retry_count);
        retry_count++;
      } while (retry_count < max_retries);
      printf("%d number post(%zu digits)\n", p_count, length);
      p_count++;
    }
    mpz_fdiv_q_ui(hard_limit, hard_limit, 10);
    current_digits--;
    p_count = 0;
  }
  curl_easy_cleanup(hnd);
  curl_global_cleanup();
  gmp_randclear(state);
  mpz_clears(p, factor1, hard_limit, soft_limit, min, window, NULL);
  return 0;
}
