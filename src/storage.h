#pragma once
#include <Arduino.h>

void     storageInit();
bool     hasMasterCard();
String   getMasterUID();
void     setMasterUID(const String& uid);
int      getCardCount();
String   getCard(int index);
bool     addCard(const String& uid);
bool     removeCard(const String& uid);
bool     isAuthorized(const String& uid);
bool     isMaster(const String& uid);
void     clearAll();
