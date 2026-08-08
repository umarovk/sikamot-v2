# SIKAMOT-V2

Sistem pengaman kendaraan berbasis RFID. Kendaraan hanya bisa dinyalakan menggunakan kartu RFID atau KTP yang sudah terdaftar.

---

## Pertama Kali Digunakan (First Setup)

Saat pertama dinyalakan, sistem masuk **mode setup** karena belum ada Master Card.

1. Sambungkan HP/laptop ke WiFi **SIKAMOT-V2**
2. Buka browser → **http://192.168.4.1**
3. Halaman setup akan muncul — tempel kartu RFID/KTP ke pembaca
4. Klik **"Jadikan Master Card"** untuk konfirmasi
5. Sistem siap digunakan

> Master Card adalah kartu admin utama. Simpan baik-baik, jangan sampai hilang.

---

## Informasi WiFi

| | |
|---|---|
| **SSID** | `SIKAMOT-V2` |
| **Password** | `sikamot123` |
| **Alamat portal** | `http://192.168.4.1` |

WiFi ini dibuat langsung oleh ESP32 (hotspot mandiri, tidak butuh router).

---

## Cara Penggunaan

### Menyalakan Kendaraan
Tempel kartu yang sudah terdaftar ke pembaca RFID → mesin menyala otomatis.

### Mematikan Kendaraan
Tempel kartu authorized ke pembaca RFID lagi → mesin mati.

### Menambah / Menghapus Kartu
Ada 2 cara:

**Cara 1 — Fisik (tap Master Card):**
1. Tempel **Master Card** → buzzer 2 beep, sistem masuk mode enroll
2. Tempel kartu baru → kartu **ditambahkan**
3. Tempel kartu yang sudah terdaftar → kartu **dihapus**
4. Tempel Master Card lagi untuk keluar, atau tunggu 15 detik (auto keluar)

**Cara 2 — Web portal:**
1. Sambung ke WiFi `SIKAMOT-V2`
2. Buka `http://192.168.4.1`
3. Tempel kartu ke pembaca → klik **"Tambah Kartu Ini"**
4. Untuk hapus, klik tombol **"Hapus"** di samping kartu

---

## Kode Suara Buzzer

| Suara | Artinya |
|-------|---------|
| 2 beep pendek | Kartu diterima / mesin mulai menyala |
| 1 beep panjang | Mesin dimatikan |
| 3 beep cepat | Kartu ditolak / tidak terdaftar |
| 2 beep medium | Mode enroll aktif (siap tambah/hapus kartu) |
| 3 beep cepat (nada berbeda) | Kartu berhasil ditambahkan |
| 2 beep panjang | Kartu berhasil dihapus |
| 1 beep panjang saat nyala | Sistem boot / siap |

---

## Status di Web Portal

| Status | Arti |
|--------|------|
| `SETUP` | Belum ada Master Card, perlu setup awal |
| `TERKUNCI` | Mesin mati, menunggu kartu |
| `HIDUP` | Mesin menyala |
| `ENROLL` | Mode tambah/hapus kartu sedang aktif |

---

## Batas Kartu

Maksimal **10 kartu** authorized bisa didaftarkan (tidak termasuk Master Card).

---

## Troubleshooting

**Kartu tidak terbaca**
- Tempel lebih dekat ke modul PN532
- Tunggu 2 detik setelah tap sebelumnya (ada jeda anti-bounce)
- Pastikan kartu bukan jenis yang diblokir

**Web portal tidak bisa dibuka**
- Pastikan sudah terhubung ke WiFi `SIKAMOT-V2`
- Coba ketik manual: `http://192.168.4.1` (bukan https)
- Matikan data seluler di HP saat terhubung ke WiFi ini

**Mesin tidak menyala padahal kartu diterima (ada beep)**
- Cek koneksi relay ke kendaraan
- Cek tegangan aki kendaraan

**Lupa Master Card**
- Tidak ada bypass otomatis (by design, demi keamanan)
- Solusi: flash ulang firmware → sistem reset ke first setup

---

## Spesifikasi Teknis

| | |
|---|---|
| **Mikrokontroler** | ESP32 DEVKITC V4 WROOM-32D |
| **Modul RFID** | PN532 (SPI / I2C) |
| **Tegangan input** | 12V DC (aki motor) |
| **Tegangan sistem** | 5V via step-down |
| **Kartu yang didukung** | MIFARE Classic, MIFARE Ultralight, KTP Indonesia |
| **Penyimpanan kartu** | NVS internal ESP32 (tahan mati listrik) |
| **WiFi mode** | Access Point (hotspot mandiri) |

---

## Repository

`git clone https://github.com/umarovk/sikamot-v2.git`

| Branch | Interface PN532 | DIP Switch |
|--------|----------------|------------|
| `spi` | SPI | SW1=ON, SW2=OFF |
| `i2c` | I2C | SW1=OFF, SW2=ON |
