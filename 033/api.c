
#include "api.h"
#include "xmlrpc.h"

static int initflag;
static xmlrpc_env env;
static xmlrpc_client* cp;
static xmlrpc_server_info* sinfo;

int api_init(){
    xmlrpc_env_init(&env);
    xmlrpc_client_setup_global_const(&env);
    xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, "bm", "0.0.0.1-alpha", NULL, 0, &cp);
    sinfo = xmlrpc_server_info_new(&env, "http://localhost:8442/");
    xmlrpc_server_info_set_user(&env, sinfo, "teruteru128", "testpassword");
    xmlrpc_server_info_allow_auth_basic(&env, sinfo);
    initflag = 1;
}

int api_cleanup(){
    xmlrpc_client_setup_global_const(&env);
    xmlrpc_env_clean(&env);
}

static int api_call(char* methodName, xmlrpc_value* paramArray, xmlrpc_value** resultPP){
  xmlrpc_client_call2(&env, cp, sinfo, methodName, paramArray, resultP);
}

/***/
int api_helloWorld(char* first, char* second, char** msg){
  char* msg = NULL;
  if(initflag != 1){
    return EXIT_FAILURE;
  }
  xmlrpc_value* paramArray;
  xmlrpc_value* first = NULL;
  xmlrpc_value* second = NULL;
  xmlrpc_value* resultP = NULL;

  // create array
  paramArray = xmlrpc_array_new(&env);
  first = xmlrpc_string_new(&env, first);
  second = xmlrpc_string_new(&env, second);
  xmlrpc_array_append_item(&env, paramArray, first);
  xmlrpc_array_append_item(&env, paramArray, second);

  api_call("helloWorld", paramArray, resultP);
  xmlrpc_read_string(&env, resultP, msg);
  xmlrpc_DECREF(paramArray);
  xmlrpc_DECREF(resultP);

  return EXIT_SUCCESS;
}

