/**
 * wifi_mgr.cpp
 *
 * WiFi connection manager with fallback AP + minimal config portal.
 *
 * The portal serves a single HTML page at http://192.168.4.1/ that posts
 * SSID / password / MQTT settings back to /save. After a successful save
 * the ESP reboots and connects in STA mode.
 *
 * NOTE: when integrating with rednblkx/HomeKey-ESP32 (Option B overlay),
 * this module should be disabled — HomeKey-ESP32 has its own captive portal.
 * See HOMEKEY-INTEGRATION.md.
 */

#include "wifi_mgr.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

static wifi_state_t s_state = WIFI_STATE_BOOT;
static WebServer    s_http(80);
static Preferences  s_prefs;
static uint32_t     s_connect_started = 0;
static uint32_t     s_last_retry = 0;
static String       s_ssid;

static String ap_ssid() {
    uint64_t mac = ESP.getEfuseMac();
    char suffix[5];
    snprintf(suffix, sizeof(suffix), "%04X", (uint16_t)(mac & 0xFFFF));
    return String(WIFI_AP_SSID_PREFIX) + suffix;
}

static const char* PORTAL_HTML = R"HTML(
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>August HomeKey Setup</title>
<style>
body{font-family:system-ui,sans-serif;max-width:420px;margin:2em auto;padding:1em;background:#111;color:#eee}
h1{font-size:1.3em}label{display:block;margin-top:1em}
input{width:100%;padding:.6em;box-sizing:border-box;border:1px solid #333;border-radius:6px;background:#222;color:#eee}
button{margin-top:1.5em;width:100%;padding:.8em;background:#28a745;color:#fff;border:0;border-radius:6px;font-size:1em}
.muted{color:#888;font-size:.9em}
</style></head><body>
<h1>August HomeKey Setup</h1>
<p class="muted">Configure WiFi + MQTT. The lock will reboot after saving.</p>
<form action="/save" method="POST">
  <label>WiFi SSID<input name="ssid" required></label>
  <label>WiFi Password<input name="pass" type="password"></label>
  <label>MQTT Host<input name="mqtt_host" placeholder="192.168.1.10"></label>
  <label>MQTT Port<input name="mqtt_port" value="1883"></label>
  <label>MQTT User<input name="mqtt_user"></label>
  <label>MQTT Password<input name="mqtt_pass" type="password"></label>
  <button type="submit">Save &amp; Reboot</button>
</form>
</body></html>
)HTML";

static void handle_root() {
    s_http.send(200, "text/html", PORTAL_HTML);
}

static void handle_save() {
    String ssid = s_http.arg("ssid");
    String pass = s_http.arg("pass");
    if (ssid.isEmpty()) { s_http.send(400, "text/plain", "SSID required"); return; }

    s_prefs.begin(NVS_NAMESPACE, false);
    s_prefs.putString(NVS_KEY_WIFI_SSID, ssid);
    s_prefs.putString(NVS_KEY_WIFI_PASS, pass);
    if (s_http.hasArg("mqtt_host")) s_prefs.putString(NVS_KEY_MQTT_HOST, s_http.arg("mqtt_host"));
    if (s_http.hasArg("mqtt_port")) s_prefs.putUInt(NVS_KEY_MQTT_PORT, s_http.arg("mqtt_port").toInt());
    if (s_http.hasArg("mqtt_user")) s_prefs.putString(NVS_KEY_MQTT_USER, s_http.arg("mqtt_user"));
    if (s_http.hasArg("mqtt_pass")) s_prefs.putString(NVS_KEY_MQTT_PASS, s_http.arg("mqtt_pass"));
    s_prefs.end();

    s_http.send(200, "text/html",
        "<h2>Saved. Rebooting…</h2><p>Reconnect to your WiFi to find the lock.</p>");
    delay(1200);
    ESP.restart();
}

static void start_ap() {
    String ssid = ap_ssid();
    LOG_TAG("WIFI", "starting AP '%s'", ssid.c_str());
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), WIFI_AP_PASSWORD);
    s_http.on("/", handle_root);
    s_http.on("/save", HTTP_POST, handle_save);
    s_http.onNotFound(handle_root);
    s_http.begin();
    s_state = WIFI_STATE_AP_CONFIG;
}

static bool try_connect(const String& ssid, const String& pass) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    s_connect_started = millis();
    s_state = WIFI_STATE_CONNECTING;
    s_ssid = ssid;
    LOG_TAG("WIFI", "connecting to '%s'...", ssid.c_str());
    return true;
}

void wifi_mgr_init() {
    s_prefs.begin(NVS_NAMESPACE, true);
    String ssid = s_prefs.getString(NVS_KEY_WIFI_SSID, "");
    String pass = s_prefs.getString(NVS_KEY_WIFI_PASS, "");
    s_prefs.end();

    if (ssid.isEmpty()) {
        start_ap();
    } else {
        try_connect(ssid, pass);
    }
}

void wifi_mgr_loop() {
    switch (s_state) {
        case WIFI_STATE_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                s_state = WIFI_STATE_CONNECTED;
                LOG_TAG("WIFI", "connected — IP=%s RSSI=%d",
                        WiFi.localIP().toString().c_str(), WiFi.RSSI());
            } else if (millis() - s_connect_started > WIFI_CONNECT_TIMEOUT_MS) {
                LOG_TAG("WIFI", "connect timeout — falling back to AP");
                start_ap();
            }
            break;

        case WIFI_STATE_CONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                LOG_TAG("WIFI", "disconnected — will retry");
                s_state = WIFI_STATE_RETRY;
                s_last_retry = millis();
            }
            break;

        case WIFI_STATE_RETRY:
            if (millis() - s_last_retry > WIFI_RETRY_INTERVAL_MS) {
                s_prefs.begin(NVS_NAMESPACE, true);
                String ssid = s_prefs.getString(NVS_KEY_WIFI_SSID, "");
                String pass = s_prefs.getString(NVS_KEY_WIFI_PASS, "");
                s_prefs.end();
                if (!ssid.isEmpty()) try_connect(ssid, pass);
            }
            break;

        case WIFI_STATE_AP_CONFIG:
            s_http.handleClient();
            break;

        default:
            break;
    }
}

wifi_state_t wifi_mgr_state() { return s_state; }
String       wifi_mgr_ssid()  { return s_ssid; }
int          wifi_mgr_rssi()  { return WiFi.RSSI(); }
String       wifi_mgr_ip()    {
    return (s_state == WIFI_STATE_AP_CONFIG) ? WiFi.softAPIP().toString()
                                             : WiFi.localIP().toString();
}

bool wifi_mgr_save_credentials(const String& ssid, const String& pass) {
    if (ssid.isEmpty()) return false;
    s_prefs.begin(NVS_NAMESPACE, false);
    s_prefs.putString(NVS_KEY_WIFI_SSID, ssid);
    s_prefs.putString(NVS_KEY_WIFI_PASS, pass);
    s_prefs.end();
    return true;
}

void wifi_mgr_factory_reset() {
    s_prefs.begin(NVS_NAMESPACE, false);
    s_prefs.remove(NVS_KEY_WIFI_SSID);
    s_prefs.remove(NVS_KEY_WIFI_PASS);
    s_prefs.end();
    delay(500);
    ESP.restart();
}
