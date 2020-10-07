

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>
#include <string.h>
#include <uuid/uuid.h>
#include <bitmessage.h>
#include <base64.h>
#include <bm.h>
#include <bmapi.h>
#include <err.h>
#define localhost_ip "127.0.0.1"
#define bitmessage_port 8442
#define NAME "TR BM TEST CLIENT"
#define SERVER_URL "http://192.168.1.128:8442/"
#define GENELRAL "BM-2cW67GEKkHGonXKZLCzouLLxnLym3azS8r"

void die_if_fault_occurred(xmlrpc_env *env);

#define SENDTO_ADDRESS_FILE "main.txt"
#define SUBJECT "TWVycnkgQ2hyaXN0bWFzIQ=="
#define MESSAGE "PHByZT4KICAgIEBAICAgICAgICAgIEBACiAgICBAQEAgICAgICAgIEBAQAogICAgQEBAQCAgICAg" \
                "IEBAQEAgICBAQEBAICBAQCBAQEAgIEBAIEBAQCBAQCAgICAgIEBACiAgICBAQCBAQCAgICBAQCBA" \
                "QCAgQEAgIEBAIEBAQCAgQEAgQEBAICBAQCBAQCAgICBAQAogICAgQEAgIEBAICBAQCAgQEAgQEBA" \
                "QEBAICBAQCAgICAgIEBAICAgICAgIEBAICBAQAogICAgQEAgICBAQEBAICAgQEAgIEBAICAgICBA" \
                "QCAgICAgIEBAICAgICAgICBAQEBACiAgICBAQCAgICBAQCAgICBAQCAgIEBAQEAgIEBAICAgICAg" \
                "QEAgICAgICAgICBAQAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg" \
                "ICBAQAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgQEAgIEBACiAgICAg" \
                "ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgQEBAQAogICBAQEAgICBAQCAgICAg" \
                "ICAgICAgICAgQEAgICAgICAgICAgQEAKIEBAICAgQEAgQEAgICAgICAgICAgICAgICAgICAgICAg" \
                "ICAgIEBACkBAICAgICAgIEBAIEBAQCAgQEAgQEBAICBAQCAgIEBAQEAgQEBAQEBAICBAQCBAQCAg" \
                "QEAgICAgICBAQEAgICAgQEBAQApAQCAgICAgICBAQEAgIEBAIEBAQCAgQEAgQEAgIEBAICAgICAg" \
                "QEAgICAgQEBAICBAQCAgQEAgIEBAICBAQCAgQEAKQEAgICAgICAgQEAgICBAQCBAQCAgICAgIEBA" \
                "ICAgQEBAICAgIEBAICAgIEBAICAgQEAgIEBAIEBAICAgQEAgICBAQEAKIEBAICAgQEAgQEAgICBA" \
                "QCBAQCAgICAgIEBAICAgICBAQCAgIEBAIEBAIEBAICAgICAgIEBAIEBAICAgQEAgICAgIEBACiAg" \
                "IEBAQCAgIEBAICAgQEAgQEAgICAgICAgQEAgQEBAQCAgICAgQEAgICBAQCAgICAgICBAQCAgQEBA" \
                "QCBAQCBAQEBACgoKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg" \
                "ICAgIHpyCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICBZdCQk" \
                "JC4KICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIC4sZSQkJCQkRiYj" \
                "MDM5OwogICAgICAgICAgICAgICAgICAgICAgICAgNGUgciAgICAgICAgICAgICAgICQkJCQkJCQu" \
                "CiAgICAgICAgICAgICAgICAgICAgICAgICBkJCRiciAgICAgICAgICAgIF96JCQkJCQkJEZgCiAg" \
                "ICAgICAgICAgICAgICAgICAgICAgICAgPyQkYi5fICAgICAgICAgIF4/JCQkJCQkJAogICAgICAg" \
                "ICAgICAgICAgICAgICAgICAgICA0JCQkJnF1b3Q7ICAgICAtZWVjLiAgJnF1b3Q7JnF1b3Q7SlAm" \
                "cXVvdDsgLi5lZWUkJS4uCiAgICAgICAgICAgICAgICAgICAgICAgICAgIC0qKk4gI2MgICAtXioq" \
                "Ki5lRSAgXnokUCQkJCQkJCQkJHItCiAgICAgICAgICAgICAgICAgIC56ZSQkJCQkZXU/JGV1ICYj" \
                "MDM5OyQkJCRiICQ9XiokJCAuJCQkJCQkJCQkJCZxdW90OwogICAgICAgICAgICAgICAtLS4mcXVv" \
                "dDs/JCQkJCQkJCQkYyZxdW90OyQkYyAuJnF1b3Q7JnF1b3Q7JnF1b3Q7IGUkSyAgPSZxdW90OyZx" \
                "dW90Oyo/JCQkUCZxdW90OyZxdW90OyZxdW90OyZxdW90OwogICB1ZWVlLiBgOmAgICRFICEhaCA/" \
                "JCQkJCQkJCRiIFIkTiYjMDM5O34hISAqJCRGIEomcXVvdDsmcXVvdDsmcXVvdDtDLiAgYAogIEog" \
                "IGAmcXVvdDskJGV1YCFoICEhIWA0ISEmbHQ7PyQkJCQkJCRQID8mcXVvdDsuZWVlLXouZWUmcXVv" \
                "dDsgfiQkZS5icgogICYjMDM5O2okJE5lYD8kJGNgNCF+YC1lLTohOmAkJCQkJCQkICQkKiomcXVv" \
                "dDt6ICReUiRQICAzICZxdW90OyQkJGJKCiAgIDQkJCRGJnF1b3Q7LmA/JCRjYCEhIFwpLiEhIWA/" \
                "JCQkJEYuJCQkIyAkdSQlIGVlKiZxdW90O14gOjRgJnF1b3Q7JCZxdW90Oz8kcQogICAgJnF1b3Q7" \
                "JnF1b3Q7YCwhISE6YCQkTi40ISF+fi5+fjQgPyQkRiYjMDM5OyQkRi5ALiogLUwuZUAkJCQkZWMu" \
                "ICAgICAgJnF1b3Q7CiAgICAmcXVvdDtScmAhISEhaCA/JCRjYGg6IGAjICEhICRGLHI0JEwqKiog" \
                "IGUkJCQkJCQkJCQkJCRoYwogICAgICAjZSYjMDM5OzQhISEhTGAkJGImIzAzOTshLjohaGB+fiAu" \
                "JEYmIzAzOTsmcXVvdDsgICAgZCQkJCQkJCQkJCQkJCQkJCQkaCwKICAgICAgIF4kLmAhISEhaCAk" \
                "JGJgIS4gLSAgICAkUCAvJiMwMzk7ICAgLiQkJCQkJCQkJCQkJCQkJCQkJCQkJGMKICAgICAgICAg" \
                "JnF1b3Q7JGNgISEhaGAkJC40fiAgICAgICQkJHImIzAzOTsgICAmbHQ7JCQkJCQkJCQkJCQkJCQk" \
                "JCQkJFAmcXVvdDsmcXVvdDsmcXVvdDsKICAgICAgICAgICBedGUuYH4gJCRiICAgICAgICBgRnVl" \
                "LSAgIGAkJCQkJCQkJCQkJCQkJFAmcXVvdDsuOiAgISEgJnF1b3Q7Jmx0OwogICAgICAgICAgICAg" \
                "IF4mcXVvdDs9NCRQJnF1b3Q7ICAgICAuLCwsLiAtXi4gICA/JCQkJCQkJCQkJCZxdW90Oz86LiAh" \
                "ISA6ISF+ICwsZWMuLgogICAgICAgICAgICAgICAgICAgIC4ueiQkJCQkJCQkJGgsICAgIGAkJCQk" \
                "JCRQJnF1b3Q7Li5gIWYgOiFmIH4pTHplJCQkUCZxdW90OyZxdW90OyZxdW90OyZxdW90Oz9pCiAg" \
                "ICAgICAgICAgICAgICAgIHVkJCQkJCQkJCQkJCQkJCRoICAgIGA/JCRGICZsdDshISYjMDM5OyZs" \
                "dDshJmd0Ozp+KXVlJCRQKiZxdW90Oy4uOiEhISEhIEoKICAgICAgICAgICAgICAgIC5LJCQkJCQk" \
                "JCQkJCQkJCQkJCQsICAgICBQLiZndDtlJiMwMzk7IWYgIX4gZWQkJFAmcXVvdDsuOiEhISEhISEh" \
                "YC5kJnF1b3Q7CiAgICAgICAgICAgICAgIHokJCQkJCQkJCQkJCQkJCQkJCQkJCAgICAgIDQhIX5c" \
                "ZSQkJFBgOiEhISEhISEhISEmIzAzOTsuZVAmIzAzOTsKICAgICAgICAgICAgICAtKiZxdW90Oy4g" \
                "LiAmcXVvdDs/PyQkJCQkJCQkJCQkJCAgICAgICB+IGB6JCQkRiZxdW90Oy5gISEhISEhISEhISYj" \
                "MDM5OyxkUCZxdW90OwogICAgICAgICAgICAuJnF1b3Q7ICk6ISFoIGlgIS0gKCZxdW90Oz8kJCQk" \
                "JCRmICAgICAgICAsJCRQJnF1b3Q7OiEgKS4gYCYjMDM5OyEhISFgLGQkRiYjMDM5OwogICAgICAg" \
                "LnVlZWV1LkpgLV4uIWggJmx0Oy0gIH5gLi4gPz8kJCYjMDM5OyAgICAgICAsJCQgOiEhYGUkJCQk" \
                "ZSBgLGUkRiYjMDM5OwogICAgZSQkJCQkJCQkJCQkJCRlZWlDICZxdW90Oyk/Oi0mbHQ7OiUmIzAz" \
                "OTs6Xj8gICAgICAgID8kZiAhISEgPyQkJCQmcXVvdDssRiZxdW90OwogICBQJnF1b3Q7Li4uLmBg" \
                "YCZxdW90OyZxdW90Oz8kJCQkJCQkJCRldUxeLiEuLmAgLiAgICAgICAgICZxdW90O1R1Ll8uLGBg" \
                "JnF1b3Q7JnF1b3Q7CiAgICQgISEhISEhISEhITo6LiZxdW90OyZxdW90Oz8/JCQkJCQkZUp+Xj0u" \
                "ICAgICAgICAgICAgYGBgYAogICA/JC5gISEhISEhISEhISEhISE6LiZxdW90Oz8/JCQkJCRjJiMw" \
                "Mzk7LgogICAgJnF1b3Q7P2IuYCEhISEhISEhISEhISEhISEmZ3Q7LiZxdW90Oz8kJCQkYwogICAg" \
                "ICBePyRjYCYjMDM5OyEhISEhISEhISEhJiMwMzk7LGVlYiwgJnF1b3Q7JCQkawogICAgICAgICAm" \
                "cXVvdDs/JGUuYCYjMDM5OyEhISEhISEgJCQkJCQgOy4/JCQKICAgICAgICAgICAgJnF1b3Q7PyRl" \
                "ZSxgYCYjMDM5OyYjMDM5OyEuJnF1b3Q7PyRQYGkhISAzUAogICAgICAgICAgICAgICAgJnF1b3Q7" \
                "JnF1b3Q7Pz8kYmVjLCwuLGNlZWVQJnF1b3Q7CiAgICAgICAgICAgICAgICAgICAgICAgYCZxdW90" \
                "OyZxdW90OyZxdW90OyZxdW90OyZxdW90OyZxdW90Owo8L3ByZT4="

#define METHOD_NAME "sendMessage"
#define USER_NAME "teruteru128"
#define PASSWORD "testpassword"

/**
 * int main(int argc, char* argv[]){
 *   // グローバル定数初期化
 *   global_init();
 *   // 設定ファイルパース
 *   configfile_parse();
 *   // コマンドライン引数パース
 *   arg_parse();
 *   // メイン処理
 *   do_main();
 *   // 後片付け
 *   global_cleanup();
 * }
 * */
int main(int const argc, const char **const argv)
{
  FILE *toaddrfile = fopen(SENDTO_ADDRESS_FILE, "r");
  if (toaddrfile == NULL)
  {
    err(EXIT_FAILURE, "fopen");
  }

  xmlrpc_env env;
  xmlrpc_client *clientP;
  xmlrpc_server_info *serverP;
  xmlrpc_value *resultP;
  /* Initialize our error-handling environment. */
  xmlrpc_env_init(&env);
  die_if_fault_occurred(&env);
  xmlrpc_client_setup_global_const(&env);
  die_if_fault_occurred(&env);
  xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0,
                       &clientP);
  die_if_fault_occurred(&env);
  serverP = xmlrpc_server_info_new(&env, SERVER_URL);
  die_if_fault_occurred(&env);
  xmlrpc_server_info_set_user(&env, serverP, USER_NAME, PASSWORD);
  die_if_fault_occurred(&env);
  xmlrpc_server_info_allow_auth_basic(&env, serverP);
  die_if_fault_occurred(&env);

  char toaddress[64];
  char *tmp = NULL;
  xmlrpc_value *paramArray = NULL;
  xmlrpc_value *toaddressv = NULL;
  xmlrpc_value *fromaddressv = xmlrpc_string_new(&env, GENELRAL);
  die_if_fault_occurred(&env);
  xmlrpc_value *subjectv = xmlrpc_string_new(&env, SUBJECT);
  die_if_fault_occurred(&env);
  xmlrpc_value *messagev = xmlrpc_string_new(&env, MESSAGE);
  die_if_fault_occurred(&env);
  xmlrpc_value *encodingTypev = xmlrpc_int_new(&env, 2);
  die_if_fault_occurred(&env);
  xmlrpc_value *TTLv = xmlrpc_int_new(&env, 4 * 24 * 60 * 60);
  die_if_fault_occurred(&env);
  fprintf(stderr, "initialized\n");
  char *p = NULL;
  while ((tmp = fgets(toaddress, 64, toaddrfile)) != NULL)
  {
    /* ファイルから読み込んだ文字列から改行文字を取り除く */
    p = strpbrk(toaddress, "\r\n");
    if (p != NULL)
    {
      *p = '\0';
    }
    /* 文字列をxmlrpc文字列オブジェクトに変換する */
    toaddressv = xmlrpc_string_new(&env, toaddress);
    die_if_fault_occurred(&env);

    /* xmlrpcのパラメータを組み立てる */
    paramArray = xmlrpc_array_new(&env);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, toaddressv);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, fromaddressv);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, subjectv);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, messagev);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, encodingTypev);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, TTLv);
    die_if_fault_occurred(&env);

    /* Make the remote procedure call パラメーターとメソッドを指定して呼び出す */
    xmlrpc_client_call2(&env, clientP, serverP, METHOD_NAME, paramArray, &resultP);
    die_if_fault_occurred(&env);

    printf("%s\n", toaddress);

    /* Dispose of our result value. ゴミ掃除 */
    xmlrpc_DECREF(paramArray);
    xmlrpc_DECREF(toaddressv);
    xmlrpc_DECREF(fromaddressv);
    xmlrpc_DECREF(subjectv);
    xmlrpc_DECREF(messagev);
    xmlrpc_DECREF(encodingTypev);
    xmlrpc_DECREF(TTLv);
    xmlrpc_DECREF(resultP);
  }
  xmlrpc_DECREF(fromaddressv);
  xmlrpc_DECREF(subjectv);
  xmlrpc_DECREF(messagev);
  xmlrpc_DECREF(encodingTypev);
  xmlrpc_DECREF(TTLv);

  /* Clean up our error-handling environment. */
  xmlrpc_env_clean(&env);

  xmlrpc_client_destroy(clientP);

  xmlrpc_client_teardown_global_const();

  return EXIT_SUCCESS;
}
