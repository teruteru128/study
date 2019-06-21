
#ifndef API_H
#define API_H
#include <stdarg.h>
#include <string.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>

int api_init();
int api_cleanup();
char *api_helloWorld(char*, char*);
int api_add(int, int);
int api_statusBar();
int api_listAddresses2();
int api_createRandomAddress();
char* api_getDeterministicAddress(char*, int, int);
int api_getAllInboxMessages();
int api_getAllInboxMessageIDs();
int api_getSentMessageByAckData();
int api_getAllSentMessages();
int api_getSentMessageByID();
int api_getSentMessagesBySender();
int api_trashMessages();
/**
  simpleSendMessage(toaddress, fromaddress, subject, message)
  sendMessage(toAddress, fromAddress, subject, message, encodingType, TTL)
*/
char* api_simpleSendMessage(char*, char*, char*, char*);
char* api_sendMessage(char*, char*, char*, char*, int, int);
int api_sendBroadcast();
char* api_getStatus(char*);
int api_listSubscriptions();
int api_addSubscription();
int api_deleteSubscriptions();
int api_listAddressBookEntries();
int api_addAddressBookEntry();
int api_deleteAddressBookEntry();
char *api_createChan(char *);
int api_deleteAddress();
int api_decodeAddress();
int api_addAddressToBlackWhiteList();
int api_removeAddressFromBlackWhiteList();
int api_clientStatus();
#endif

