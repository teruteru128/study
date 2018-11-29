
#include <stdio.h>
#include <stdlib.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>
#include <string.h>
#include "bitmessage.h"
#define localhost_ip "127.0.0.1"
#define bitmessage_port 8442
#define NAME "TR BM TEST CLIENT"
#define VERSION "0.0.1-alpha"
#define SERVER_URL "http://127.0.0.1:8442/"

void die_if_fault_occurred (xmlrpc_env *env)
{
    /* Check our error-handling environment for an XML-RPC fault. */
    if (env->fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n",
                env->fault_string, env->fault_code);
        exit(1);
    }
}

int main(int const argc, const char ** const argv) {

    xmlrpc_env env;
    xmlrpc_value * resultP = NULL;
    xmlrpc_server_info *sinfo = NULL;
    const char * msg = NULL;
    char * const clientName = NAME;
    char * const clientVersion = VERSION;
    char * const url = SERVER_URL;
    char * const methodName = "helloWorld";
    xmlrpc_client* cp = NULL;
    xmlrpc_value* paramArray;
    xmlrpc_value* first;
    xmlrpc_value* second;

    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);

    /* Start up our XML-RPC client library. */
    //xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, clientName, clientVersion, NULL, 0);
    xmlrpc_client_setup_global_const(&env);
    //[ handle possible failure of above ]

    xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, clientName, clientVersion, NULL, 0, &cp);
    die_if_fault_occurred(&env);

    sinfo = xmlrpc_server_info_new(&env, url);
    die_if_fault_occurred(&env);
    xmlrpc_server_info_set_user(&env, sinfo, "teruteru128", "testpassword");
    die_if_fault_occurred(&env);
    xmlrpc_server_info_allow_auth_basic(&env, sinfo);

    paramArray = xmlrpc_array_new(&env);
    die_if_fault_occurred(&env);

    first = xmlrpc_string_new(&env, "Hello");
    die_if_fault_occurred(&env);

    second = xmlrpc_string_new(&env, "World!");
    die_if_fault_occurred(&env);

    xmlrpc_array_append_item(&env, paramArray, first);
    die_if_fault_occurred(&env);
    xmlrpc_array_append_item(&env, paramArray, second);
    die_if_fault_occurred(&env);

    xmlrpc_client_call2(&env, cp, sinfo, methodName,
                 paramArray, &resultP);
    die_if_fault_occurred(&env);

    //xmlrpc_parse_value(&env, resultP, "s", &msg);
    xmlrpc_read_string(&env, resultP, &msg);
    die_if_fault_occurred(&env);

    printf("message : %s\n", msg);
    
    free((void *)msg);
    msg = NULL;

    xmlrpc_client_destroy(cp);
    xmlrpc_DECREF(paramArray);
    xmlrpc_server_info_free(sinfo);
    /* Dispose of our result value. */
    xmlrpc_DECREF(resultP);

    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(&env);
    
    /* Shutdown our XML-RPC client library. */
    //xmlrpc_client_cleanup();
    xmlrpc_client_setup_global_const(&env);

    return 0;
}

