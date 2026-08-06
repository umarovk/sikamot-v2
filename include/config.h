#pragma once

// ── PN532 SPI Pins (branch: spi) ───────────────────────────────
#define PN532_SCK     18
#define PN532_MISO    19
#define PN532_MOSI    23
#define PN532_SS       5

// ── PN532 I2C Pins (branch: i2c) ───────────────────────────────
#define PN532_SDA     21
#define PN532_SCL     22

// ── PN532 Shared ────────────────────────────────────────────────
#define PN532_IRQ      4

// ── Relay (Active LOW) ──────────────────────────────────────────
#define PIN_RELAY_ON      26   // Relay kontak / ignition
#define PIN_RELAY_STARTER 27   // Relay starter motor

// ── Buzzer (via transistor NPN) ─────────────────────────────────
#define PIN_BUZZER    25

// ── WiFi AP ─────────────────────────────────────────────────────
#define WIFI_AP_SSID  "SIKAMOT-V2"
#define WIFI_AP_PASS  "sikamot123"

// ── Timing (ms) ─────────────────────────────────────────────────
#define IGNITION_DELAY_MS    1500   // Jeda sebelum starter aktif
#define STARTER_DURATION_MS  3000   // Durasi starter aktif
#define SCAN_COOLDOWN_MS     2000   // Anti-bounce scan RFID
#define ENROLL_TIMEOUT_MS   15000   // Auto-exit enroll mode

// ── Storage ─────────────────────────────────────────────────────
#define MAX_CARDS     10
