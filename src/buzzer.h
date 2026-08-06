#pragma once
#include <Arduino.h>

void buzzerInit();
void beepSuccess();    // Akses granted / engine start
void beepStop();       // Engine mati
void beepDenied();     // Akses ditolak
void beepEnroll();     // Masuk enroll mode
void beepCardAdded();  // Kartu berhasil ditambah
void beepCardRemoved();// Kartu berhasil dihapus
void beepBoot();       // First setup / boot
