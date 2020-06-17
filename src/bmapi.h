
#ifndef API_H
#define API_H
#include <stdarg.h>
#include <string.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>

typedef struct bm_client_t
{
  xmlrpc_client *cp;
  xmlrpc_server_info *sinfo;
} bm_client;

int bmapi_init(void);
int bmapi_cleanup();
char *bmapi_helloWorld(char*, char*);
int bmapi_add(int, int);
int bmapi_statusBar();
int bmapi_listAddresses2();
int bmapi_createRandomAddress();
char* bmapi_getDeterministicAddress(char*, int, int);
int bmapi_getAllInboxMessages();
int bmapi_getAllInboxMessageIDs();
int bmapi_getSentMessageByAckData();
int bmapi_getAllSentMessages();
int bmapi_getSentMessageByID();
int bmapi_getSentMessagesBySender();
int bmapi_trashMessages();
/**
  simpleSendMessage(toaddress, fromaddress, subject, message)
  sendMessage(toAddress, fromAddress, subject, message, encodingType, TTL)
*/
char* bmapi_simpleSendMessage(char*, char*, char*, char*);
char* bmapi_sendMessage(char*, char*, char*, char*, int, int);
int bmapi_sendBroadcast();
char* bmapi_getStatus(char*);
int bmapi_listSubscriptions();
int bmapi_addSubscription();
int bmapi_deleteSubscriptions();
int bmapi_listAddressBookEntries();
int bmapi_addAddressBookEntry();
int bmapi_deleteAddressBookEntry();
char *bmapi_createChan(char *);
int bmapi_deleteAddress();
int bmapi_decodeAddress();
int bmapi_addAddressToBlackWhiteList();
int bmapi_removeAddressFromBlackWhiteList();
int bmapi_clientStatus();
#endif
