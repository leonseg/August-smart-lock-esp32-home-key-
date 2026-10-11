/**
 * web_ui.cpp
 *
 * Local control page (see web_ui.h). The server is only started in STA mode;
 * in AP mode port 80 belongs to the config portal in wifi_mgr.cpp.
 */

#include "web_ui.h"

#if WEB_UI_ENABLED

#include <WiFi.h>
#include <WebServer.h>
#include "wifi_mgr.h"
#include "mqtt.h"
#include "leds.h"
#include "lock_controller.h"

static WebServer s_web(80);
static bool      s_started = false;

static const char* PAGE_HTML = R"HTML(<!doctype html>
<html lang="fr"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Serrure August</title>
<style>
:root{--bg:#0f1115;--card:#181b22;--line:#2a2f3a;--txt:#e8eaf0;--mut:#8b93a5;--ok:#2fbf71;--bad:#e5484d;--warn:#f5a524;--acc:#4c8dff}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--txt);font:16px/1.45 system-ui,sans-serif}
main{max-width:460px;margin:0 auto;padding:16px;display:grid;gap:14px}
h1{font-size:1.15em;margin:4px 0 0}
h2{font-size:.78em;text-transform:uppercase;letter-spacing:.08em;color:var(--mut);margin:0 0 10px}
.card{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:16px}
#state{font-size:2em;font-weight:700;text-align:center;margin:6px 0 14px}
.row{display:flex;gap:10px;align-items:center;flex-wrap:wrap}
.row+.row{margin-top:12px}
button{flex:1;background:#232733;color:var(--txt);border:1px solid var(--line);border-radius:10px;padding:16px 12px;font:inherit;font-weight:600;cursor:pointer}
button:disabled{opacity:.45}
button.lock{background:var(--bad);border-color:var(--bad);color:#fff}
button.unlock{background:var(--ok);border-color:var(--ok);color:#fff}
button.sw{flex:0 0 40px;height:40px;padding:0}
input[type=range]{flex:1}
input[type=color]{flex:0 0 52px;height:40px;padding:0;border:1px solid var(--line);border-radius:8px;background:none}
.meta{color:var(--mut);font-size:.85em;display:grid;grid-template-columns:1fr 1fr;gap:4px 12px}
.meta b{color:var(--txt);font-weight:500}
#err{color:var(--bad);font-size:.9em;min-height:1.2em;text-align:center}
</style></head><body><main>
<h1>Serrure August</h1>
<section class="card">
  <div id="state">…</div>
  <div class="row">
    <button class="lock" id="bLock">Verrouiller</button>
    <button class="unlock" id="bUnlock">Déverrouiller</button>
  </div>
  <div id="err"></div>
</section>
<section class="card">
  <h2>Anneau lumineux</h2>
  <div class="row" id="sw"></div>
  <div class="row">
    <input type="color" id="col" value="#ff8800">
    <button id="bCol">Appliquer</button>
    <button id="bOff">Éteindre</button>
  </div>
  <div class="row"><input type="range" id="bri" min="0" max="255" value="128"></div>
</section>
<section class="card">
  <div class="meta">
    <span>Wi-Fi</span><b id="mWifi">—</b>
    <span>Signal</span><b id="mRssi">—</b>
    <span>Adresse</span><b id="mIp">—</b>
    <span>MQTT</span><b id="mMqtt">—</b>
    <span>Version</span><b id="mVer">—</b>
  </div>
</section>
</main>
<script>
(function(){
  "use strict";
  var $=function(id){return document.getElementById(id);};
  var NAMES={LOCKED:"Verrouillée",UNLOCKED:"Déverrouillée",LOCKING:"Verrouillage…",UNLOCKING:"Déverrouillage…",JAMMED:"Bloquée",ERROR:"Erreur",UNKNOWN:"Position inconnue"};
  var COLORS={LOCKED:"var(--bad)",UNLOCKED:"var(--ok)",LOCKING:"var(--warn)",UNLOCKING:"var(--warn)",JAMMED:"var(--bad)",ERROR:"var(--bad)"};
  var briBusy=false;
  function show(s){
    $("state").textContent=NAMES[s.state]||s.state;
    $("state").style.color=COLORS[s.state]||"";
    var moving=(s.state==="LOCKING"||s.state==="UNLOCKING");
    $("bLock").disabled=moving; $("bUnlock").disabled=moving;
    $("mWifi").textContent=s.ssid; $("mRssi").textContent=s.rssi+" dBm"; $("mIp").textContent=s.ip;
    $("mMqtt").textContent=s.mqtt?"connecté":"non connecté"; $("mVer").textContent=s.version;
    if(!briBusy) $("bri").value=s.brightness;
    $("err").textContent="";
  }
  function refresh(){
    fetch("/api/status").then(function(r){return r.json();}).then(show)
      .catch(function(){$("err").textContent="Serrure injoignable";});
  }
  function post(path,body){
    return fetch(path,{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body:body||""})
      .then(function(r){return r.json();}).then(show)
      .catch(function(){$("err").textContent="Commande non transmise";});
  }
  $("bLock").addEventListener("click",function(){post("/api/lock");});
  $("bUnlock").addEventListener("click",function(){post("/api/unlock");});
  $("bOff").addEventListener("click",function(){post("/api/led","off=1");});
  $("bCol").addEventListener("click",function(){
    var h=$("col").value;
    post("/api/led","rgb="+parseInt(h.substr(1,2),16)+","+parseInt(h.substr(3,2),16)+","+parseInt(h.substr(5,2),16));
  });
  $("bri").addEventListener("input",function(){briBusy=true;});
  $("bri").addEventListener("change",function(){
    post("/api/led","brightness="+$("bri").value).then(function(){briBusy=false;});
  });
  [[255,0,0],[0,255,0],[0,0,255],[255,255,255],[255,180,0],[160,0,255]].forEach(function(c){
    var b=document.createElement("button");
    b.className="sw"; b.style.background="rgb("+c.join(",")+")";
    b.addEventListener("click",function(){post("/api/led","rgb="+c.join(","));});
    $("sw").appendChild(b);
  });
  refresh(); setInterval(refresh,1500);
})();
</script></body></html>
)HTML";

static void send_status() {
    char buf[320];
    snprintf(buf, sizeof(buf),
             "{\"state\":\"%s\",\"ssid\":\"%s\",\"ip\":\"%s\",\"rssi\":%d,\"mqtt\":%s,"
             "\"led_mode\":%d,\"brightness\":%u,\"version\":\"%s\"}",
             lock_controller_state_str(), wifi_mgr_ssid().c_str(), wifi_mgr_ip().c_str(),
             wifi_mgr_rssi(), mqtt_is_connected() ? "true" : "false",
             (int)leds_mode(), leds_brightness(), FW_VERSION);
    s_web.sendHeader("Cache-Control", "no-store");
    s_web.send(200, "application/json", buf);
}

static void handle_lock()   { lock_controller_request_lock();   send_status(); }
static void handle_unlock() { lock_controller_request_unlock(); send_status(); }

static void handle_led() {
    if (s_web.hasArg("off")) {
        leds_set_mode(LED_MODE_OFF);
    }
    if (s_web.hasArg("brightness")) {
        leds_set_brightness((uint8_t)constrain(s_web.arg("brightness").toInt(), 0, 255));
    }
    if (s_web.hasArg("rgb")) {
        int r, g, b;
        if (sscanf(s_web.arg("rgb").c_str(), "%d,%d,%d", &r, &g, &b) == 3) {
            leds_set_custom_rgb((uint8_t)constrain(r, 0, 255), (uint8_t)constrain(g, 0, 255),
                                (uint8_t)constrain(b, 0, 255));
        }
    }
    send_status();
}

void web_ui_loop() {
    if (wifi_mgr_state() != WIFI_STATE_CONNECTED) return;
    if (!s_started) {
        s_web.on("/", HTTP_GET, []() { s_web.send(200, "text/html; charset=utf-8", PAGE_HTML); });
        s_web.on("/api/status", HTTP_GET, send_status);
        s_web.on("/api/lock", HTTP_POST, handle_lock);
        s_web.on("/api/unlock", HTTP_POST, handle_unlock);
        s_web.on("/api/led", HTTP_POST, handle_led);
        s_web.begin();
        s_started = true;
        LOG_TAG("WEB", "control page at http://%s/", wifi_mgr_ip().c_str());
    }
    s_web.handleClient();
}

#else

void web_ui_loop() {}

#endif // WEB_UI_ENABLED
