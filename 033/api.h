
#ifndef API_H
#define API_H
int api_init();
int api_cleanup();
char *api_helloWorld(char*, char*);
int api_add(int, int);
int api_statusBar();
int api_listAddresses2();
int api_createRandomAddress();
int api_getDeterministicAddress();
int api_getAllInboxMessages();
int api_getAllInboxMessageIDs();
int api_getSentMessageByAckData();
int api_getAllSentMessages();
int api_getSentMessageByID();
int api_getSentMessagesBySender();
int api_trashMessages();
int api_sendMessage();
int api_sendBroadcast();
int api_getStatus();
int api_listSubscriptions();
int api_addSubscription();
int api_deleteSubscriptions();
int api_listAddressBookEntries();
int api_addAddressBookEntry();
int api_deleteAddressBookEntry();
int api_createChan();
int api_deleteAddress();
int api_decodeAddress();
int api_addAddressToBlackWhiteList();
int api_removeAddressFromBlackWhiteList();
int api_clientStatus();
#endif

