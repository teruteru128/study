
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

int main(int argc, char **argv)
{
  struct curl_slist *list = curl_slist_append(NULL, "Accept-Language: ja,en-US;q=0.7,en;q=0.3");
  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
  curl_easy_setopt(curl, CURLOPT_PROXY, "socks5h://localhost:9050");
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:65.0) Gecko/20100101 Firefox/68.11.0esr");
#if 1
  curl_easy_setopt(curl, CURLOPT_URL, "http://xiwayy2kn32bo3ko.onion/tor/dat/1525034784.dat");
#else
  curl_easy_setopt(curl, CURLOPT_URL, "http://2ayu6gqru3xzfzbvud64ezocamykp56kunmkzveqmuxvout2yubeeuad.onion:8000/");
#endif
  curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  curl_slist_free_all(list);
  return EXIT_SUCCESS;
}
