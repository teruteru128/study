
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "bm.h"
#include "bmapi.h"
#include "xmlrpc.h"
#include "base64.h"
#include <stdlib.h>

static int initflag = 0;
static xmlrpc_env env;
static xmlrpc_client *cp;
static xmlrpc_server_info *sinfo;

typedef struct bm_client_t bm_client;

static void die_if_fault_occurred(xmlrpc_env *env)
{
  /* Check our error-handling environment for an XML-RPC fault. */
  if (env->fault_occurred)
  {
    fprintf(stderr, "XML-RPC Fault: %s (%d)\n",
            env->fault_string, env->fault_code);
    exit(1);
  }
}

bm_client *bm_client_new()
{
  if(initflag == 0)
  {
    return NULL;
  }
  bm_client *c = malloc(sizeof(bm_client));
  xmlrpc_client_create(&env, XMLRPC_CLIENT_NO_FLAGS, PACKAGE_NAME, PACKAGE_VERSION, NULL, 0, &c->cp);
  c->sinfo = xmlrpc_server_info_new(&env, "http://localhost:8442/");
  xmlrpc_server_info_set_user(&env, c->sinfo, "teruteru128", "testpassword");
  xmlrpc_server_info_allow_auth_basic(&env, c->sinfo);
  return c;
}

int bm_client_set_server(bm_client *c, const char *const url)
{
  c->sinfo = xmlrpc_server_info_new(&env, url);
}

int bm_client_set_user(bm_client *c, const char *const username, const char *const password)
{
  xmlrpc_server_info_set_user(&env, c->sinfo, "teruteru128", "testpassword");
}

void bm_client_free(bm_client *c)
{
  return;
}

int bmapi_init()
{
  if(initflag == 0)
  {
    xmlrpc_env_init(&env);
    xmlrpc_init(&env);
    xmlrpc_client_setup_global_const(&env);
    initflag = 1;
  }
  return 0;
}

int bmapi_cleanup()
{
  if(initflag == 1)
  {
    xmlrpc_client_setup_global_const(&env);
    xmlrpc_env_clean(&env);
    xmlrpc_term();
    initflag = 0;
  }
  return 0;
}

static int bmapi_call(char *methodName, xmlrpc_value *paramArray, xmlrpc_value **resultPP)
{
  xmlrpc_client_call2(&env, cp, sinfo, methodName, paramArray, resultPP);
}

/***/
char *bmapi_helloWorld(char *first, char *second)
{
  char *msg = NULL;
  if (initflag != 1)
  {
    return NULL;
  }
  xmlrpc_value *paramArray;
  xmlrpc_value *f = NULL;
  xmlrpc_value *s = NULL;
  xmlrpc_value *resultP = NULL;

  // create array
  paramArray = xmlrpc_array_new(&env);
  f = xmlrpc_string_new(&env, first);
  s = xmlrpc_string_new(&env, second);
  xmlrpc_array_append_item(&env, paramArray, f);
  xmlrpc_array_append_item(&env, paramArray, s);

  bmapi_call("helloWorld", paramArray, &resultP);
  printf("%d\n", env.fault_occurred);
  xmlrpc_read_string(&env, resultP, (const char **const) & msg);
  xmlrpc_DECREF(paramArray);
  xmlrpc_DECREF(resultP);

  return msg;
}

char *bmapi_getStatus(char *ackData)
{
  char *methodName = "getStatus";

  xmlrpc_value *paramArray = NULL;
  xmlrpc_value *ack_xml = NULL;
  xmlrpc_value *resultP = NULL;

  char *msg = NULL;
  paramArray = xmlrpc_array_new(&env);
  ack_xml = xmlrpc_string_new(&env, ackData);
  xmlrpc_array_append_item(&env, paramArray, ack_xml);
  bmapi_call(methodName, paramArray, &resultP);
  xmlrpc_read_string(&env, resultP, (const char **const) & msg);

  return msg;
}

char *bmapi_simpleSendMessage(char *toaddress, char *fromaddress, char *subject, char *message)
{
  return bmapi_sendMessage(toaddress, fromaddress, subject, message, 2, 4 * 24 * 60 * 60);
}
char *bmapi_sendMessage(char *toaddress, char *fromaddress, char *subject, char *message, int encodingType, int TTL)
{
  char *const methodName = "sendMessage";
  char *subb64tmp = NULL;
  char *msgb64tmp = NULL;
  xmlrpc_value *paramArray = NULL;
  xmlrpc_value *toaddressV = NULL;
  xmlrpc_value *fromaddressV = NULL;
  xmlrpc_value *subjectV = NULL;
  xmlrpc_value *messageV = NULL;
  xmlrpc_value *resultP = NULL;
  xmlrpc_value *encodingTypeP = NULL;
  xmlrpc_value *TTLP = NULL;
  char *msg = NULL;

  paramArray = xmlrpc_array_new(&env);
  die_if_fault_occurred(&env);
  toaddressV = xmlrpc_string_new(&env, toaddress);
  die_if_fault_occurred(&env);

  fromaddressV = xmlrpc_string_new(&env, fromaddress);
  die_if_fault_occurred(&env);

  subb64tmp = base64encode(subject, strlen(subject));
  subjectV = xmlrpc_string_new(&env, subb64tmp);
  die_if_fault_occurred(&env);

  msgb64tmp = base64encode(message, strlen(message));
  messageV = xmlrpc_string_new(&env, msgb64tmp);
  die_if_fault_occurred(&env);

  encodingTypeP = xmlrpc_int_new(&env, encodingType);
  die_if_fault_occurred(&env);

  TTLP = xmlrpc_int_new(&env, TTL);
  die_if_fault_occurred(&env);

  xmlrpc_array_append_item(&env, paramArray, toaddressV);
  die_if_fault_occurred(&env);
  xmlrpc_array_append_item(&env, paramArray, fromaddressV);
  die_if_fault_occurred(&env);
  xmlrpc_array_append_item(&env, paramArray, subjectV);
  die_if_fault_occurred(&env);
  xmlrpc_array_append_item(&env, paramArray, messageV);
  die_if_fault_occurred(&env);
  xmlrpc_array_append_item(&env, paramArray, encodingTypeP);
  die_if_fault_occurred(&env);
  xmlrpc_array_append_item(&env, paramArray, TTLP);
  die_if_fault_occurred(&env);

  bmapi_call(methodName, paramArray, &resultP);
  die_if_fault_occurred(&env);

  xmlrpc_read_string(&env, resultP, (const char **const) & msg);
  xmlrpc_DECREF(paramArray);
  xmlrpc_DECREF(resultP);

  die_if_fault_occurred(&env);

  return msg;
}
char *bmapi_getDeterministicAddress(char *pass, int addver, int stream)
{
  char *const methodName = "getDeterministicAddress";

  char *passb64 = NULL;
  xmlrpc_value *paramArray = NULL;
  xmlrpc_value *passphrase = NULL;
  xmlrpc_value *add = NULL;
  xmlrpc_value *s = NULL;
  xmlrpc_value *resultP = NULL;

  char *msg = NULL;
  paramArray = xmlrpc_array_new(&env);
  passb64 = base64encode(pass, strlen(pass));
  passphrase = xmlrpc_string_new(&env, passb64);
  add = xmlrpc_int_new(&env, addver);
  s = xmlrpc_int_new(&env, stream);
  xmlrpc_array_append_item(&env, paramArray, passphrase);
  xmlrpc_array_append_item(&env, paramArray, add);
  xmlrpc_array_append_item(&env, paramArray, s);
  bmapi_call(methodName, paramArray, &resultP);
  xmlrpc_read_string(&env, resultP, (const char **const) & msg);
  xmlrpc_DECREF(paramArray);
  xmlrpc_DECREF(resultP);
  return msg;
}

char *bmapi_createChan(char *passphrase)
{
  char *const methodName = "sendMessage";
  char *address = NULL;
  xmlrpc_value *passArray = NULL;
  xmlrpc_value *passV = NULL;
  xmlrpc_value *resultP = NULL;
  xmlrpc_value *encodingTypeP = NULL;
  char *adb64tmp = base64encode(passphrase, strlen(passphrase));
  printf("%s, %s\n", passphrase, adb64tmp);
  size_t len = strlen(adb64tmp);
  char *adbtmp2 = calloc(sizeof(char), len + 2);
  strncpy(adbtmp2, adb64tmp, len);
  adbtmp2[len] = '\n';

  passArray = xmlrpc_array_new(&env);
  die_if_fault_occurred(&env);

  passV = xmlrpc_string_new(&env, adbtmp2);
  die_if_fault_occurred(&env);
  //  encodingTypeP = xmlrpc_int_new(&env, 2);
  //  die_if_fault_occurred(&env);

  xmlrpc_array_append_item(&env, passArray, passV);
  die_if_fault_occurred(&env);
  //  xmlrpc_array_append_item(&env, passArray, encodingTypeP);
  //  die_if_fault_occurred(&env);
  bmapi_call(methodName, passArray, &resultP);
  die_if_fault_occurred(&env);
  printf("%s\n", xmlrpc_type_name(xmlrpc_value_type(resultP)));
  xmlrpc_read_string(&env, resultP, (const char **const) & address);
  xmlrpc_DECREF(passArray);
  xmlrpc_DECREF(resultP);
  free(adb64tmp);
  free(adbtmp2);
  return address;
}
