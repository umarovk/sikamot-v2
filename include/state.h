#pragma once

enum SystemState {
    STATE_FIRST_SETUP,  // Belum ada master card, tampil halaman setup
    STATE_LOCKED,       // Mesin mati, menunggu tap kartu authorized
    STATE_UNLOCKED,     // Mesin hidup, relay ON aktif
    STATE_ENROLL        // Mode tambah/hapus kartu (master card di-tap)
};

extern SystemState systemState;
extern unsigned long enrollStartTime;

enum PendingCommand {
    CMD_NONE,
    CMD_START_ENGINE,
    CMD_STOP_ENGINE
};
extern PendingCommand pendingCommand;
