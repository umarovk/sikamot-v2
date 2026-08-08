#include "web_portal.h"
#include "storage.h"
#include "../include/state.h"
#include <WebServer.h>
#include <ArduinoJson.h>

static WebServer server(80);
static String lastScannedUID = "";
static unsigned long lastScannedTime = 0;

void setLastScannedUID(const String& uid) {
    lastScannedUID = uid;
    lastScannedTime = millis();
}

// ── HTML Page ────────────────────────────────────────────────────
static const char HTML_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SIKAMOT-V2</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,sans-serif;background:#0d0d0d;color:#e0e0e0}
.hdr{background:#111827;padding:18px;text-align:center;border-bottom:2px solid #1f2937}
.hdr h1{color:#38bdf8;font-size:22px;letter-spacing:2px}
.hdr p{color:#6b7280;font-size:12px;margin-top:4px}
.wrap{max-width:460px;margin:16px auto;padding:0 14px}
.card{background:#111827;border-radius:12px;padding:18px;margin-bottom:14px;border:1px solid #1f2937}
.dot{width:12px;height:12px;border-radius:50%;display:inline-block;margin-right:8px;vertical-align:middle}
.dot-on{background:#22c55e;box-shadow:0 0 8px #22c55e}
.dot-off{background:#ef4444}
.badge{display:inline-block;padding:3px 10px;border-radius:20px;font-size:12px;font-weight:700}
.b-locked{background:#7f1d1d;color:#fca5a5;border:1px solid #ef4444}
.b-unlocked{background:#14532d;color:#86efac;border:1px solid #22c55e}
.b-enroll{background:#713f12;color:#fde68a;border:1px solid #f59e0b}
.b-setup{background:#1e3a5f;color:#93c5fd;border:1px solid #3b82f6}
.btn{display:block;width:100%;padding:11px;border:none;border-radius:8px;font-size:15px;cursor:pointer;margin-top:10px;font-weight:700;transition:opacity .2s}
.btn:hover{opacity:.85}
.btn-blue{background:#38bdf8;color:#0d0d0d}
.btn-red{background:#ef4444;color:#fff}
.btn-green{background:#22c55e;color:#0d0d0d}
.btn-yellow{background:#f59e0b;color:#0d0d0d}
.btn-sm{padding:5px 11px;border-radius:6px;border:none;cursor:pointer;font-size:12px;font-weight:700}
.btn-rm{background:#ef4444;color:#fff}
.ci{display:flex;justify-content:space-between;align-items:center;padding:9px 0;border-bottom:1px solid #1f2937}
.uid{font-family:monospace;font-size:13px;color:#38bdf8}
.scan-box{background:#0a0a0a;border:2px dashed #374151;border-radius:8px;padding:16px;text-align:center;margin:10px 0}
.scan-uid{font-family:monospace;font-size:17px;color:#22c55e;margin:8px 0;min-height:26px}
.lbl{font-size:12px;color:#6b7280;margin-bottom:4px}
.msg{font-size:13px;color:#9ca3af;margin-top:10px}
h2{font-size:16px;margin-bottom:12px}
h3{font-size:14px;margin-bottom:8px}
hr{border:none;border-top:1px solid #1f2937;margin:12px 0}
</style>
</head>
<body>
<div class="hdr">
  <h1>SIKAMOT-V2</h1>
  <p>IoT Motor Ignition via RFID</p>
</div>
<div class="wrap">

  <!-- Status Card -->
  <div class="card">
    <div style="display:flex;justify-content:space-between;align-items:center">
      <div>
        <span class="dot" id="sDot"></span>
        <strong id="sVehicle">--</strong>
      </div>
      <span class="badge" id="sBadge">--</span>
    </div>
    <p class="msg" id="sMsg">Memuat...</p>
    <button class="btn btn-green" id="btnStart" style="display:none" onclick="doStartEngine()">&#9654; Hidupkan Mesin</button>
    <button class="btn btn-red"   id="btnStop"  style="display:none" onclick="doStopEngine()">&#9632; Matikan Mesin</button>
  </div>

  <!-- First Setup -->
  <div class="card" id="secSetup" style="display:none">
    <h2 style="color:#3b82f6">Setup Awal — Master Card</h2>
    <p style="font-size:13px;margin-bottom:12px">Tempel kartu RFID/KTP ke pembaca untuk dijadikan Master Card.</p>
    <div class="scan-box">
      <div class="lbl">Kartu terdeteksi:</div>
      <div class="scan-uid" id="setupUID">Menunggu scan...</div>
    </div>
    <button class="btn btn-yellow" id="btnMaster" disabled onclick="doSetMaster()">Jadikan Master Card</button>
  </div>

  <!-- Card Management -->
  <div class="card" id="secCards" style="display:none">
    <h2>Kartu Terdaftar</h2>
    <div id="cardList"><p style="color:#6b7280;font-size:13px">Memuat...</p></div>
    <hr>
    <h3>Tambah Kartu Baru</h3>
    <div class="scan-box">
      <div class="lbl">Tempel kartu baru ke pembaca:</div>
      <div class="scan-uid" id="addUID">Menunggu scan...</div>
    </div>
    <button class="btn btn-blue" id="btnAdd" disabled onclick="doAddCard()">Tambah Kartu Ini</button>
  </div>

  <!-- Enroll Mode -->
  <div class="card" id="secEnroll" style="display:none">
    <h2 style="color:#f59e0b">Mode Enroll Aktif</h2>
    <p style="font-size:13px;color:#9ca3af">Tempel kartu baru = tambah. Tempel kartu terdaftar = hapus. Tap Master Card lagi untuk keluar.</p>
    <div class="scan-box" style="margin-top:12px">
      <div class="lbl">Kartu terakhir discan:</div>
      <div class="scan-uid" id="enrollUID">Menunggu scan...</div>
    </div>
  </div>

</div>

<script>
let scannedUID='', curState='';

async function fetchStatus(){
  try{
    const r=await fetch('/api/status');
    const d=await r.json();
    renderUI(d);
  }catch(e){}
}

async function fetchScan(){
  try{
    const r=await fetch('/api/last-scan');
    const d=await r.json();
    if(d.uid && d.uid!==scannedUID){
      scannedUID=d.uid;
      document.getElementById('setupUID').textContent=d.uid;
      document.getElementById('addUID').textContent=d.uid;
      document.getElementById('enrollUID').textContent=d.uid;
      document.getElementById('btnMaster').disabled=false;
      document.getElementById('btnAdd').disabled=false;
    }
  }catch(e){}
}

function renderUI(d){
  curState=d.state;
  const dot=document.getElementById('sDot');
  const veh=document.getElementById('sVehicle');
  const badge=document.getElementById('sBadge');
  const msg=document.getElementById('sMsg');
  const on=d.state==='UNLOCKED';
  dot.className='dot '+(on?'dot-on':'dot-off');
  veh.textContent=on?'Mesin Hidup':'Mesin Mati';
  const map={
    FIRST_SETUP:['SETUP','b-setup','Daftarkan Master Card terlebih dahulu.'],
    LOCKED:['TERKUNCI','b-locked','Tempel kartu authorized untuk menyalakan mesin.'],
    UNLOCKED:['HIDUP','b-unlocked','Tempel kartu authorized untuk mematikan mesin.'],
    ENROLL:['ENROLL','b-enroll','Mode pendaftaran kartu aktif.']
  };
  const [label,cls,text]=map[d.state]||['--','',''];
  badge.textContent=label;
  badge.className='badge '+cls;
  msg.textContent=text;
  document.getElementById('btnStart').style.display=d.state==='LOCKED'?'block':'none';
  document.getElementById('btnStop').style.display=d.state==='UNLOCKED'?'block':'none';
  document.getElementById('secSetup').style.display=d.state==='FIRST_SETUP'?'block':'none';
  document.getElementById('secCards').style.display=(d.state==='LOCKED'||d.state==='UNLOCKED')?'block':'none';
  document.getElementById('secEnroll').style.display=d.state==='ENROLL'?'block':'none';
  if(d.cards!==undefined){
    const list=document.getElementById('cardList');
    if(d.cards.length===0){list.innerHTML='<p style="color:#6b7280;font-size:13px">Belum ada kartu terdaftar.</p>';return;}
    list.innerHTML=d.cards.map(u=>`<div class="ci"><span class="uid">${u}</span><button class="btn-sm btn-rm" onclick="doRemove('${u}')">Hapus</button></div>`).join('');
  }
}

async function doStartEngine(){
  if(!confirm('Hidupkan mesin via web?'))return;
  const r=await fetch('/api/start-engine',{method:'POST'});
  const d=await r.json();
  if(!d.success)alert('Gagal: '+d.message);
  setTimeout(fetchStatus,500);
}

async function doStopEngine(){
  if(!confirm('Matikan mesin?'))return;
  const r=await fetch('/api/stop-engine',{method:'POST'});
  const d=await r.json();
  if(!d.success)alert('Gagal: '+d.message);
  setTimeout(fetchStatus,500);
}

async function doSetMaster(){
  if(!scannedUID)return;
  const r=await fetch('/api/set-master',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({uid:scannedUID})});
  const d=await r.json();
  if(d.success){alert('Master Card berhasil didaftarkan!');scannedUID='';fetchStatus();}
  else alert('Gagal: '+d.message);
}

async function doAddCard(){
  if(!scannedUID)return;
  const r=await fetch('/api/add-card',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({uid:scannedUID})});
  const d=await r.json();
  if(d.success){
    alert('Kartu berhasil ditambahkan!');
    scannedUID='';
    document.getElementById('addUID').textContent='Menunggu scan...';
    document.getElementById('btnAdd').disabled=true;
    fetchStatus();
  } else alert('Gagal: '+d.message);
}

async function doRemove(uid){
  if(!confirm('Hapus kartu '+uid+'?'))return;
  const r=await fetch('/api/remove-card',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({uid})});
  const d=await r.json();
  if(d.success)fetchStatus();
  else alert('Gagal menghapus kartu.');
}

fetchStatus();
setInterval(fetchStatus,2000);
setInterval(fetchScan,500);
</script>
</body>
</html>
)rawhtml";

// ── Helpers ─────────────────────────────────────────────────────
static String stateToString() {
    switch (systemState) {
        case STATE_FIRST_SETUP: return "FIRST_SETUP";
        case STATE_LOCKED:      return "LOCKED";
        case STATE_UNLOCKED:    return "UNLOCKED";
        case STATE_ENROLL:      return "ENROLL";
    }
    return "UNKNOWN";
}

static void sendJson(int code, const String& body) {
    server.send(code, "application/json", body);
}

static String parseUID(const String& body) {
    StaticJsonDocument<128> doc;
    if (deserializeJson(doc, body)) return "";
    return doc["uid"].as<String>();
}

// ── Route Handlers ───────────────────────────────────────────────
static void handleRoot() {
    server.send_P(200, "text/html", HTML_PAGE);
}

static void handleStatus() {
    StaticJsonDocument<512> doc;
    doc["state"] = stateToString();

    JsonArray cards = doc.createNestedArray("cards");
    int count = getCardCount();
    for (int i = 0; i < count; i++) {
        cards.add(getCard(i));
    }

    String out;
    serializeJson(doc, out);
    sendJson(200, out);
}

static void handleLastScan() {
    StaticJsonDocument<64> doc;
    // Hapus UID lama setelah 30 detik agar tidak stale
    if (millis() - lastScannedTime > 30000) lastScannedUID = "";
    doc["uid"] = lastScannedUID;
    String out;
    serializeJson(doc, out);
    sendJson(200, out);
}

static void handleSetMaster() {
    String uid = parseUID(server.arg("plain"));
    if (uid.isEmpty()) {
        sendJson(400, "{\"success\":false,\"message\":\"UID kosong\"}");
        return;
    }
    if (systemState != STATE_FIRST_SETUP) {
        sendJson(403, "{\"success\":false,\"message\":\"Bukan mode setup\"}");
        return;
    }
    setMasterUID(uid);
    systemState = STATE_LOCKED;
    lastScannedUID = "";
    sendJson(200, "{\"success\":true}");
}

static void handleAddCard() {
    String uid = parseUID(server.arg("plain"));
    if (uid.isEmpty()) {
        sendJson(400, "{\"success\":false,\"message\":\"UID kosong\"}");
        return;
    }
    if (isMaster(uid)) {
        sendJson(400, "{\"success\":false,\"message\":\"UID ini adalah Master Card\"}");
        return;
    }
    if (isAuthorized(uid)) {
        sendJson(400, "{\"success\":false,\"message\":\"Kartu sudah terdaftar\"}");
        return;
    }
    if (!addCard(uid)) {
        sendJson(400, "{\"success\":false,\"message\":\"Daftar kartu penuh (maks 10)\"}");
        return;
    }
    lastScannedUID = "";
    sendJson(200, "{\"success\":true}");
}

static void handleRemoveCard() {
    String uid = parseUID(server.arg("plain"));
    if (uid.isEmpty()) {
        sendJson(400, "{\"success\":false,\"message\":\"UID kosong\"}");
        return;
    }
    if (!removeCard(uid)) {
        sendJson(404, "{\"success\":false,\"message\":\"Kartu tidak ditemukan\"}");
        return;
    }
    sendJson(200, "{\"success\":true}");
}

static void handleStartEngine() {
    if (systemState != STATE_LOCKED) {
        sendJson(400, "{\"success\":false,\"message\":\"Mesin sudah hidup atau sistem tidak siap\"}");
        return;
    }
    pendingCommand = CMD_START_ENGINE;
    sendJson(200, "{\"success\":true}");
}

static void handleStopEngine() {
    if (systemState != STATE_UNLOCKED) {
        sendJson(400, "{\"success\":false,\"message\":\"Mesin sudah mati\"}");
        return;
    }
    pendingCommand = CMD_STOP_ENGINE;
    sendJson(200, "{\"success\":true}");
}

// ── Public ───────────────────────────────────────────────────────
void webPortalInit() {
    server.on("/",                  HTTP_GET,  handleRoot);
    server.on("/api/status",        HTTP_GET,  handleStatus);
    server.on("/api/last-scan",     HTTP_GET,  handleLastScan);
    server.on("/api/set-master",    HTTP_POST, handleSetMaster);
    server.on("/api/add-card",      HTTP_POST, handleAddCard);
    server.on("/api/remove-card",   HTTP_POST, handleRemoveCard);
    server.on("/api/start-engine",  HTTP_POST, handleStartEngine);
    server.on("/api/stop-engine",   HTTP_POST, handleStopEngine);
    server.begin();
}

void webPortalHandle() {
    server.handleClient();
}
