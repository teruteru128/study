
#include "api.h"
#include "xmlrpc.h"
#include <stdlib.h>

static int initflag;
static xmlrpc_env env;
static xmlrpc_client* cp;
static xmlrpc_server_info* sinfo;
void die_if_fault_occurred (xmlrpc_env*);

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
  xmlrpc_client_call2(&env, cp, sinfo, methodName, paramArray, resultPP);
}

/***/
char* api_helloWorld(char* first, char* second){
  char* msg = NULL;
  if(initflag != 1){
    return NULL;
  }
  xmlrpc_value* paramArray;
  xmlrpc_value* f = NULL;
  xmlrpc_value* s = NULL;
  xmlrpc_value* resultP = NULL;

  // create array
  paramArray = xmlrpc_array_new(&env);
  f = xmlrpc_string_new(&env, first);
  s = xmlrpc_string_new(&env, second);
  xmlrpc_array_append_item(&env, paramArray, f);
  xmlrpc_array_append_item(&env, paramArray, s);

  api_call("helloWorld", paramArray, &resultP);
  xmlrpc_read_string(&env, resultP, (const char** const)&msg);
  xmlrpc_DECREF(paramArray);
  xmlrpc_DECREF(resultP);

  return msg;
}

char* api_getStatus(xmlrpc_env* env, xmlrpc_client* cp, xmlrpc_server_info* sinfo,char* ackData){
  char* methodName = "getStatus";

  xmlrpc_value* paramArray = NULL;
  xmlrpc_value* ack_xml = NULL;
  xmlrpc_value* resultP = NULL;

  char* msg = NULL;
  paramArray = xmlrpc_array_new(env);
  ack_xml = xmlrpc_string_new(env, ackData);
  xmlrpc_array_append_item(env, paramArray, ack_xml);
  xmlrpc_client_call2(env, cp, sinfo, methodName, paramArray, &resultP);
  xmlrpc_read_string(env, resultP, (const char** const)&msg);

  return msg;
}

int api_simpleSendMessage(xmlrpc_env* env, xmlrpc_client* cp, char* toaddress, char* fromaddress, char* subject, char* message){
  api_sendMessage(env, cp, toaddress, fromaddress, subject, message, 2, 4 * 24 * 60 * 60);
}
char* api_sendMessage(xmlrpc_env* env, xmlrpc_client* cp, char* toaddress, char* fromaddress, char* subject, char* message, int encodingType, int TTL){
    char * const methodName = "getStatus";

  xmlrpc_value* paramArray = NULL;
  xmlrpc_value* toaddressV = NULL;
  xmlrpc_value* fromaddressV = NULL;
  xmlrpc_value* subjectV = NULL;
  xmlrpc_value* messageV = NULL;
  xmlrpc_value* resultP = NULL;
  char* msg = NULL;

  paramArray = xmlrpc_array_new(env);
  die_if_fault_occurred(env);
  toaddressV = xmlrpc_string_new(env, toaddress);
  die_if_fault_occurred(env);

  fromaddressV = xmlrpc_string_new(env, fromaddress);
  die_if_fault_occurred(env);

  subjectV = xmlrpc_string_new(env, "dG9vbHRlc3Q=\n");
  die_if_fault_occurred(env);

  messageV = xmlrpc_string_new(env, "SGVsbG8gV29ybGQh\n");
  die_if_fault_occurred(env);

  xmlrpc_array_append_item(env, paramArray, toaddressV);
  die_if_fault_occurred(env);
  xmlrpc_array_append_item(env, paramArray, fromaddressV);
  die_if_fault_occurred(env);
  xmlrpc_array_append_item(env, paramArray, subjectV);
  die_if_fault_occurred(env);
  xmlrpc_array_append_item(env, paramArray, messageV);
  die_if_fault_occurred(env);

  xmlrpc_client_call2(env, cp, sinfo, methodName, paramArray, &resultP);
  die_if_fault_occurred(env);

  xmlrpc_read_string(env, resultP, (const char** const)&msg);
  die_if_fault_occurred(env);

  return msg;
}

