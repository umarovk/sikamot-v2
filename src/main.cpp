// SIKAMOT-V2 — IoT Motor Ignition via RFID
// Branch: spi  |  Interface: PN532 via SPI
// Board  : ESP32 DEVKITC V4 WROOM-32D

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "../include/config.h"
#include "../include/state.h"
#include "buzzer.h"
#include "storage.h"
#include "web_portal.h"

// ── PN532 SPI instance ───────────────────────────────────────────
Adafruit_PN532 nfc(PN532_SS, &SPI);

// ── Global state (dideklarasikan di state.h sebagai extern) ──────
SystemState systemState  = STATE_FIRST_SETUP;
unsigned long enrollStartTime = 0;

// ── Relay helpers (Active LOW) ───────────────────────────────────
static void relayOn(bool on) {
    digitalWrite(PIN_RELAY_ON, on ? LOW : HIGH);
}

static void relayStarter(bool on) {
    digitalWrite(PIN_RELAY_STARTER, on ? LOW : HIGH);
}

// ── Engine control ───────────────────────────────────────────────
static void startEngine() {
    Serial.println("[ENGINE] Menghidupkan mesin...");
    beepSuccess();
    relayOn(true);
    delay(IGNITION_DELAY_MS);
    relayStarter(true);
    delay(STARTER_DURATION_MS);
    relayStarter(false);
    systemState = STATE_UNLOCKED;
    Serial.println("[ENGINE] Mesin hidup.");
}

static void stopEngine() {
    Serial.println("[ENGINE] Mematikan mesin...");
    beepStop();
    relayOn(false);
    relayStarter(false);
    systemState = STATE_LOCKED;
    Serial.println("[ENGINE] Mesin mati.");
}

// ── UID helper ───────────────────────────────────────────────────
static String uidToString(uint8_t* uid, uint8_t len) {
    String s = "";
    for (uint8_t i = 0; i < len; i++) {
        if (uid[i] < 0x10) s += "0";
        s += String(uid[i], HEX);
    }
    s.toUpperCase();
    return s;
}

// ── RFID scan handler ────────────────────────────────────────────
static unsigned long lastScanTime = 0;

static void handleRFID() {
    uint8_t uid[7];
    uint8_t uidLen = 0;

    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 100)) return;

    unsigned long now = millis();
    if (now - lastScanTime < SCAN_COOLDOWN_MS) return;
    lastScanTime = now;

    String scanned = uidToString(uid, uidLen);
    Serial.println("[RFID] Scan: " + scanned);

    setLastScannedUID(scanned);

    switch (systemState) {

        case STATE_FIRST_SETUP:
            // Hanya simpan UID, konfirmasi via web portal
            break;

        case STATE_LOCKED:
            if (isMaster(scanned)) {
                systemState = STATE_ENROLL;
                enrollStartTime = millis();
                beepEnroll();
                Serial.println("[ENROLL] Mode enroll aktif.");
            } else if (isAuthorized(scanned)) {
                startEngine();
            } else {
                beepDenied();
                Serial.println("[AUTH] Ditolak: " + scanned);
            }
            break;

        case STATE_UNLOCKED:
            if (isAuthorized(scanned) || isMaster(scanned)) {
                stopEngine();
            } else {
                beepDenied();
            }
            break;

        case STATE_ENROLL:
            if (isMaster(scanned)) {
                // Tap master card lagi = keluar enroll
                systemState = STATE_LOCKED;
                beepSuccess();
                Serial.println("[ENROLL] Keluar enroll mode.");
            } else if (isAuthorized(scanned)) {
                removeCard(scanned);
                beepCardRemoved();
                Serial.println("[ENROLL] Kartu dihapus: " + scanned);
                systemState = STATE_LOCKED;
            } else {
                if (addCard(scanned)) {
                    beepCardAdded();
                    Serial.println("[ENROLL] Kartu ditambah: " + scanned);
                } else {
                    beepDenied();
                    Serial.println("[ENROLL] Daftar kartu penuh.");
                }
                systemState = STATE_LOCKED;
            }
            break;
    }
}

// ── Setup ────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== SIKAMOT-V2 [SPI] ===");

    // Relay
    pinMode(PIN_RELAY_ON,      OUTPUT);
    pinMode(PIN_RELAY_STARTER, OUTPUT);
    relayOn(false);
    relayStarter(false);

    buzzerInit();
    storageInit();

    // SPI + PN532
    SPI.begin(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
    nfc.begin();

    uint32_t ver = nfc.getFirmwareVersion();
    if (!ver) {
        Serial.println("[ERROR] PN532 tidak ditemukan! Cek wiring & DIP switch.");
        while (true) { beepDenied(); delay(2000); }
    }
    Serial.printf("[PN532] Firmware: %d.%d\n", (ver >> 16) & 0xFF, (ver >> 8) & 0xFF);
    nfc.SAMConfig();

    // State awal
    if (!hasMasterCard()) {
        systemState = STATE_FIRST_SETUP;
        Serial.println("[BOOT] Belum ada master card — masuk mode setup.");
    } else {
        systemState = STATE_LOCKED;
        Serial.println("[BOOT] Sistem terkunci, siap digunakan.");
    }
    beepBoot();

    // WiFi AP
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
    Serial.print("[WIFI] AP aktif — IP: ");
    Serial.println(WiFi.softAPIP());

    webPortalInit();
    Serial.println("[WEB] Portal aktif di http://192.168.4.1");
    Serial.println("=========================\n");
}

// ── Loop ─────────────────────────────────────────────────────────
void loop() {
    webPortalHandle();

    // Auto-exit enroll mode setelah timeout
    if (systemState == STATE_ENROLL) {
        if (millis() - enrollStartTime > ENROLL_TIMEOUT_MS) {
            systemState = STATE_LOCKED;
            beepSuccess();
            Serial.println("[ENROLL] Timeout — kembali ke locked.");
        }
    }

    handleRFID();
}
