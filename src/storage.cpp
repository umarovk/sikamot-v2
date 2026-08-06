#include "storage.h"
#include "../include/config.h"
#include <Preferences.h>

static Preferences prefs;

void storageInit() {
    prefs.begin("sikamot", false);
}

bool hasMasterCard() {
    return prefs.isKey("master_uid");
}

String getMasterUID() {
    return prefs.getString("master_uid", "");
}

void setMasterUID(const String& uid) {
    prefs.putString("master_uid", uid);
}

int getCardCount() {
    return prefs.getInt("card_count", 0);
}

String getCard(int index) {
    String key = "card_" + String(index);
    return prefs.getString(key.c_str(), "");
}

bool isAuthorized(const String& uid) {
    int count = getCardCount();
    for (int i = 0; i < count; i++) {
        if (getCard(i) == uid) return true;
    }
    return false;
}

bool isMaster(const String& uid) {
    return hasMasterCard() && getMasterUID() == uid;
}

bool addCard(const String& uid) {
    if (isAuthorized(uid)) return false;
    int count = getCardCount();
    if (count >= MAX_CARDS) return false;
    String key = "card_" + String(count);
    prefs.putString(key.c_str(), uid);
    prefs.putInt("card_count", count + 1);
    return true;
}

bool removeCard(const String& uid) {
    int count = getCardCount();
    int found = -1;
    for (int i = 0; i < count; i++) {
        if (getCard(i) == uid) { found = i; break; }
    }
    if (found == -1) return false;

    // Geser kartu setelah index yang dihapus ke kiri
    for (int i = found; i < count - 1; i++) {
        String next = getCard(i + 1);
        String key  = "card_" + String(i);
        prefs.putString(key.c_str(), next);
    }
    String lastKey = "card_" + String(count - 1);
    prefs.remove(lastKey.c_str());
    prefs.putInt("card_count", count - 1);
    return true;
}

void clearAll() {
    prefs.clear();
}
